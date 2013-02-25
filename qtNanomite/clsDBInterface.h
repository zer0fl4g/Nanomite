#ifndef CLSDBINTERFACE_H
#define CLSDBINTERFACE_H

#include <sqlite3.h>
#include <string>

class clsDBInterface
{

public:
	clsDBInterface(std::wstring FilePath);
	~clsDBInterface();
	
	bool DBAPI_getSymbols(unsigned long long Key,std::wstring &ModName,std::wstring &FuncName);
	bool DBAPI_insertSymbols(unsigned long long Key,std::wstring ModName,std::wstring FuncName);

private:
	bool _isDBOpen;

	sqlite3 *pDB;

	std::string getFileNameFromPath(std::wstring FileName);
};

#endif