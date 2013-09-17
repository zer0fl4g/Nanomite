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
#include "clsBreakpointMemory.h"
#include "clsDebugger\clsDebugger.h"

bool clsBreakpointMemory::wMemoryBP(DWORD processID, DWORD64 breakpointOffset, DWORD breakpointSize, DWORD typeFlag, DWORD *savedProtection)
{
	if(breakpointOffset == NULL || breakpointSize == NULL)
		return false;

	MEMORY_BASIC_INFORMATION MBI;
	DWORD	oldProtection	= NULL,
			newProtection	= NULL;
	HANDLE processHandle	= clsDebugger::GetProcessHandleByPID(processID);

	if(VirtualQueryEx(processHandle, (LPVOID)breakpointOffset, &MBI, sizeof(MBI)) <= 0)
		return false;

	switch(typeFlag)
	{
	case BP_ACCESS:
		newProtection = MBI.Protect | PAGE_GUARD;
		break;
	case BP_WRITE:
		newProtection = PAGE_EXECUTE_READ;//MBI.Protect | PAGE_GUARD;
		break;
	//case BP_READ:
	//	newProtection = MBI.Protect | PAGE_GUARD;
		break;
	case BP_EXEC:
		newProtection = PAGE_READWRITE;//MBI.Protect | PAGE_GUARD;
		break;
	default:
		newProtection = MBI.Protect | PAGE_GUARD;
		break;
	}
	
	if(!VirtualProtectEx(processHandle, (LPVOID)breakpointOffset, breakpointSize, newProtection, &oldProtection))
		return false;
	
	*savedProtection = MBI.Protect;
	return true;
}

bool clsBreakpointMemory::dMemoryBP(DWORD processID, DWORD64 breakpointOffset, DWORD breakpointSize, DWORD oldProtection)
{
	if(breakpointOffset == NULL || breakpointSize == NULL)
		return false;

	MEMORY_BASIC_INFORMATION MBI;
	DWORD tmpOldProtection	= NULL;
	HANDLE processHandle	= clsDebugger::GetProcessHandleByPID(processID);

	if(VirtualQueryEx(processHandle, (LPVOID)breakpointOffset, &MBI, sizeof(MBI)) <= 0)
		return false;

	if(!VirtualProtectEx(processHandle, (LPVOID)breakpointOffset, breakpointSize, oldProtection, &tmpOldProtection))
		return false;

	return true;
}