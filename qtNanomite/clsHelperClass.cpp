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
#include <fstream>
#include <algorithm>
#include <QtCore>
#include <QFileDialog>
#include <string>

using namespace std;

clsHelperClass::clsHelperClass()
{
}

clsHelperClass::~clsHelperClass()
{
}

bool clsHelperClass::WriteToSettingsFile(clsDebugger *_coreDebugger,qtNanomiteDisAsColorSettings *qtNanomiteDisAsColor, wstring originalJIT)
{
	wofstream outfile;
	outfile.open("NanomiteConfig.ini");

	if(!outfile.is_open())
		return false;

	PTCHAR cTemp = (PTCHAR)clsMemManager::CAlloc(128);

	wsprintf(cTemp,L"%s=%s\n",L"DebugChilds",_coreDebugger->dbgSettings.bDebugChilds ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"AutoLoadSym",_coreDebugger->dbgSettings.bAutoLoadSymbols ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"BreakOnNewDLL",_coreDebugger->dbgSettings.bBreakOnNewDLL ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"BreakOnNewTID",_coreDebugger->dbgSettings.bBreakOnNewTID ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"BreakOnNewPID",_coreDebugger->dbgSettings.bBreakOnNewPID ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"BreakOnExDLL",_coreDebugger->dbgSettings.bBreakOnExDLL ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"BreakOnExTID",_coreDebugger->dbgSettings.bBreakOnExTID ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"BreakOnExPID",_coreDebugger->dbgSettings.bBreakOnExPID ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"BreakOnModuleEP",_coreDebugger->dbgSettings.bBreakOnModuleEP ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"BreakOnSystemEP",_coreDebugger->dbgSettings.bBreakOnSystemEP ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"BreakOnTLS",_coreDebugger->dbgSettings.bBreakOnTLS ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));

	wsprintf(cTemp,L"%s=%d\n",L"SUSPENDTYPE",_coreDebugger->dbgSettings.dwSuspendType);
	outfile.write(cTemp,wcslen(cTemp));

	for(size_t i = 0;i < _coreDebugger->ExceptionHandler.size();i++)
	{
		wsprintf(cTemp,L"%s=%08X:%d\n",L"EXCEPTION",_coreDebugger->ExceptionHandler[i].dwExceptionType,_coreDebugger->ExceptionHandler[i].dwAction);
		outfile.write(cTemp,wcslen(cTemp));
	}

	wsprintf(cTemp,L"%s=%d\n",L"DefaultExceptionMode",_coreDebugger->dbgSettings.dwDefaultExceptionMode);
	outfile.write(cTemp,wcslen(cTemp));

	wsprintf(cTemp,L"%s=%s\n",L"COLOR_BP",qtNanomiteDisAsColor->colorBP.data());
	outfile.write(cTemp,wcslen(cTemp));

	wsprintf(cTemp,L"%s=%s\n",L"COLOR_CALL",qtNanomiteDisAsColor->colorCall.data());
	outfile.write(cTemp,wcslen(cTemp));

	wsprintf(cTemp,L"%s=%s\n",L"COLOR_JUMP",qtNanomiteDisAsColor->colorJump.data());
	outfile.write(cTemp,wcslen(cTemp));

	wsprintf(cTemp,L"%s=%s\n",L"COLOR_MOVE",qtNanomiteDisAsColor->colorMove.data());
	outfile.write(cTemp,wcslen(cTemp));

	wsprintf(cTemp,L"%s=%s\n",L"COLOR_STACK",qtNanomiteDisAsColor->colorStack.data());
	outfile.write(cTemp,wcslen(cTemp));

	wsprintf(cTemp,L"%s=%s\n",L"COLOR_MATH",qtNanomiteDisAsColor->colorMath.data());
	outfile.write(cTemp,wcslen(cTemp));

	wsprintf(cTemp,L"%s=%s\n",L"defaultJIT",originalJIT.c_str());
	outfile.write(cTemp,wcslen(cTemp));

	outfile.close();
	clsMemManager::CFree(cTemp);
	return true;
}

bool clsHelperClass::ReadFromSettingsFile(clsDebugger *_coreDebugger,qtNanomiteDisAsColorSettings *qtNanomiteDisAsColor, wstring& originalJIT)
{
	wstring sLine;
	wifstream infile;
	infile.open(L"NanomiteConfig.ini");

	if(!infile.is_open())
		return false;

	_coreDebugger->CustomExceptionRemoveAll();
	while(!infile.eof())
	{
		getline(infile,sLine); // Saves the line in STRING.

		vector<wstring> sSettingLine = split(sLine,L"=");

		if(sSettingLine[0] == L"DebugChilds")
		{
			if(sSettingLine[1] == L"true")
				_coreDebugger->dbgSettings.bDebugChilds = true;
			else
				_coreDebugger->dbgSettings.bDebugChilds = false;
		}
		else if(sSettingLine[0] == L"AutoLoadSym")
		{
			if(sSettingLine[1] == L"true")
				_coreDebugger->dbgSettings.bAutoLoadSymbols = true;
			else
				_coreDebugger->dbgSettings.bAutoLoadSymbols = false;
		}
		else if(sSettingLine[0] == L"BreakOnNewDLL")
		{
			if(sSettingLine[1] == L"true")
				_coreDebugger->dbgSettings.bBreakOnNewDLL = true;
			else
				_coreDebugger->dbgSettings.bBreakOnNewDLL = false;
		}
		else if(sSettingLine[0] == L"BreakOnNewTID")
		{
			if(sSettingLine[1] == L"true")
				_coreDebugger->dbgSettings.bBreakOnNewTID = true;
			else
				_coreDebugger->dbgSettings.bBreakOnNewTID = false;
		}
		else if(sSettingLine[0] == L"BreakOnNewPID")
		{
			if(sSettingLine[1] == L"true")
				_coreDebugger->dbgSettings.bBreakOnNewPID = true;
			else
				_coreDebugger->dbgSettings.bBreakOnNewPID = false;
		}
		else if(sSettingLine[0] == L"BreakOnExDLL")
		{
			if(sSettingLine[1] == L"true")
				_coreDebugger->dbgSettings.bBreakOnExDLL = true;
			else
				_coreDebugger->dbgSettings.bBreakOnExDLL = false;
		}
		else if(sSettingLine[0] == L"BreakOnExTID")
		{
			if(sSettingLine[1] == L"true")
				_coreDebugger->dbgSettings.bBreakOnExTID = true;
			else
				_coreDebugger->dbgSettings.bBreakOnExTID = false;
		}
		else if(sSettingLine[0] == L"BreakOnExPID")
		{
			if(sSettingLine[1] == L"true")
				_coreDebugger->dbgSettings.bBreakOnExPID = true;
			else
				_coreDebugger->dbgSettings.bBreakOnExPID = false;
		}
		else if(sSettingLine[0] == L"BreakOnModuleEP")
		{
			if(sSettingLine[1] == L"true")
				_coreDebugger->dbgSettings.bBreakOnModuleEP = true;
			else
				_coreDebugger->dbgSettings.bBreakOnModuleEP = false;
		}
		else if(sSettingLine[0] == L"BreakOnSystemEP")
		{
			if(sSettingLine[1] == L"true")
				_coreDebugger->dbgSettings.bBreakOnSystemEP = true;
			else
				_coreDebugger->dbgSettings.bBreakOnSystemEP = false;
		}
		else if(sSettingLine[0] == L"BreakOnTLS")
		{
			if(sSettingLine[1] == L"true")
				_coreDebugger->dbgSettings.bBreakOnTLS = true;
			else
				_coreDebugger->dbgSettings.bBreakOnTLS = false;
		}
		else if(sSettingLine[0] == L"DefaultExceptionMode")
			_coreDebugger->dbgSettings.dwDefaultExceptionMode = _wtoi(sSettingLine[1].c_str());
		else if(sSettingLine[0] == L"SUSPENDTYPE")
			_coreDebugger->dbgSettings.dwSuspendType = _wtoi(sSettingLine[1].c_str());
		else if(sSettingLine[0] == L"EXCEPTION")
		{
			QString sTemp = QString().fromStdWString(sSettingLine[1]);
			_coreDebugger->CustomExceptionAdd(sTemp.split(":")[0].toULong(0,16),sTemp.split(":")[1].toULong(0,16),NULL);
		}
		else if(sSettingLine[0] == L"COLOR_BP")
			qtNanomiteDisAsColor->colorBP = QString::fromStdWString(sSettingLine[1]);
		else if(sSettingLine[0] == L"COLOR_CALL")
			qtNanomiteDisAsColor->colorCall = QString::fromStdWString(sSettingLine[1]);
		else if(sSettingLine[0] == L"COLOR_JUMP")
			qtNanomiteDisAsColor->colorJump = QString::fromStdWString(sSettingLine[1]);
		else if(sSettingLine[0] == L"COLOR_MOVE")
			qtNanomiteDisAsColor->colorMove = QString::fromStdWString(sSettingLine[1]);
		else if(sSettingLine[0] == L"COLOR_STACK")
			qtNanomiteDisAsColor->colorStack = QString::fromStdWString(sSettingLine[1]);
		else if(sSettingLine[0] == L"COLOR_MATH")
			qtNanomiteDisAsColor->colorMath = QString::fromStdWString(sSettingLine[1]);
		else if(sSettingLine[0] == L"defaultJIT")
			originalJIT = sSettingLine[1];
	}
	infile.close();
	return true;
}

bool clsHelperClass::MenuLoadNewFile(clsDebugger *_coreDebugger)
{
	QString fileName = QFileDialog::getOpenFileName(NULL,"Please select a Target",QDir::currentPath(),"Executables (*.exe)");

	if(!fileName.isEmpty())
		_coreDebugger->SetTarget(fileName.toStdWString());
	else
	{
		//MessageBox(NULL,L"Error while selecting your file!",L"Nanomite",MB_OK);
		return false;
	}
	return true;
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