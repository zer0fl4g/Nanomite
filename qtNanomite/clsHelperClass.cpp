/*
 * 	This file is part of Nanomite.
 *
 *    Nanomite is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Nanomite is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Nanomite.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "qtDLGNanomite.h"

#include "clsHelperClass.h"
#include "clsMemManager.h"

#include "dbghelp.h"

#include <Psapi.h>
#include <TlHelp32.h>
#include <algorithm>
#include <QtCore>
#include <ObjIdl.h>
#include <Shobjidl.h>
#include <atlbase.h>
#include <shlguid.h>

clsHelperClass::clsHelperClass()
{
}

clsHelperClass::~clsHelperClass()
{
}

bool clsHelperClass::LoadSymbolForAddr(QString &functionName, QString &moduleName, quint64 symbolOffset, HANDLE processHandle)
{
	bool bTest = false;
	IMAGEHLP_MODULEW64 imgMod = {0};
	imgMod.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);
	PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)malloc(sizeof(SYMBOL_INFOW) + MAX_PATH * 2);
	memset(pSymbol, 0, sizeof(SYMBOL_INFOW) + MAX_PATH * 2);
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
	pSymbol->MaxNameLen = MAX_PATH;
	quint64 dwDisplacement;

	SymGetModuleInfoW64(processHandle, symbolOffset, &imgMod);
	SymFromAddrW(processHandle, symbolOffset, &dwDisplacement, pSymbol);

	functionName = QString::fromWCharArray(pSymbol->Name);
	moduleName = QString::fromWCharArray(imgMod.ModuleName);

	free(pSymbol);

	return true;
}

bool clsHelperClass::LoadSourceForAddr(QString &fileName, int &lineNumber, quint64 sourceOffset, HANDLE processHandle)
{
	DWORD dwDisplacement = NULL;
	IMAGEHLP_LINEW64 imgSource;
	imgSource.SizeOfStruct = sizeof(imgSource);
	
	if(SymGetLineFromAddrW64(processHandle,sourceOffset,(PDWORD)&dwDisplacement,&imgSource))
	{
		fileName = QString::fromWCharArray(imgSource.FileName);
		lineNumber = imgSource.LineNumber;

		return true;
	}

	return false;
}

PTCHAR clsHelperClass::reverseStrip(PTCHAR lpString, TCHAR lpSearchString)
{
	size_t	iModPos = NULL,
			iModLen = NULL;
	PTCHAR	lpTempString = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));

	iModLen = wcslen(lpString);
	if(iModLen > 0)
	{
		for(size_t i = iModLen; i > 0 ; i--)
		{
			if(lpString[i] == lpSearchString)
			{
				iModPos = i;
				break;
			}
		}

		memcpy(lpTempString,(LPVOID)&lpString[iModPos + 1],(iModLen - iModPos) * sizeof(TCHAR));
		return lpTempString;
	}					

	clsMemManager::CFree(lpTempString);
	return NULL;
}

QString clsHelperClass::LoadStyleSheet()
{
	//QFile File("stylesheet.qss");
	//File.open(QFile::ReadOnly);
	//QString styleSheet = QLatin1String(File.readAll());
	//File.close();
	//return styleSheet;

	return  "QTableWidget, QTreeWidget"
			"{ "
			"	background: rgb(230, 235, 230);"
			"	font-family: Consolas;"
			"	font-size: 8;"
			"}";
}

bool clsHelperClass::IsWindowsXP()
{
	OSVERSIONINFO versionInfo;
	memset (&versionInfo, 0, sizeof(OSVERSIONINFO));
	versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&versionInfo);

	if (versionInfo.dwMinorVersion == 1  &&  versionInfo.dwMajorVersion == 5)
		return true;
	return false;
}

DWORD clsHelperClass::GetMainThread(DWORD ProcessID)
{
	DWORD ThreadID = NULL;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,ProcessID);
    if(hSnap == INVALID_HANDLE_VALUE) return ThreadID;
 
    THREADENTRY32 threadEntry;
    threadEntry.dwSize = sizeof(THREADENTRY32);

	if(!Thread32First(hSnap,&threadEntry))
	{
		CloseHandle(hSnap);
		return ThreadID;
	}

	do
	{
        if(threadEntry.th32OwnerProcessID == ProcessID)
		{
			ThreadID = threadEntry.th32ThreadID;
			break;
		}
	}
	while(Thread32Next(hSnap,&threadEntry));

	CloseHandle(hSnap);
	return ThreadID;
}

quint64 clsHelperClass::CalcOffsetForModule(PTCHAR moduleName,quint64 Offset,DWORD PID)
{
	HANDLE hProc = clsDebugger::GetProcessHandleByPID(PID);
	PTCHAR sTemp = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	PTCHAR sTemp2 = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));

	MEMORY_BASIC_INFORMATION mbi;

	quint64 dwAddress = NULL,
			dwBase = NULL;
	
	while(VirtualQueryEx(hProc,(LPVOID)dwAddress,&mbi,sizeof(mbi)))
	{
		// Path
		size_t	iModPos = NULL,
				iModLen = NULL;

		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
		memset(sTemp2,0,MAX_PATH * sizeof(TCHAR));
		GetMappedFileName(hProc,(LPVOID)dwAddress,sTemp2,MAX_PATH);

		iModLen = wcslen(sTemp2);
		if(iModLen > 0)
		{
			for(size_t i = iModLen; i > 0 ; i--)
			{
				if(sTemp2[i] == '\\')
				{
					iModPos = i;
					break;
				}
			}
						
			memcpy_s(sTemp,MAX_PATH,(LPVOID)&sTemp2[iModPos + 1],(iModLen - iModPos) * sizeof(TCHAR));

			if(dwBase == 0)
				dwBase = (DWORD64)mbi.BaseAddress;

			if(wcslen(moduleName) <= 0 && Offset > (DWORD64)mbi.BaseAddress && Offset < ((DWORD64)mbi.BaseAddress + mbi.RegionSize))
			{
				wcscpy_s(moduleName,MAX_PATH,sTemp);
				clsMemManager::CFree(sTemp2);
				clsMemManager::CFree(sTemp);

				return dwBase;
			}
			else if(wcscmp(moduleName,sTemp) == 0)
			{
				clsMemManager::CFree(sTemp2);
				clsMemManager::CFree(sTemp);

				return (DWORD64)mbi.BaseAddress;
			}
		}
		else
			dwBase = 0;

		dwAddress += mbi.RegionSize;
	}

	clsMemManager::CFree(sTemp2);
	clsMemManager::CFree(sTemp);
	return Offset;
}

DWORD64 clsHelperClass::RemoteGetProcAddr(QString apiName, quint64 moduleBase, quint64 processID)
{
	HANDLE processHandle = clsDebugger::GetProcessHandleByPID(processID); // do not close
	IMAGE_DOS_HEADER IDH = {0};
	IMAGE_FILE_HEADER IFH = {0};
	IMAGE_OPTIONAL_HEADER64 IOH64 = {0};
	IMAGE_OPTIONAL_HEADER32 IOH32 = {0};
	IMAGE_EXPORT_DIRECTORY exportTable = {0};
	DWORD exportTableVA = NULL,
		ntSig = NULL;
	bool is64Bit = false;
	
	if(!ReadProcessMemory(processHandle, (LPVOID)moduleBase, &IDH, sizeof(IMAGE_DOS_HEADER), NULL) || IDH.e_magic != IMAGE_DOS_SIGNATURE)
		return 0;

	if(!ReadProcessMemory(processHandle, (LPVOID)(moduleBase + IDH.e_lfanew), &ntSig, sizeof(DWORD), NULL) || ntSig != IMAGE_NT_SIGNATURE)
		return 0;
	
	if(!ReadProcessMemory(processHandle, (LPVOID)(moduleBase + IDH.e_lfanew + sizeof(DWORD)), &IFH, sizeof(IFH), NULL))
		return 0;
 
	if(IFH.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER64))
		is64Bit = true;
	else if(IFH.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER32))
		is64Bit = false;
	else
		return 0;
 
	if(is64Bit)
	{
		if(!ReadProcessMemory(processHandle, (LPVOID)(moduleBase + IDH.e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER)), &IOH64, IFH.SizeOfOptionalHeader, NULL) || IOH64.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
			return 0;

		exportTableVA = IOH64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	}
	else
	{
		if(!ReadProcessMemory(processHandle, (LPVOID)(moduleBase + IDH.e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER)), &IOH32, IFH.SizeOfOptionalHeader, NULL) || IOH32.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
			return 0;

		exportTableVA = IOH32.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	}


	if(!ReadProcessMemory(processHandle, (LPVOID)(moduleBase + exportTableVA), &exportTable, sizeof(IMAGE_EXPORT_DIRECTORY), NULL))
		return 0;

	DWORD *ExportFunctionTable = new DWORD[exportTable.NumberOfFunctions];
	DWORD *ExportNameTable = new DWORD[exportTable.NumberOfNames];
	WORD *ExportOrdinalTable = new WORD[exportTable.NumberOfNames];
	
	if(!ReadProcessMemory(processHandle, (LPCVOID)(moduleBase + exportTable.AddressOfNames), ExportNameTable, exportTable.NumberOfNames * sizeof(DWORD), NULL) ||
		!ReadProcessMemory(processHandle, (LPCVOID)(moduleBase + exportTable.AddressOfNameOrdinals), ExportOrdinalTable, exportTable.NumberOfNames * sizeof(WORD), NULL) ||
		!ReadProcessMemory(processHandle, (LPCVOID)(moduleBase + exportTable.AddressOfFunctions), ExportFunctionTable, exportTable.NumberOfFunctions * sizeof(DWORD), NULL))
	{
		delete [] ExportFunctionTable;
		delete [] ExportNameTable;
		delete [] ExportOrdinalTable;

		return 0;
	}

	QString functioName;
	bool isFullString = false;
	CHAR oneCharOfFunction = '\0';

	for(unsigned int i = 0; i < exportTable.NumberOfNames; i++)
	{ 
		isFullString = false;
		functioName.clear(); 

		for(int stringLen = 0; !isFullString; stringLen++)
		{
			if(!ReadProcessMemory(processHandle, (LPCVOID)(moduleBase + ExportNameTable[i] + stringLen), &oneCharOfFunction, sizeof(CHAR), NULL))
			{
				delete [] ExportFunctionTable;
				delete [] ExportNameTable;
				delete [] ExportOrdinalTable;

				return 0;
			}
 
			functioName.append(oneCharOfFunction);

			if(oneCharOfFunction == (CHAR)'\0')
				isFullString = true;
		}
 
		if(functioName.contains(apiName))
		{		
			DWORD64 returnValue = moduleBase + ExportFunctionTable[ExportOrdinalTable[i]];

			delete[] ExportFunctionTable;
			delete[] ExportNameTable;
			delete[] ExportOrdinalTable;

			return returnValue;
		}
	}

	delete [] ExportFunctionTable;
	delete [] ExportNameTable;
	delete [] ExportOrdinalTable;

	return 0;
}

QString clsHelperClass::ResolveShortcut(QString shortcutFile)
{
	HRESULT resultCode = E_FAIL;
	CComPtr<IShellLink> shellLink;
	TCHAR resolvedFilePath[MAX_PATH];  
	WIN32_FIND_DATA findDataStruct;    
	
	resultCode = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&shellLink); 
	if (resultCode >= NULL) 
	{ 
		CComQIPtr<IPersistFile> persistFile(shellLink);
		
		resultCode = persistFile->Load(shortcutFile.toStdWString().c_str(), STGM_READ); 
		if (resultCode >= NULL) 
		{
			resultCode = shellLink->Resolve(NULL, SLR_UPDATE); 
			if (resultCode >= NULL) 
			{
				resultCode = shellLink->GetPath(resolvedFilePath, MAX_PATH, &findDataStruct, SLGP_RAWPATH); 
				if (resultCode < NULL)
					return "";
			} 
		} 
	} 

	return QString::fromWCharArray(resolvedFilePath);
}