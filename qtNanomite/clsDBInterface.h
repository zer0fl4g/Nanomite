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