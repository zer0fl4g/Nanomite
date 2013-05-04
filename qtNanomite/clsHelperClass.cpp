#include "clsHelperClass.h"
#include "clsDBManager.h"
#include "clsMemManager.h"

#include <dbghelp.h>

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

bool clsHelperClass::WriteToSettingsFile(clsDebugger *_coreDebugger,qtNanomiteDisAsColorSettings *qtNanomiteDisAsColor)
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
	wsprintf(cTemp,L"%s=%d\n",L"SUSPENDTYPE",_coreDebugger->dbgSettings.dwSuspendType);
	outfile.write(cTemp,wcslen(cTemp));

	for(size_t i = 0;i < _coreDebugger->ExceptionHandler.size();i++)
	{
		if(_coreDebugger->ExceptionHandler[i].dwExceptionType == EXCEPTION_BREAKPOINT)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_BREAKPOINT",_coreDebugger->ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
		else if(_coreDebugger->ExceptionHandler[i].dwExceptionType == EXCEPTION_SINGLE_STEP)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_SINGLE_STEP",_coreDebugger->ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
		else if(_coreDebugger->ExceptionHandler[i].dwExceptionType == EXCEPTION_ACCESS_VIOLATION)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_ACCESS_VIOLATION",_coreDebugger->ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
		else if(_coreDebugger->ExceptionHandler[i].dwExceptionType == EXCEPTION_PRIV_INSTRUCTION)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_PRIV_INSTRUCTION",_coreDebugger->ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
		else if(_coreDebugger->ExceptionHandler[i].dwExceptionType == EXCEPTION_ILLEGAL_INSTRUCTION)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_ILLEGAL_INSTRUCTION",_coreDebugger->ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
		else if(_coreDebugger->ExceptionHandler[i].dwExceptionType == EXCEPTION_INT_DIVIDE_BY_ZERO)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_INT_DIVIDE_BY_ZERO",_coreDebugger->ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
		else
		{
			wsprintf(cTemp,L"%s=%08X:%d\n",L"CUSTOM_EXCEPTION",_coreDebugger->ExceptionHandler[i].dwExceptionType,_coreDebugger->ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
	}

	wsprintf(cTemp,L"%s=%d\n",L"DefaultExceptionMode",_coreDebugger->dbgSettings.dwDefaultExceptionMode);
	outfile.write(cTemp,wcslen(cTemp));

	wsprintf(cTemp,L"%s=%d\n",L"BreakOnEPMode",_coreDebugger->dbgSettings.dwBreakOnEPMode);
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

	outfile.close();
	clsMemManager::CFree(cTemp);
	return true;
}

bool clsHelperClass::ReadFromSettingsFile(clsDebugger *_coreDebugger,qtNanomiteDisAsColorSettings *qtNanomiteDisAsColor)
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
		else if(sSettingLine[0] == L"EXCEPTION_ACCESS_VIOLATION")
			_coreDebugger->CustomExceptionAdd(EXCEPTION_ACCESS_VIOLATION,_wtoi(sSettingLine[1].c_str()),NULL);
		else if(sSettingLine[0] == L"EXCEPTION_PRIV_INSTRUCTION")
			_coreDebugger->CustomExceptionAdd(EXCEPTION_PRIV_INSTRUCTION,_wtoi(sSettingLine[1].c_str()),NULL);
		else if(sSettingLine[0] == L"EXCEPTION_ILLEGAL_INSTRUCTION")
			_coreDebugger->CustomExceptionAdd(EXCEPTION_ILLEGAL_INSTRUCTION,_wtoi(sSettingLine[1].c_str()),NULL);
		else if(sSettingLine[0] == L"EXCEPTION_INT_DIVIDE_BY_ZERO")
			_coreDebugger->CustomExceptionAdd(EXCEPTION_INT_DIVIDE_BY_ZERO,_wtoi(sSettingLine[1].c_str()),NULL);
		else if(sSettingLine[0] == L"BreakOnEPMode")
			_coreDebugger->dbgSettings.dwBreakOnEPMode = _wtoi(sSettingLine[1].c_str());
		else if(sSettingLine[0] == L"DefaultExceptionMode")
			_coreDebugger->dbgSettings.dwDefaultExceptionMode = _wtoi(sSettingLine[1].c_str());
		else if(sSettingLine[0] == L"SUSPENDTYPE")
			_coreDebugger->dbgSettings.dwSuspendType = _wtoi(sSettingLine[1].c_str());
		else if(sSettingLine[0] == L"CUSTOM_EXCEPTION")
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
	}
	infile.close();
	return true;
}

bool clsHelperClass::MenuLoadNewFile(clsDebugger *_coreDebugger)
{
	QString fileName = QFileDialog::getOpenFileName(NULL,"Please select a Target",QDir::currentPath(),"Executables (*.exe)");

	wstring *pFile = new wstring(fileName.toStdWString());

	if(!fileName.isEmpty())
		_coreDebugger->SetTarget(*pFile);
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
	size_t newSize = wcstombs(NULL, FileName.c_str(), 0) + 2;
	char* newStr = (char*)clsMemManager::CAlloc(newSize);
	wcstombs(newStr, FileName.c_str(), newSize);
	string str = newStr;
	clsMemManager::CFree(newStr);
	return str;
}

wstring clsHelperClass::convertSTRtoWSTR(string FileName)
{
	size_t newSize = mbstowcs(NULL, FileName.c_str(), 0) + 2;
	wchar_t* newStr = (wchar_t*)clsMemManager::CAlloc(newSize);
	mbstowcs(newStr, FileName.c_str(), newSize);
	wstring str = newStr;
	clsMemManager::CFree(newStr);
	return str;
}

PTCHAR clsHelperClass::reverseStrip(PTCHAR lpString, TCHAR lpSearchString)
{
	int iModPos = NULL,
		iModLen = NULL;
	PTCHAR lpTempString = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

	iModLen = wcslen(lpString);
	if(iModLen > 0)
	{
		for(int i = iModLen; i > 0 ; i--)
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

	return	"QTableWidget, QTreeWidget"
			"{ "
			"	background: rgb(230, 235, 230);"
			"	font-family: Consolas;"
			"	font-size: 12;"
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

wstring clsHelperClass::replaceAll(wstring orgString, wstring oldString, wstring newString)
{
	QString org = QString().fromStdWString(orgString);
	QString old = QString().fromStdWString(oldString);
	QString news = QString().fromStdWString(newString);

	wstring* ret = new wstring(org.replace(old,news).toStdWString()); 
	return *ret;
}

bool clsHelperClass::SetThreadPriorityByTid(DWORD ThreadID, int threadPrio)
{
	HANDLE hThread = OpenThread(THREAD_SET_INFORMATION,false,ThreadID);
	if(hThread == INVALID_HANDLE_VALUE) return false;

	bool bSuccess = false;
	if(!(bSuccess = SetThreadPriority(hThread,threadPrio)))
		MessageBoxW(NULL,L"ERROR, could not set the thread priority",L"Nanomite",MB_OK);
	CloseHandle(hThread);
	return bSuccess;
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

//quint64 clsHelperClass::GetImageBaseFromModuleName(QString moduleName,bool is64Bit)
//{
//	qtDLGNanomite *pMainGUI = qtDLGNanomite::GetInstance();
//
//	for(int i = 0; i < pMainGUI->dlgDetInfo->tblModules->rowCount(); i++)
//	{
//		if(is64Bit)
//		{
//			if(pMainGUI->dlgDetInfo->tblModules->item(i,3)->text().toLower().contains(moduleName.toLower()) &&
//				pMainGUI->dlgDetInfo->tblModules->item(i,3)->text().contains("System32") &&
//				pMainGUI->dlgDetInfo->tblModules->item(i,2)->text().contains("Loaded"))
//			{
//				return pMainGUI->dlgDetInfo->tblModules->item(i,1)->text().toULongLong(0,16);
//			}
//		}
//		else
//		{
//			if(pMainGUI->dlgDetInfo->tblModules->item(i,3)->text().toLower().contains(moduleName.toLower()) &&
//				pMainGUI->dlgDetInfo->tblModules->item(i,3)->text().contains("WOW64") &&
//				pMainGUI->dlgDetInfo->tblModules->item(i,2)->text().contains("Loaded"))
//			{
//				return pMainGUI->dlgDetInfo->tblModules->item(i,1)->text().toULongLong(0,16);
//			}
//		}
//	}
//	return 0;
//}