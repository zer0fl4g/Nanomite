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
#include "clsDBInterface.h"
#include "clsHelperClass.h"
#include "clsMemManager.h"

#include <QCryptographicHash>

using namespace std;

clsDBInterface::clsDBInterface(wstring FilePath)
{
	string FileName = getFileNameFromPath(FilePath);
	QString FileNameHash = QString(QCryptographicHash::hash(QString().fromStdString(FileName).toAscii(),QCryptographicHash::Md5).toHex());
	string *FileNameSTDHash = new string(FileNameHash.toStdString());
	
	FileName.append("_").append(*FileNameSTDHash).append(".sqlite");

	if(!sqlite3_open(FileName.c_str(),&pDB) == SQLITE_OK)
	{
		_isDBOpen = false;
		return;
	}
	_isDBOpen = true;

	sqlite3_exec(pDB,"PRAGMA junral_mode = OFF;",NULL,NULL,NULL);
	sqlite3_exec(pDB,"synchronous = OFF;",NULL,NULL,NULL);

	sqlite3_stmt *insertStmt;

	// symbol table
	string createTableQuery = "CREATE TABLE IF NOT EXISTS symbols (offset REAL PRIMARY KEY,modname TEXT,funcname TEXT);";
	sqlite3_prepare(pDB, createTableQuery.c_str(), createTableQuery.size(), &insertStmt, NULL);
    if (sqlite3_step(insertStmt) != SQLITE_DONE)
	{
		// only for debug
		return;
	}

	// drop previous traces
	createTableQuery = "DROP TABLE IF EXISTS trace;";
	sqlite3_prepare(pDB, createTableQuery.c_str(), createTableQuery.size(), &insertStmt, NULL);
	if (sqlite3_step(insertStmt) != SQLITE_DONE)
	{
		// only for debug
		return;
	}

	// create trace table
	createTableQuery = "CREATE TABLE IF NOT EXISTS trace (offset REAL PRIMARY KEY,PID TEXT,TID TEXT,instruction TEXT,regs TEXT);";
	sqlite3_prepare(pDB, createTableQuery.c_str(), createTableQuery.size(), &insertStmt, NULL);
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

	char* symbolQuery = (char*)clsMemManager::CAlloc(120);
	sprintf(symbolQuery,"select * from symbols where 'Offset' == %016I64X;",Key);

    sqlite3_prepare(pDB, symbolQuery, strlen(symbolQuery) + 1, &selectStmt, NULL);
    
	while(true)
	{
		int s = sqlite3_step(selectStmt);
		if (s == SQLITE_ROW)
		{
			ModName = clsHelperClass::convertSTRtoWSTR((const char*)sqlite3_column_text(selectStmt, 1));
			FuncName = clsHelperClass::convertSTRtoWSTR((const char*)sqlite3_column_text(selectStmt, 2));
		}
		else if (s == SQLITE_DONE)
			break;
		else
			break;
	}

	clsMemManager::CFree(symbolQuery);

	if(ModName.length() <= 0)
		return false;
	return true;
}

bool clsDBInterface::DBAPI_insertSymbols(quint64 Key,std::wstring ModName,std::wstring FuncName)
{
	sqlite3_stmt *insertStmt;

	char* insertQuery = (char*)clsMemManager::CAlloc(ModName.size() + FuncName.size() + 100);
	sprintf(insertQuery,"INSERT INTO symbols (offset,modname,funcname) VALUES ('%016I64X', '%s','%s');",
		Key,
		clsHelperClass::convertWSTRtoSTR(ModName).c_str(),
		clsHelperClass::convertWSTRtoSTR(FuncName).c_str());
    
    sqlite3_prepare(pDB, insertQuery, strlen(insertQuery) + 1, &insertStmt, NULL);
    if (sqlite3_step(insertStmt) != SQLITE_DONE)
	{
		clsMemManager::CFree(insertQuery);
		return false;
	}
	
	clsMemManager::CFree(insertQuery);
	return true;
}

string clsDBInterface::getFileNameFromPath(wstring FilePath)
{
	vector<wstring> TempData = clsHelperClass::split(FilePath,L"\\");
	return clsHelperClass::convertWSTRtoSTR(TempData[TempData.size() - 1]);
}

//bool clsDBInterface::DBAPI_insertTraceInfo(unsigned long long currentOffset,int PID,int TID,std::wstring currentInstruction,std::wstring currentRegs)
//{
//	sqlite3_stmt *insertStmt;
//
//	char* insertQuery = (char*)clsMemManager::CAlloc(sizeof(unsigned long long) + sizeof(int) + sizeof(int) + currentRegs.size() + currentInstruction.size() + 100);
//	sprintf(insertQuery,"INSERT INTO trace (offset,PID,TID,instruction,regs) VALUES ('%016I64X','%08X','%08X','%s','%s');",
//		currentOffset,
//		PID,
//		TID,
//		clsHelperClass::convertWSTRtoSTR(currentInstruction).c_str(),
//		clsHelperClass::convertWSTRtoSTR(currentRegs).c_str());
//
//	sqlite3_prepare(pDB, insertQuery, strlen(insertQuery) + 1, &insertStmt, NULL);
//	if (sqlite3_step(insertStmt) != SQLITE_DONE)
//	{
//		clsMemManager::CFree(insertQuery);
//		return false;
//	}
//
//	clsMemManager::CFree(insertQuery);
//	return true;
//}
//
//bool clsDBInterface::DBAPI_getTraceInfo(int count,unsigned long long startOffset,int &PID, int &TID,vector<wstring> instructions, vector<wstring> regs)
//{
//	sqlite3_stmt *selectStmt;
//
//	char* symbolQuery = (char*)clsMemManager::CAlloc(120);
//	sprintf(symbolQuery,"select * from trace where 'Offset' == %016I64X;",startOffset);
//
//	sqlite3_prepare(pDB, symbolQuery, strlen(symbolQuery) + 1, &selectStmt, NULL);
//
//	while(true)
//	{
//		int s = sqlite3_step(selectStmt);
//		if (s == SQLITE_ROW)
//		{
//			instructions.push_back(clsHelperClass::convertSTRtoWSTR((const char*)sqlite3_column_text(selectStmt, 3)));
//			regs.push_back(clsHelperClass::convertSTRtoWSTR((const char*)sqlite3_column_text(selectStmt, 4)));
//		}
//		else if (s == SQLITE_DONE)
//			break;
//		else
//			break;
//	}
//
//	clsMemManager::CFree(symbolQuery);
//
//	if(instructions.size() <= 0 || regs.size() <= 0)
//		return false;
//	return true;
//}