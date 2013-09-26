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
#ifndef qHelperClass
#define qHelperClass

#include <QStringList>
#include <QString>

#include "clsPEManager.h"
#include "clsDebugger/clsDebugger.h"
#include "qtDLGNanomite.h"

class clsHelperClass
{
public:
	clsHelperClass();
	~clsHelperClass();

	static bool IsWindowsXP();
	static bool SetThreadPriorityByTid(DWORD ThreadID, int threadPrio);
	static bool LoadSymbolForAddr(std::wstring& sFuncName,std::wstring& sModName,quint64 dwOffset,HANDLE hProc);
	static DWORD GetMainThread(DWORD ThreadID);
	static DWORD64 RemoteGetProcAddr(QString apiName, quint64 moduleBase, quint64 processID);
	static void LoadSourceForAddr(std::wstring &FileName,int &LineNumber,quint64 dwOffset,HANDLE hProc);
	static std::vector<std::wstring> split(const std::wstring& s,const std::wstring& f);
	static std::string convertWSTRtoSTR(std::wstring FileName);
	static std::wstring convertSTRtoWSTR(std::string FileName);
	static std::wstring replaceAll(std::wstring orgString,wchar_t oldString,wchar_t newString);
	static PTCHAR reverseStrip(PTCHAR lpString, TCHAR lpSearchString);
	static QString ResolveShortcut(QString shortcutFile);
	static QString LoadStyleSheet();
	static quint64 CalcOffsetForModule(PTCHAR moduleName,quint64 Offset,DWORD PID);
	//static quint64 GetImageBaseFromModuleName(QString moduleName,bool is64Bit);
};

#endif // QTDLGNANOMITE_H
