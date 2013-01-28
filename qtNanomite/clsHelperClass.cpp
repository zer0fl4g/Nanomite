#include "clsHelperClass.h"
#include "dbghelp.h"

#include <fstream>
#include <algorithm>

using namespace std;

clsHelperClass::clsHelperClass()
{
}

clsHelperClass::~clsHelperClass()
{
}

bool clsHelperClass::WriteToSettingsFile(clsDebugger *_coreDebugger)
{
	wofstream outfile;
	outfile.open("NanomiteConfig.ini");

	if(!outfile.is_open())
		return false;

	TCHAR cTemp[128];
	wsprintf(cTemp,L"%s=%s\n",L"DebugChilds",_coreDebugger->dbgSettings.bDebugChilds ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"AutoLoadSym",_coreDebugger->dbgSettings.bAutoLoadSymbols ? L"true" : L"false");
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

	outfile.close();
	return true;
}

bool clsHelperClass::ReadFromSettingsFile(clsDebugger *_coreDebugger)
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
	}
	infile.close();
	return true;
}

bool clsHelperClass::MenuLoadNewFile(clsDebugger *_coreDebugger)
{
	TCHAR cFile[MAX_PATH];

	OPENFILENAME ofn; 
	ZeroMemory(&ofn,sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);

	ofn.hwndOwner = NULL;
	ofn.lpstrFile = cFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(cFile);
	ofn.lpstrFilter = L"Executable Files (*.exe)\0*.exe\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn) == true)
		_coreDebugger->SetTarget(ofn.lpstrFile);
	else
	{
		MessageBox(NULL,L"No valid file!",L"Nanomite",MB_OK);
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

	IMAGEHLP_MODULEW64 imgMod = {0};
	imgMod.SizeOfStruct = sizeof(imgMod);
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
	return bTest;
}

