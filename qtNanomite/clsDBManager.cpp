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
	if(pThis == NULL) return false;

	for(int i = 0; i < pThis->DBs.size(); i++)
		if(pThis->DBs[i].PID == PID)
			return pThis->DBs[i].DBi->DBAPI_getSymbols(Key,ModName,FuncName);

	return false;
}

bool clsDBManager::DBAPI_insertSymbolsFromPID(int PID,unsigned long long Key,std::wstring ModName,std::wstring FuncName)
{
	if(pThis == NULL) return false;

	for(int i = 0; i < pThis->DBs.size(); i++)
		if(pThis->DBs[i].PID == PID)
			return pThis->DBs[i].DBi->DBAPI_insertSymbols(Key,ModName,FuncName);

	return false;
}

//bool clsDBManager::DBAPI_insertTraceInfo(unsigned long long currentOffset,int PID,int TID,std::wstring currentInstruction,std::wstring currentRegs)
//{
//	if(pThis != NULL)
//		for(int i = 0; i < pThis->DBs.size(); i++)
//			if(pThis->DBs[i].PID == PID)
//				return pThis->DBs[i].DBi->DBAPI_insertTraceInfo(currentOffset,PID,TID,currentInstruction,currentRegs);
//	return false;
//}
//
//bool clsDBManager::DBAPI_getTraceInfo(int count,unsigned long long startOffset,int &PID, int &TID,vector<wstring> instructions, vector<wstring> regs)
//{
//	if(pThis != NULL)
//		for(int i = 0; i < pThis->DBs.size(); i++)
//			if(pThis->DBs[i].PID == PID)
//				return pThis->DBs[i].DBi->DBAPI_getTraceInfo(count,startOffset,PID,TID,instructions,regs);
//	return false;
//}

bool clsDBManager::OpenNewFile(int PID, wstring FilePath)
{
	if(pThis == NULL) return false;

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
	if(pThis == NULL) return false;
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

	return false;
}