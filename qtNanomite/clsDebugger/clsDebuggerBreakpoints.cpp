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
#include "clsDebugger.h"
#include "clsMemManager.h"
#include "clsHelperClass.h"
#include "clsAPIImport.h"

#include <TlHelp32.h>

using namespace std;

bool clsDebugger::wSoftwareBP(DWORD processID, quint64 breakpointOffset, DWORD breakpointSize, BYTE &dataBackup)
{
	if(breakpointOffset == 0 || breakpointSize <= 0)
		return false;

	SIZE_T	bytesWritten			= NULL,
			bytesRead				= NULL;
	BYTE	breakpointData			= 0xCC,
			breakpointDataBackup	= NULL;
	DWORD	oldProtection			= NULL,
			newProtection			= PAGE_READWRITE;
	HANDLE	processHandle			= m_dbgPI.hProcess;
	bool	returnValue				= false;

	if(processID != -1)
	{
		for(size_t i = 0;i < PIDs.size();i++)
		{
			if(processID == PIDs[i].dwPID)
			{
				processHandle = PIDs[i].hProc;
				break;
			}
		}
	}

	if(VirtualProtectEx(processHandle, (LPVOID)breakpointOffset, breakpointSize, newProtection, &oldProtection) &&
		ReadProcessMemory(processHandle, (LPVOID)breakpointOffset, (LPVOID)&breakpointDataBackup, breakpointSize, &bytesRead))
	{
		if(dataBackup == NULL)
			dataBackup = breakpointDataBackup;

		if(WriteProcessMemory(processHandle, (LPVOID)breakpointOffset, (LPVOID)&breakpointData, breakpointSize, &bytesWritten))
			returnValue = true;
		
		VirtualProtectEx(processHandle, (LPVOID)breakpointOffset, breakpointSize, oldProtection, &newProtection);
	}
	
	return returnValue;
}

bool clsDebugger::wMemoryBP(DWORD processID, quint64 breakpointOffset, DWORD breakpointSize, DWORD typeFlag, DWORD *savedProtection)
{
	if(breakpointOffset == NULL || breakpointSize == NULL)
		return false;

	MEMORY_BASIC_INFORMATION MBI;
	DWORD	oldProtection	= NULL,
			newProtection	= NULL;
	HANDLE processHandle	= m_dbgPI.hProcess;

	if(processID != -1)
	{
		for(size_t i = 0;i < PIDs.size(); i++)
		{
			if(PIDs[i].dwPID == processID)
			{
				processHandle = PIDs[i].hProc;
				break;
			}
		}
	}

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

bool clsDebugger::wHardwareBP(DWORD processID, quint64 breakpointOffset, DWORD breakpointSize, DWORD breakpointSlot, DWORD typeFlag)
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
		processID = m_dbgPI.dwProcessId;

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

bool clsDebugger::dHardwareBP(DWORD processID, quint64 breakpointOffset, DWORD breakpointSlot)
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

bool clsDebugger::dSoftwareBP(DWORD processID, quint64 breakpointOffset, DWORD breakpointSize, BYTE orgBreakpointData)
{
	if(breakpointOffset == 0 || orgBreakpointData == NULL)
		return false;

	SIZE_T	bytesWritten	= NULL;
	HANDLE	processHandle	= m_dbgPI.hProcess;
	DWORD	oldProtection	= NULL,
			newProtection	= PAGE_READWRITE;

	if(processID != -1)
	{
		for(size_t i = 0;i < PIDs.size();i++)
		{
			if(processID == PIDs[i].dwPID)
			{
				processHandle = PIDs[i].hProc;
				break;
			}
		}
	}

	if(VirtualProtectEx(processHandle, (LPVOID)breakpointOffset, breakpointSize, newProtection, &oldProtection) &&
		WriteProcessMemory(processHandle, (LPVOID)breakpointOffset, (LPVOID)&orgBreakpointData, breakpointSize, &bytesWritten))
	{
		return VirtualProtectEx(processHandle, (LPVOID)breakpointOffset, breakpointSize, oldProtection, &newProtection);
	}

	return false;
}

bool clsDebugger::dMemoryBP(DWORD processID, quint64 breakpointOffset, DWORD breakpointSize, DWORD oldProtection)
{
	if(breakpointOffset == NULL || breakpointSize == NULL)
		return false;

	MEMORY_BASIC_INFORMATION MBI;
	DWORD tmpOldProtection	= NULL;
	HANDLE processHandle	= m_dbgPI.hProcess;

	if(processID != -1)
	{
		for(size_t i = 0;i < PIDs.size(); i++)
		{
			if(PIDs[i].dwPID == processID)
			{
				processHandle = PIDs[i].hProc;
				break;
			}
		}	
	}

	if(VirtualQueryEx(processHandle, (LPVOID)breakpointOffset, &MBI, sizeof(MBI)) <= 0)
		return false;

	if(!VirtualProtectEx(processHandle, (LPVOID)breakpointOffset, breakpointSize, oldProtection, &tmpOldProtection))
		return false;

	return true;
}

bool clsDebugger::InitBP()
{
	for(size_t i = 0;i < SoftwareBPs.size(); i++)
		wSoftwareBP(SoftwareBPs[i].dwPID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwSize,SoftwareBPs[i].bOrgByte);

	for(size_t i = 0;i < MemoryBPs.size(); i++)
		wMemoryBP(MemoryBPs[i].dwPID,MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].dwTypeFlag,&MemoryBPs[i].dwOldProtection);

	for(size_t i = 0;i < HardwareBPs.size(); i++)
		wHardwareBP(HardwareBPs[i].dwPID,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSize,HardwareBPs[i].dwSlot,HardwareBPs[i].dwTypeFlag);
	return true;
}

bool clsDebugger::RemoveBPs()
{
	for(size_t i = 0; i < SoftwareBPs.size(); i++)
		RemoveBPFromList(SoftwareBPs[i].dwOffset,SOFTWARE_BP);

	for(size_t i = 0; i < MemoryBPs.size(); i++)
		RemoveBPFromList(MemoryBPs[i].dwOffset,MEMORY_BP);

	for(size_t i = 0; i < HardwareBPs.size(); i++)	
		RemoveBPFromList(HardwareBPs[i].dwOffset,HARDWARE_BP);

	SoftwareBPs.clear();
	HardwareBPs.clear();
	MemoryBPs.clear();
	return true;
}

bool clsDebugger::RemoveBPFromList(quint64 breakpointOffset, DWORD breakpointType) //,DWORD dwPID)
{ 
	switch(breakpointType)
	{
	case 0:
		for (vector<BPStruct>::iterator it = SoftwareBPs.begin();it != SoftwareBPs.end(); ++it)
		{
			if(it->dwOffset == breakpointOffset /* && it->dwPID == dwPID */)
			{
				dSoftwareBP(it->dwPID,it->dwOffset,it->dwSize,it->bOrgByte);
				clsMemManager::CFree(it->moduleName);

				SoftwareBPs.erase(it);
				it = SoftwareBPs.begin();

				if(it == SoftwareBPs.end())
					break;
			}
		}
		break;

	case 1:
		for (vector<BPStruct>::iterator it = MemoryBPs.begin();it != MemoryBPs.end(); ++it)
		{
			if(it->dwOffset == breakpointOffset /* && it->dwPID == dwPID */)
			{
				dMemoryBP(it->dwPID,it->dwOffset,it->dwSize,it->dwOldProtection);
				clsMemManager::CFree(it->moduleName);

				MemoryBPs.erase(it);
				it = MemoryBPs.begin();

				if(it == MemoryBPs.end())
					break;
			}
		}
		break;

	case 2:
		for (vector<BPStruct>::iterator it = HardwareBPs.begin();it != HardwareBPs.end(); ++it)
		{
			if(it->dwOffset == breakpointOffset /* && it->dwPID == dwPID */)
			{
				dHardwareBP(it->dwPID,it->dwOffset,it->dwSlot);
				clsMemManager::CFree(it->moduleName);

				HardwareBPs.erase(it);
				it = HardwareBPs.begin();

				if(it == HardwareBPs.end())
					break;
			}
		}
		break;
	}
	
	emit OnBreakpointDeleted(breakpointOffset);

	return true;
}

bool clsDebugger::AddBreakpointToList(DWORD breakpointType, DWORD typeFlag, DWORD processID, quint64 breakpointOffset, DWORD breakpointHandleType)
{
	bool	bExists		= false,
			bRetValue	= false;

	for(size_t i = 0;i < SoftwareBPs.size();i++)
	{
		if(SoftwareBPs[i].dwOffset == breakpointOffset/* && SoftwareBPs[i].dwPID == dwPID */)
		{
			bExists = true;
			break;
		}
	}
	for(size_t i = 0;i < MemoryBPs.size();i++)
	{
		if(MemoryBPs[i].dwOffset == breakpointOffset /* && MemoryBPs[i].dwPID == dwPID*/)
		{
			bExists = true;
			break;
		}
	}
	for(size_t i = 0;i < HardwareBPs.size();i++)
	{
		if(HardwareBPs[i].dwOffset == breakpointOffset /* && HardwareBPs[i].dwPID == dwPID */)
		{
			bExists = true;
			break;
		}
	}

	if(!bExists)
	{
		DWORD dwSize = 1;

		switch(breakpointType)
		{
		case SOFTWARE_BP:
			{
				BPStruct newBP;
				ZeroMemory(&newBP,sizeof(BPStruct));

				if(!wSoftwareBP(processID, breakpointOffset, dwSize, newBP.bOrgByte))
					break;

				newBP.dwOffset = breakpointOffset;
				newBP.dwHandle = breakpointHandleType;
				newBP.dwSize = dwSize;
				newBP.dwPID = processID;
				newBP.moduleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
				ZeroMemory(newBP.moduleName,MAX_PATH * sizeof(TCHAR));

				if(breakpointHandleType == BP_KEEP)
					newBP.dwBaseOffset = clsHelperClass::CalcOffsetForModule(newBP.moduleName,newBP.dwOffset,newBP.dwPID);

				SoftwareBPs.push_back(newBP);

				emit OnNewBreakpointAdded(newBP,SOFTWARE_BP);
				bRetValue = true;

				break;
			}
		case MEMORY_BP:
			{
				DWORD oldProtection = NULL;

				if(!wMemoryBP(processID, breakpointOffset, dwSize, typeFlag, &oldProtection))
					break;

				BPStruct newBP;
				ZeroMemory(&newBP,sizeof(BPStruct));

				newBP.dwOffset = breakpointOffset;
				newBP.dwHandle = breakpointHandleType;
				newBP.dwSize = dwSize;
				newBP.dwPID = processID;
				newBP.dwTypeFlag = typeFlag;
				newBP.dwOldProtection = oldProtection;
				newBP.moduleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
				ZeroMemory(newBP.moduleName,MAX_PATH * sizeof(TCHAR));
				
				if(breakpointHandleType == BP_KEEP)
					newBP.dwBaseOffset = clsHelperClass::CalcOffsetForModule(newBP.moduleName,newBP.dwOffset,newBP.dwPID);
				
				MemoryBPs.push_back(newBP);
				emit OnNewBreakpointAdded(newBP,MEMORY_BP);
				bRetValue = true;

				break;
			}
		case HARDWARE_BP:
			{
				if(HardwareBPs.size() == 4)
					break;

				BPStruct newBP;
				ZeroMemory(&newBP,sizeof(BPStruct));

				newBP.dwOffset = breakpointOffset;
				newBP.dwHandle = breakpointHandleType;
				newBP.dwSize = dwSize;
				newBP.dwPID = processID;
				newBP.dwTypeFlag = typeFlag;
				newBP.moduleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
				ZeroMemory(newBP.moduleName,MAX_PATH * sizeof(TCHAR));
			
				if(breakpointHandleType == BP_KEEP)
					newBP.dwBaseOffset = clsHelperClass::CalcOffsetForModule(newBP.moduleName,newBP.dwOffset,newBP.dwPID);

				bool bSlot1 = false,bSlot2 = false,bSlot3 = false,bSlot4 = false;
				for(size_t i = 0;i < HardwareBPs.size();i++)
				{
					switch(HardwareBPs[i].dwSlot)
					{
					case 0:
						bSlot1 = true;
						break;
					case 1:
						bSlot2 = true;
						break;
					case 2:
						bSlot3 = true;
						break;
					case 3:
						bSlot4 = true;
						break;
					}
				}
				if(!bSlot4) newBP.dwSlot = 3;
				else if(!bSlot3) newBP.dwSlot = 2;
				else if(!bSlot2) newBP.dwSlot = 1;
				else if(!bSlot1) newBP.dwSlot = 0;

				if(!wHardwareBP(processID, breakpointOffset, dwSize, newBP.dwSlot, typeFlag))
				{
					clsMemManager::CFree(newBP.moduleName);
					break;
				}

				HardwareBPs.push_back(newBP);
				emit OnNewBreakpointAdded(newBP,HARDWARE_BP);
				bRetValue = true;

				break;
			}
		}
	}

	return bRetValue;
}

void clsDebugger::UpdateOffsetsBPs()
{
	for(size_t i = 0;i < SoftwareBPs.size(); i++)
	{
		if(SoftwareBPs[i].dwHandle != BP_KEEP) continue;

		DWORD64 newBaseOffset = clsHelperClass::CalcOffsetForModule(SoftwareBPs[i].moduleName,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwPID);
		if(newBaseOffset != SoftwareBPs[i].dwOffset && newBaseOffset != SoftwareBPs[i].dwBaseOffset)
		{
			if(SoftwareBPs[i].dwBaseOffset == NULL)
			{
				SoftwareBPs[i].dwOldOffset = SoftwareBPs[i].dwOffset;
				SoftwareBPs[i].dwBaseOffset = newBaseOffset;
			}
			else
			{
				DWORD64 newOffset = (SoftwareBPs[i].dwOffset - SoftwareBPs[i].dwBaseOffset);

				SoftwareBPs[i].dwOldOffset = SoftwareBPs[i].dwOffset;
				SoftwareBPs[i].dwBaseOffset = newBaseOffset;
				SoftwareBPs[i].dwOffset = newOffset + newBaseOffset;
			}
			
			SoftwareBPs[i].dwHandle = BP_OFFSETUPDATE;
			emit OnNewBreakpointAdded(SoftwareBPs[i],0);
			SoftwareBPs[i].dwHandle = BP_KEEP;
		}
	}

	for(size_t i = 0;i < MemoryBPs.size(); i++)
	{
		if(MemoryBPs[i].dwHandle != BP_KEEP) continue;

		DWORD64 newBaseOffset = clsHelperClass::CalcOffsetForModule(MemoryBPs[i].moduleName,MemoryBPs[i].dwOffset,MemoryBPs[i].dwPID);
		if(newBaseOffset != MemoryBPs[i].dwOffset && newBaseOffset != MemoryBPs[i].dwBaseOffset)
		{
			if(MemoryBPs[i].dwBaseOffset == NULL)
			{
				MemoryBPs[i].dwOldOffset = MemoryBPs[i].dwOffset;
				MemoryBPs[i].dwBaseOffset = newBaseOffset;
			}
			else
			{
				DWORD64 newOffset = (MemoryBPs[i].dwOffset - MemoryBPs[i].dwBaseOffset);

				MemoryBPs[i].dwOldOffset = MemoryBPs[i].dwOffset;
				MemoryBPs[i].dwBaseOffset = newBaseOffset;
				MemoryBPs[i].dwOffset = newOffset + newBaseOffset;
			}

			MemoryBPs[i].dwHandle = BP_OFFSETUPDATE;
			emit OnNewBreakpointAdded(MemoryBPs[i],1);
			MemoryBPs[i].dwHandle = BP_KEEP;
		}
	}

	for(size_t i = 0;i < HardwareBPs.size(); i++)
	{
		if(HardwareBPs[i].dwHandle != BP_KEEP) continue;

		DWORD64 newBaseOffset = clsHelperClass::CalcOffsetForModule(HardwareBPs[i].moduleName,HardwareBPs[i].dwOffset,HardwareBPs[i].dwPID);
		if(newBaseOffset != HardwareBPs[i].dwOffset && newBaseOffset != HardwareBPs[i].dwBaseOffset)
		{
			if(HardwareBPs[i].dwBaseOffset == NULL)
			{
				HardwareBPs[i].dwOldOffset = HardwareBPs[i].dwOffset;
				HardwareBPs[i].dwBaseOffset = newBaseOffset;
			}
			else
			{
				DWORD64 newOffset = (HardwareBPs[i].dwOffset - HardwareBPs[i].dwBaseOffset);

				HardwareBPs[i].dwOldOffset = HardwareBPs[i].dwOffset;
				HardwareBPs[i].dwBaseOffset = newBaseOffset;
				HardwareBPs[i].dwOffset = newOffset + newBaseOffset;
			}

			HardwareBPs[i].dwHandle = BP_OFFSETUPDATE;
			emit OnNewBreakpointAdded(HardwareBPs[i],1);
			HardwareBPs[i].dwHandle = BP_KEEP;
		}
	}
}