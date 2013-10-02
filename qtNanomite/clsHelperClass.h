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
#ifndef CLSHELPERCLASS_H
#define CLSHELPERCLASS_H

#include <QStringList>
#include <QString>

#include <Windows.h>

class clsHelperClass
{
public:
	clsHelperClass();
	~clsHelperClass();

	static bool IsWindowsXP();
	static bool SetThreadPriorityByTid(DWORD ThreadID, int threadPrio);
	static bool LoadSymbolForAddr(QString &functionName, QString &moduleName, quint64 symbolOffset, HANDLE processHandle);
	static bool LoadSourceForAddr(QString &fileName, int &lineNumber, quint64 sourceOffset, HANDLE processHandle);
	static DWORD GetMainThread(DWORD ThreadID);
	static DWORD64 RemoteGetProcAddr(QString apiName, quint64 moduleBase, quint64 processID);
	static PTCHAR reverseStrip(PTCHAR lpString, TCHAR lpSearchString);
	static QString ResolveShortcut(QString shortcutFile);
	static QString LoadStyleSheet();
	static quint64 CalcOffsetForModule(PTCHAR moduleName,quint64 Offset,DWORD PID);
};

#endif
