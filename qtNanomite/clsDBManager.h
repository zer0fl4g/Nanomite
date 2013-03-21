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
//	static bool DBAPI_insertTraceInfo(unsigned long long currentOffset,int PID,int TID,std::wstring currentInstruction,std::wstring currentRegs);
//	static bool DBAPI_getTraceInfo(int count,unsigned long long startOffset,int &PID, int &TID,std::vector<std::wstring> instructions,std::vector<std::wstring> regs);
	static bool OpenNewFile(int PID,std::wstring FilePath);
	static bool CloseFile(int PID);

private:
	static clsDBManager* pThis;
	
	std::vector<DB> DBs;

};

#endif