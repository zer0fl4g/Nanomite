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
#ifndef clsCALLBACK
#define clsCALLBACK

#include <Windows.h>
#include <string>
#include <QTCore>

class clsCallbacks : public QObject
{
	Q_OBJECT
public:
	clsCallbacks();
	~clsCallbacks();

	static bool CALLBACK EnumWindowCallBack(HWND hWnd,LPARAM lParam);

public slots:
		int OnThread(DWORD dwPID,DWORD dwTID,quint64 dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
		int OnPID(DWORD dwPID,std::wstring sFile,DWORD dwExitCode,quint64 dwEP,bool bFound);
		int OnException(std::wstring sFuncName,std::wstring sModName,quint64 dwOffset,quint64 dwExceptionCode,DWORD dwPID,DWORD dwTID);
		int OnDbgString(std::wstring sMessage,DWORD dwPID);
		int OnDll(std::wstring sDLLPath,DWORD dwPID,quint64 dwEP,bool bLoaded);
};

#endif
