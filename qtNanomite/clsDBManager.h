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