#include "clsDBInterface.h"
#include "clsHelperClass.h"

using namespace std;

clsDBInterface* clsDBInterface::pThis = NULL;

clsDBInterface::clsDBInterface(wstring FilePath,int PID)
{
	pThis = this;
	_PID = PID;

	string FileName = getFileNameFromPath(FilePath);
	FileName.append("_db_HASH_.sqlite");

	if(!sqlite3_open(FileName.c_str(),&pDB) == SQLITE_OK)
	{
		_isDBOpen = false;
		return;
	}
	_isDBOpen = true;

	sqlite3_exec(pDB,"PRAGMA junral_mode = OFF;",NULL,NULL,NULL);
	sqlite3_exec(pDB,"synchronous = OFF;",NULL,NULL,NULL);

	sqlite3_stmt *insertStmt;

	string createTableQuery = "CREATE TABLE IF NOT EXISTS symbols (offset REAL PRIMARY KEY,modname TEXT,funcname TEXT);";
    
	sqlite3_prepare(pThis->pDB, createTableQuery.c_str(), createTableQuery.size(), &insertStmt, NULL);
    if (sqlite3_step(insertStmt) != SQLITE_DONE)
	{
		// only for debug
		return;
	}
}

clsDBInterface::~clsDBInterface()
{
	if(_isDBOpen)
	{
		sqlite3_close(pDB);
	}
}

bool clsDBInterface::DBAPI_getSymbols(quint64 Key,std::wstring &ModName,std::wstring &FuncName)
{
	sqlite3_stmt *selectStmt;

	char* symbolQuery = (char*)malloc(120);
	sprintf(symbolQuery,"select * from symbols where 'Offset' == %016I64X;",Key);

    sqlite3_prepare(pThis->pDB, symbolQuery, strlen(symbolQuery) + 1, &selectStmt, NULL);
    
	while(true)
	{
		int s = sqlite3_step(selectStmt);
		if (s == SQLITE_ROW)
		{
			const unsigned char * text;
			ModName = clsHelperClass::convertSTRtoWSTR((const char*)sqlite3_column_text(selectStmt, 1));
			FuncName = clsHelperClass::convertSTRtoWSTR((const char*)sqlite3_column_text(selectStmt, 2));
		}
		else if (s == SQLITE_DONE)
			break;
		else
			break;
	}

	free(symbolQuery);

	if(ModName.length() <= 0)
		return false;
	return true;
}

bool clsDBInterface::DBAPI_insertSymbols(quint64 Key,std::wstring ModName,std::wstring FuncName)
{
	sqlite3_stmt *insertStmt;

	char* insertQuery = (char*)malloc(ModName.size() + FuncName.size() + 100);
	sprintf(insertQuery,"INSERT INTO symbols (offset,modname,funcname) VALUES ('%016I64X', '%s','%s');",
		Key,
		clsHelperClass::convertWSTRtoSTR(ModName).c_str(),
		clsHelperClass::convertWSTRtoSTR(FuncName).c_str());
    
    sqlite3_prepare(pThis->pDB, insertQuery, strlen(insertQuery) + 1, &insertStmt, NULL);
    if (sqlite3_step(insertStmt) != SQLITE_DONE)
	{
		free(insertQuery);
		return false;
	}
	
	free(insertQuery);
	return true;
}

clsDBInterface* clsDBInterface::GetInstance()
{
	return pThis;
}

string clsDBInterface::getFileNameFromPath(wstring FilePath)
{
	vector<wstring> TempData = clsHelperClass::split(FilePath,L"\\");
	return clsHelperClass::convertWSTRtoSTR(TempData[TempData.size() - 1]);
}