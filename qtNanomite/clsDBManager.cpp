#include "clsDBManager.h"
#include "clsMemManager.h"

using namespace std;

clsDBManager* clsDBManager::pThis = NULL;

clsDBManager::clsDBManager()
{
	pThis = this;
}

clsDBManager::~clsDBManager()
{
	for(int i = 0; i < DBs.size(); i++)
		delete pThis->DBs[i].DBi;
}

bool clsDBManager::DBAPI_getSymbolsFromPID(int PID,unsigned long long Key,std::wstring &ModName,std::wstring &FuncName)
{
	if(pThis != NULL)
		for(int i = 0; i < pThis->DBs.size(); i++)
			if(pThis->DBs[i].PID == PID)
				return pThis->DBs[i].DBi->DBAPI_getSymbols(Key,ModName,FuncName);

	return false;
}

bool clsDBManager::DBAPI_insertSymbolsFromPID(int PID,unsigned long long Key,std::wstring ModName,std::wstring FuncName)
{
	if(pThis != NULL)
		for(int i = 0; i < pThis->DBs.size(); i++)
			if(pThis->DBs[i].PID == PID)
				return pThis->DBs[i].DBi->DBAPI_insertSymbols(Key,ModName,FuncName);
	return false;
}

bool clsDBManager::OpenNewFile(int PID, wstring FilePath)
{
	if(pThis != NULL)
		for(int i = 0; i < pThis->DBs.size(); i++)
			if(pThis->DBs[i].PID == PID)
				return false; // does already exist

	DB newDB;
	newDB.PID = PID;
	newDB.DBi = new clsDBInterface(FilePath);

	pThis->DBs.push_back(newDB);

	return true;
}

bool clsDBManager::CloseFile(int PID)
{
	if(pThis != NULL)
	{
		if(pThis->DBs.size() <= 0) return false;

		for(vector<DB>::const_iterator it = pThis->DBs.cbegin(); it != pThis->DBs.cend(); ++it)
		{
			if(it->PID == PID)
			{
				delete it->DBi;
				pThis->DBs.erase(it);
				return true;
			}
		}
	}

	return false;
}