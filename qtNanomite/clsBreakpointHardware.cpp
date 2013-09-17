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
#include "clsMemManager.h"
#include "clsBreakpointHardware.h"
#include "clsDebugger\clsDebugger.h"
#include "clsAPIImport.h"

#include <TlHelp32.h>

bool clsBreakpointHardware::wHardwareBP(DWORD processID, DWORD64 breakpointOffset, DWORD breakpointSize, DWORD breakpointSlot, DWORD typeFlag)
{
	if(!(breakpointSize == 1 || breakpointSize == 2 || breakpointSize == 4))
		return false; 

	if(!(typeFlag == BP_EXEC || typeFlag == BP_READ || typeFlag == BP_WRITE))
		return false;

	THREADENTRY32 threadEntry32;
	threadEntry32.dwSize = sizeof(THREADENTRY32);

	CONTEXT cTT;
	cTT.ContextFlags = CONTEXT_ALL;
	WOW64_CONTEXT cTTwow;
	cTTwow.ContextFlags = CONTEXT_ALL;

	BOOL isWOW64 = false;
#ifdef _AMD64_
	clsAPIImport::pIsWow64Process(clsDebugger::GetProcessHandleByPID(processID), &isWOW64);
#endif

	if(processID == -1)
		processID = clsDebugger::GetCurrentPID();

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,processID);
	if(hProcessSnap == INVALID_HANDLE_VALUE)
		return false;

	if(!Thread32First(hProcessSnap,&threadEntry32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	HANDLE hThread = INVALID_HANDLE_VALUE;
	do{
		if(processID == threadEntry32.th32OwnerProcessID)
			hThread = OpenThread(THREAD_GETSET_CONTEXT,false,threadEntry32.th32ThreadID);

		if(hThread != INVALID_HANDLE_VALUE)
		{
			SuspendThread(hThread);
		
			if(isWOW64)
			{
				clsAPIImport::pWow64GetThreadContext(hThread,&cTTwow);

				switch(breakpointSlot)
				{
				case 0:
					cTTwow.Dr0 = breakpointOffset;
					break;
				case 1:
					cTTwow.Dr1 = breakpointOffset;
					break;
				case 2:
					cTTwow.Dr2 = breakpointOffset;
					break;
				case 3:
					cTTwow.Dr3 = breakpointOffset;
					break;
				}

				cTTwow.Dr7 |= 1 << (breakpointSlot * 2);
				cTTwow.Dr7 |= typeFlag << ((breakpointSlot * 4) + 16);
				cTTwow.Dr7 |= (breakpointSize - 1) << ((breakpointSlot * 4) + 18);
				
				clsAPIImport::pWow64SetThreadContext(hThread,&cTTwow);
			}
			else
			{
				GetThreadContext(hThread,&cTT);

				switch(breakpointSlot)
				{
				case 0:
					cTT.Dr0 = breakpointOffset;
					break;
				case 1:
					cTT.Dr1 = breakpointOffset;
					break;
				case 2:
					cTT.Dr2 = breakpointOffset;
					break;
				case 3:
					cTT.Dr3 = breakpointOffset;
					break;
				}

				cTT.Dr7 |= 1 << (breakpointSlot * 2);
				cTT.Dr7 |= typeFlag << ((breakpointSlot * 4) + 16);
				cTT.Dr7 |= (breakpointSize - 1) << ((breakpointSlot * 4) + 18);

				SetThreadContext(hThread,&cTT);
			}

			ResumeThread(hThread);
			CloseHandle(hThread);
			hThread = INVALID_HANDLE_VALUE;

			if(clsDebugger::GetCurrentTID() == threadEntry32.th32ThreadID)
				clsDebugger::SetNewThreadContext(isWOW64, cTT, cTTwow);
		}
	}while(Thread32Next(hProcessSnap,&threadEntry32));

	CloseHandle(hProcessSnap);
	return true;
}

bool clsBreakpointHardware::dHardwareBP(DWORD processID, DWORD64 breakpointOffset, DWORD breakpointSlot)
{
	if(processID == 0 || breakpointOffset == 0)
		return false;

	THREADENTRY32 threadEntry32;
	threadEntry32.dwSize = sizeof(THREADENTRY32);

	CONTEXT cTT;
	cTT.ContextFlags = CONTEXT_ALL;
	WOW64_CONTEXT cTTwow;
	cTTwow.ContextFlags = CONTEXT_ALL;

	BOOL isWOW64 = false;

#ifdef _AMD64_
	clsAPIImport::pIsWow64Process(clsDebugger::GetProcessHandleByPID(processID), &isWOW64);
#endif

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,processID);
	if(hProcessSnap == INVALID_HANDLE_VALUE)
		return false;

	if(!Thread32First(hProcessSnap,&threadEntry32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	HANDLE hThread = INVALID_HANDLE_VALUE;
	do{
		if(processID == threadEntry32.th32OwnerProcessID)
			hThread = OpenThread(THREAD_GETSET_CONTEXT,false,threadEntry32.th32ThreadID);

		if(hThread != INVALID_HANDLE_VALUE)
		{
			SuspendThread(hThread);

			if(isWOW64)
			{
				clsAPIImport::pWow64GetThreadContext(hThread,&cTTwow);
				
				if(cTTwow.Dr0 == breakpointOffset) cTTwow.Dr0 = 0;
				else if(cTTwow.Dr1 == breakpointOffset) cTTwow.Dr1 = 0;
				else if(cTTwow.Dr2 == breakpointOffset) cTTwow.Dr2 = 0;
				else if(cTTwow.Dr3 == breakpointOffset) cTTwow.Dr3 = 0;

				cTTwow.Dr7 &= ~(1 << (breakpointSlot * 2));
				cTTwow.Dr7 &= ~(3 << ((breakpointSlot * 4) + 16));
				cTTwow.Dr7 &= ~(3 << ((breakpointSlot * 4) + 18));

				clsAPIImport::pWow64SetThreadContext(hThread,&cTTwow);
			}
			else
			{
				GetThreadContext(hThread,&cTT);

				if(cTT.Dr0 == breakpointOffset) cTT.Dr0 = 0;
				else if(cTT.Dr1 == breakpointOffset) cTT.Dr1 = 0;
				else if(cTT.Dr2 == breakpointOffset) cTT.Dr2 = 0;
				else if(cTT.Dr3 == breakpointOffset) cTT.Dr3 = 0;

				cTT.Dr7 &= ~(1 << (breakpointSlot * 2));
				cTT.Dr7 &= ~(3 << ((breakpointSlot * 4) + 16));
				cTT.Dr7 &= ~(3 << ((breakpointSlot * 4) + 18));

				SetThreadContext(hThread,&cTT);
			}

			ResumeThread(hThread);
			CloseHandle(hThread);
			hThread = INVALID_HANDLE_VALUE;

			if(clsDebugger::GetCurrentTID() == threadEntry32.th32ThreadID)
				clsDebugger::SetNewThreadContext(isWOW64, cTT, cTTwow);
		}
	}while(Thread32Next(hProcessSnap,&threadEntry32));

	CloseHandle(hProcessSnap);
	return true;
}