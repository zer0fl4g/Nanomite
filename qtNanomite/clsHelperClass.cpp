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
#include "clsDBManager.h"
#include "clsMemManager.h"

#include "dbghelp.h"

#include <Psapi.h>
#include <TlHelp32.h>
#include <algorithm>
#include <QtCore>
#include <string>

using namespace std;

clsHelperClass::clsHelperClass()
{
}

clsHelperClass::~clsHelperClass()
{
}

vector<wstring> clsHelperClass::split(const wstring& s,const wstring& f ){
	vector<wstring> temp;
	if ( f.empty() ) {
		temp.push_back( s );
		return temp;
	}
	typedef wstring::const_iterator iter;
	const iter::difference_type f_size( distance( f.begin(), f.end() ) );
	iter i( s.begin() );
	for ( iter pos; ( pos = search( i , s.end(), f.begin(), f.end() ) ) != s.end(); ) {
		temp.push_back( wstring( i, pos ) );
		advance( pos, f_size );
		i = pos;
	}
	temp.push_back( wstring( i, s.end() ) );
	return temp;
}

bool clsHelperClass::LoadSymbolForAddr(wstring& sFuncName,wstring& sModName,quint64 dwOffset,HANDLE hProc)
{
	bool bTest = false;
	//DWORD PID  = GetProcessId(hProc);

	//if(clsDBManager::DBAPI_getSymbolsFromPID(PID,dwOffset,sFuncName,sModName))
	//	return true;

	IMAGEHLP_MODULEW64 imgMod = {0};
	imgMod.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);
	PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)malloc(sizeof(SYMBOL_INFOW) + MAX_SYM_NAME);
	memset(pSymbol, 0, sizeof(SYMBOL_INFOW) + MAX_SYM_NAME);
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
	pSymbol->MaxNameLen = MAX_SYM_NAME;
	quint64 dwDisplacement;

	bTest = SymGetModuleInfoW64(hProc,dwOffset,&imgMod);
	bTest = SymFromAddrW(hProc,dwOffset,&dwDisplacement,pSymbol);

	sFuncName = pSymbol->Name;
	sModName = imgMod.ModuleName;

	free(pSymbol);

	return true;
	//return clsDBManager::DBAPI_insertSymbolsFromPID(PID,dwOffset,sModName,sFuncName);
}

void clsHelperClass::LoadSourceForAddr(wstring &FileName,int &LineNumber,quint64 dwOffset,HANDLE hProc)
{
	DWORD dwDisplacement = NULL;

	IMAGEHLP_LINEW64 imgSource;
	imgSource.SizeOfStruct = sizeof(imgSource);
	
	if(SymGetLineFromAddrW64(hProc,dwOffset,(PDWORD)&dwDisplacement,&imgSource))
	{
		FileName = imgSource.FileName;
		LineNumber = imgSource.LineNumber;
	}
	return;
}

string clsHelperClass::convertWSTRtoSTR(wstring FileName)
{
	size_t newSize = NULL;
	wcstombs_s(&newSize,NULL, NULL, FileName.c_str(), 0);

	newSize += 2;
	char* newStr = (char*)clsMemManager::CAlloc(newSize);

	wcstombs_s(NULL, newStr, newSize, FileName.c_str(), newSize);
	string str = newStr;
	clsMemManager::CFree(newStr);
	return str;
}

wstring clsHelperClass::convertSTRtoWSTR(string FileName)
{
	size_t newSize = NULL;
	mbstowcs_s(&newSize, NULL, NULL, FileName.c_str(), 0);

	newSize += 2;
	wchar_t* newStr = (wchar_t*)clsMemManager::CAlloc(newSize);

	mbstowcs_s(NULL, newStr, newSize, FileName.c_str(), newSize);
	wstring str = newStr;
	clsMemManager::CFree(newStr);
	return str;
}

PTCHAR clsHelperClass::reverseStrip(PTCHAR lpString, TCHAR lpSearchString)
{
	size_t	iModPos = NULL,
			iModLen = NULL;
	PTCHAR	lpTempString = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

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
	else
	{
		free(lpTempString);
		return NULL;
	}
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

wstring clsHelperClass::replaceAll(wstring orgString, wchar_t oldString, wchar_t newString)
{
	for(int i = 0; i < orgString.length(); i++)
	{
		if(orgString.c_str()[i] == oldString)
			orgString._Myptr()[i] = newString;
	}

	return orgString;
}

DWORD clsHelperClass::GetMainThread(DWORD ProcessID)
{
	DWORD ThreadID = NULL;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
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

	MODULEENTRY32 pModEntry;
	pModEntry.dwSize = sizeof(MODULEENTRY32);
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
		GetMappedFileName(hProc,(LPVOID)dwAddress,sTemp2,MAX_PATH * sizeof(TCHAR));

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

				return dwBase;
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