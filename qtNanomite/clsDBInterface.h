#ifndef CLSDBINTERFACE_H
#define CLSDBINTERFACE_H

#include <sqlite3.h>
#include <string>
#include <vector>

class clsDBInterface
{

public:
	clsDBInterface(std::wstring FilePath);
	~clsDBInterface();
	
	bool DBAPI_getSymbols(unsigned long long Key,std::wstring &ModName,std::wstring &FuncName);
	bool DBAPI_insertSymbols(unsigned long long Key,std::wstring ModName,std::wstring FuncName);
//	bool DBAPI_insertTraceInfo(unsigned long long currentOffset,int PID,int TID,std::wstring currentInstruction,std::wstring currentRegs);
//	bool DBAPI_getTraceInfo(int count,unsigned long long startOffset,int &PID, int &TID,std::vector<std::wstring> instructions,std::vector<std::wstring> regs);

private:
	bool _isDBOpen;

	sqlite3 *pDB;

	std::string getFileNameFromPath(std::wstring FileName);
};

#endif