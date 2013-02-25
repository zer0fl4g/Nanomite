#ifndef CLSDBMANAGER_H
#define CLSDBMANAGER_H

#include "clsDBInterface.h"

#include <vector>

class clsDBManager
{
public:

	struct DB
	{
		int PID;
		clsDBInterface *DBi;
	};

	clsDBManager();
	~clsDBManager();

	static bool DBAPI_getSymbolsFromPID(int PID,unsigned long long Key,std::wstring &ModName,std::wstring &FuncName);
	static bool DBAPI_insertSymbolsFromPID(int PID,unsigned long long Key,std::wstring ModName,std::wstring FuncName);
	static bool OpenNewFile(int PID,std::wstring FilePath);
	static bool CloseFile(int PID);

private:
	static clsDBManager* pThis;
	
	std::vector<DB> DBs;

};

#endif