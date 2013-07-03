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
#include "clsUpdater.h"
#include "clsMemManager.h"

#include <Windows.h>

clsUpdater::clsUpdater(const QString &fileName)
	: m_fileName(fileName)
{

}

void clsUpdater::launchUpdater()
{
	this->start();
}

void clsUpdater::run()
{
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si,sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi,sizeof(pi));
	DWORD exitCode = 0;

	if(!CreateProcess(m_fileName.toStdWString().c_str(),NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,NULL,&si,&pi)) 
    {
        MessageBoxW(NULL,L"Error, unable to launch updater!",L"Nanomite",MB_OK);
		return;
    }

    WaitForSingleObject(pi.hProcess,INFINITE);
	GetExitCodeProcess(pi.hProcess,&exitCode);
	
	switch(exitCode)
	{
	case 0: // 0 - return if all is OK
		break;
	case 1: // 1 - if there is some error
		MessageBoxW(NULL,L"Error, some undefined error happend while updating",L"Nanomite",MB_OK);
		break;
	case 2: // 2 - if need to update updater.exe ( this case is handle inner of updater.exe )
		break;
	case 3: // 3 - means that this is update of QtNanomite.exe and for install QtNanomite.exe need to close process of QtNanomite.exe.
		break;
	}

	CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
