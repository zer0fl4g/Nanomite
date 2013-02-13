#ifndef CLSDBINTERFACE_H
#define CLSDBINTERFACE_H

#include <sqlite3.h>
#include <string>

class clsDBInterface
{

public:
	int _PID;
	
	clsDBInterface(std::wstring FilePath,int PID);
	~clsDBInterface();
	
	static bool DBAPI_getSymbols(unsigned long long Key,std::wstring &ModName,std::wstring &FuncName);
	static bool DBAPI_insertSymbols(unsigned long long Key,std::wstring ModName,std::wstring FuncName);
	
	static clsDBInterface* GetInstance();

private:
	static clsDBInterface *pThis;

	std::string _FileName;

	bool _isDBOpen;

	sqlite3 *pDB;

	std::string getFileNameFromPath(std::wstring FileName);
};

#endif