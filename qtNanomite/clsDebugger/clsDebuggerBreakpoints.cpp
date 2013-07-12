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

#include <TlHelp32.h>

using namespace std;

bool clsDebugger::wSoftwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwKeep,DWORD dwSize,BYTE &bOrgByte)
{
	if(dwOffset == 0)
		return false;

	SIZE_T dwBytesWritten,dwBytesRead;
	BYTE bNew = 0xCC,bOld;

	HANDLE hPID = _pi.hProcess;

	if(dwPID != -1)
	{
		for(size_t i = 0;i < PIDs.size();i++)
		{
			if(dwPID == PIDs[i].dwPID)
				hPID = PIDs[i].hProc;
		}
	}

	if(ReadProcessMemory(hPID,(LPVOID)dwOffset,(LPVOID)&bOld,dwSize,&dwBytesRead))
	{
		if(bOld != 0xCC)
		{
			bOrgByte = bOld;
			if(WriteProcessMemory(hPID,(LPVOID)dwOffset,(LPVOID)&bNew,dwSize,&dwBytesWritten))
				return true;
		}
	}
	return false;
}

bool clsDebugger::wMemoryBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize,DWORD dwKeep)
{
	MEMORY_BASIC_INFORMATION MBI;
	DWORD dwOldProtection;
	HANDLE hPID = _pi.hProcess;

	for(size_t i = 0;i < PIDs.size(); i++)
		if(PIDs[i].dwPID == dwPID)
			hPID = PIDs[i].hProc;

	VirtualQueryEx(hPID,(LPVOID)dwOffset,&MBI,sizeof(MBI));

	if(!VirtualProtectEx(hPID,(LPVOID)dwOffset,dwSize,MBI.Protect | PAGE_GUARD,&dwOldProtection))
		return false;
	return true;
}

bool clsDebugger::wHardwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize,DWORD dwSlot,DWORD dwTypeFlag)
{
	size_t iBP = NULL;
	DWORD dwThreadCounter = 0;
	THREADENTRY32 threadEntry32;
	threadEntry32.dwSize = sizeof(THREADENTRY32);
	CONTEXT cTT;
	cTT.ContextFlags = CONTEXT_ALL;
	HANDLE hThread = INVALID_HANDLE_VALUE;

	if(dwPID == -1 || dwPID == 0)
		dwPID = _pi.dwProcessId;

	if(!(dwSize == 1 || dwSize == 2 || dwSize == 4))
		return false; 

	if(!(dwTypeFlag == DR_EXECUTE || dwTypeFlag == DR_READ || dwTypeFlag == DR_WRITE))
		return false;

	for(size_t i = 0;i < HardwareBPs.size();i++)
		if(HardwareBPs[i].dwOffset == dwOffset)
			iBP = i;

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,dwPID);
	if(hProcessSnap == INVALID_HANDLE_VALUE)
		return false;

	if(!Thread32First(hProcessSnap,&threadEntry32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	do{
		if(dwPID == threadEntry32.th32OwnerProcessID)
			hThread = OpenThread(THREAD_ALL_ACCESS,false,threadEntry32.th32ThreadID);

		if(hThread != INVALID_HANDLE_VALUE)
		{
			dwThreadCounter++;
			GetThreadContext(hThread,&cTT);

			switch(HardwareBPs[iBP].dwSlot)
			{
			case 0:
				cTT.Dr0 = HardwareBPs[iBP].dwOffset;
				break;
			case 1:
				cTT.Dr1 = HardwareBPs[iBP].dwOffset;
				break;
			case 2:
				cTT.Dr2 = HardwareBPs[iBP].dwOffset;
				break;
			case 3:
				cTT.Dr3 = HardwareBPs[iBP].dwOffset;
				break;
			}

			cTT.Dr7 |= 1 << (HardwareBPs[iBP].dwSlot * 2);
			cTT.Dr7 |= dwTypeFlag << ((HardwareBPs[iBP].dwSlot * 4) + 16);
			cTT.Dr7 |= (dwSize - 1) << ((HardwareBPs[iBP].dwSlot * 4) + 18);

			SetThreadContext(hThread,&cTT);
		}

		CloseHandle(hThread);
	}while(Thread32Next(hProcessSnap,&threadEntry32));

	memset(tcLogString,0x00,LOGBUFFER);
	swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] New HardwareBP in %i Threads placed at %X in Slot No. %i",dwThreadCounter,HardwareBPs[iBP].dwOffset,HardwareBPs[iBP].dwSlot);
	PBLogInfo();

	CloseHandle(hProcessSnap);
	return true;
}

bool clsDebugger::dHardwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwSlot)
{
	HANDLE hProcessSnap;
	THREADENTRY32 threadEntry32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,dwPID);

	if(hProcessSnap == INVALID_HANDLE_VALUE)
		return false;

	threadEntry32.dwSize = sizeof(THREADENTRY32);

	if(!Thread32First(hProcessSnap,&threadEntry32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	do{
		CONTEXT cTT;
		cTT.ContextFlags = CONTEXT_ALL;
		HANDLE hThread = INVALID_HANDLE_VALUE;

		if(dwPID == threadEntry32.th32OwnerProcessID)
			hThread = OpenThread(THREAD_GETSET_CONTEXT,false,threadEntry32.th32ThreadID);

		if(hThread != INVALID_HANDLE_VALUE)
		{
			GetThreadContext(hThread,&cTT);

			if(cTT.Dr0 == dwOffset) cTT.Dr0 = 0;
			if(cTT.Dr1 == dwOffset) cTT.Dr1 = 0;
			if(cTT.Dr2 == dwOffset) cTT.Dr2 = 0;
			if(cTT.Dr3 == dwOffset) cTT.Dr3 = 0;

			cTT.Dr7 &= ~(1 << (dwSlot * 2));
			cTT.Dr7 &= ~(3 << ((dwSlot * 4) + 16));
			cTT.Dr7 &= ~(3 << ((dwSlot * 4) + 18));

			SetThreadContext(hThread,&cTT);
		}

		CloseHandle(hThread);
	}while(Thread32Next(hProcessSnap,&threadEntry32));

	CloseHandle(hProcessSnap);
	return true;
}

bool clsDebugger::dSoftwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize,BYTE btOrgByte)
{
	if(dwOffset == 0 && btOrgByte == 0x00)
		return false;

	SIZE_T dwBytesWritten;
	HANDLE hPID = _pi.hProcess;

	if(dwPID != -1)
	{
		for(size_t i = 0;i < PIDs.size();i++)
		{
			if(dwPID == PIDs[i].dwPID)
				hPID = PIDs[i].hProc;
		}
	}

	if(WriteProcessMemory(hPID,(LPVOID)dwOffset,(LPVOID)&btOrgByte,dwSize,&dwBytesWritten))
		return true;
	return false;
}

bool clsDebugger::dMemoryBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize)
{
	MEMORY_BASIC_INFORMATION MBI;
	DWORD dwOldProtection;
	HANDLE hPID = _pi.hProcess;

	for(size_t i = 0;i < PIDs.size(); i++)
		if(PIDs[i].dwPID == dwPID)
			hPID = PIDs[i].hProc;

	VirtualQueryEx(hPID,(LPVOID)dwOffset,&MBI,sizeof(MBI));

	if(!VirtualProtectEx(hPID,(LPVOID)dwOffset,dwSize,MBI.Protect & PAGE_GUARD,&dwOldProtection))
		return false;
	return true;
}

bool clsDebugger::InitBP()
{
	for(size_t i = 0;i < SoftwareBPs.size(); i++)
		wSoftwareBP(SoftwareBPs[i].dwPID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwHandle,SoftwareBPs[i].dwSize,SoftwareBPs[i].bOrgByte);

	for(size_t i = 0;i < MemoryBPs.size(); i++)
		wMemoryBP(MemoryBPs[i].dwPID,MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].dwHandle);

	for(size_t i = 0;i < HardwareBPs.size(); i++)
		wHardwareBP(HardwareBPs[i].dwPID,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSize,HardwareBPs[i].dwSlot,HardwareBPs[i].dwTypeFlag);
	return true;
}

bool clsDebugger::RemoveBPs()
{
	for(size_t i = 0; i < SoftwareBPs.size(); i++)
		RemoveBPFromList(SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwTypeFlag);

	for(size_t i = 0; i < MemoryBPs.size(); i++)
		RemoveBPFromList(MemoryBPs[i].dwOffset,MemoryBPs[i].dwTypeFlag);

	for(size_t i = 0; i < HardwareBPs.size(); i++)	
		RemoveBPFromList(HardwareBPs[i].dwOffset,HardwareBPs[i].dwTypeFlag);

	SoftwareBPs.clear();
	HardwareBPs.clear();
	MemoryBPs.clear();
	return true;
}

bool clsDebugger::RemoveBPFromList(quint64 dwOffset,DWORD dwType) //,DWORD dwPID)
{ 
	switch(dwType)
	{
	case 0:
		for (vector<BPStruct>::iterator it = SoftwareBPs.begin();it != SoftwareBPs.end(); ++it)
		{
			if(it->dwOffset == dwOffset /* && it->dwPID == dwPID */)
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
			if(it->dwOffset == dwOffset /* && it->dwPID == dwPID */)
			{
				dMemoryBP(it->dwPID,it->dwOffset,it->dwSize);
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
			if(it->dwOffset == dwOffset /* && it->dwPID == dwPID */)
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
	
	emit OnBreakpointDeleted(dwOffset);

	return true;
}

bool clsDebugger::AddBreakpointToList(DWORD dwBPType,DWORD dwTypeFlag,DWORD dwPID,quint64 dwOffset,DWORD dwSlot,DWORD dwKeep)
{
	bool	bExists = false,
		bRetValue = false;

	for(size_t i = 0;i < SoftwareBPs.size();i++)
	{
		if(SoftwareBPs[i].dwOffset == dwOffset/* && SoftwareBPs[i].dwPID == dwPID */)
		{
			bExists = true;
			break;
		}
	}
	for(size_t i = 0;i < MemoryBPs.size();i++)
	{
		if(MemoryBPs[i].dwOffset == dwOffset /* && MemoryBPs[i].dwPID == dwPID*/)
		{
			bExists = true;
			break;
		}
	}
	for(size_t i = 0;i < HardwareBPs.size();i++)
	{
		if(HardwareBPs[i].dwOffset == dwOffset /* && HardwareBPs[i].dwPID == dwPID */)
		{
			bExists = true;
			break;
		}
	}

	if(!bExists)
	{
		DWORD dwSize = 1;

		switch(dwBPType)
		{
		case 0:
			{
				//SoftwareBP
				BPStruct newBP;

				newBP.dwOffset = dwOffset;
				newBP.dwHandle = dwKeep;
				newBP.dwSize = dwSize;
				wSoftwareBP(dwPID,dwOffset,dwKeep,dwSize,newBP.bOrgByte);
				newBP.dwPID = dwPID;
				newBP.bRestoreBP = false;
				newBP.dwTypeFlag = dwTypeFlag;
				newBP.moduleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
				ZeroMemory(newBP.moduleName,MAX_PATH * sizeof(TCHAR));

				if(dwKeep == 0x1)
					newBP.dwBaseOffset = clsHelperClass::CalcOffsetForModule(newBP.moduleName,newBP.dwOffset,newBP.dwPID);

				SoftwareBPs.push_back(newBP);

				emit OnNewBreakpointAdded(newBP,0);
				bRetValue = true;

				break;
			}
		case 1:
			{
				//MemoryBP
				BPStruct newBP;

				newBP.dwOffset = dwOffset;
				newBP.dwHandle = dwKeep;
				newBP.dwSize = dwSize;
				newBP.dwPID = dwPID;
				newBP.bOrgByte = NULL;
				newBP.bRestoreBP = false;
				newBP.dwTypeFlag = dwTypeFlag;
				newBP.moduleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
				ZeroMemory(newBP.moduleName,MAX_PATH * sizeof(TCHAR));
				newBP.dwBaseOffset = clsHelperClass::CalcOffsetForModule(newBP.moduleName,newBP.dwOffset,newBP.dwPID);

				MemoryBPs.push_back(newBP);
				wMemoryBP(dwPID,dwOffset,dwSize,dwKeep);
				emit OnNewBreakpointAdded(newBP,1);
				bRetValue = true;

				break;
			}
		case 2:
			{
				//HardwareBP
				if(HardwareBPs.size() == 4)
					break;

				BPStruct newBP;
				newBP.dwOffset = dwOffset;
				newBP.dwHandle = dwKeep;
				newBP.dwSize = dwSize;
				newBP.dwPID = dwPID;
				newBP.bOrgByte = NULL;
				newBP.bRestoreBP = false;
				newBP.dwTypeFlag = dwTypeFlag;
				newBP.moduleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
				ZeroMemory(newBP.moduleName,MAX_PATH * sizeof(TCHAR));
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
				if(!bSlot3) newBP.dwSlot = 2;
				if(!bSlot2) newBP.dwSlot = 1;
				if(!bSlot1) newBP.dwSlot = 0;

				HardwareBPs.push_back(newBP);
				wHardwareBP(dwPID,dwOffset,dwSize,newBP.dwSlot,dwTypeFlag);
				emit OnNewBreakpointAdded(newBP,2);
				bRetValue = true;

				break;
			}
		}
	}
	//if(_isDebugging)
	//	InitBP();

	return bRetValue;
}

void clsDebugger::UpdateOffsetsBPs()
{
	for(size_t i = 0;i < SoftwareBPs.size(); i++)
	{
		if(SoftwareBPs[i].dwHandle != 0x1) continue;

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
			
			SoftwareBPs[i].dwHandle = 0x3;
			emit OnNewBreakpointAdded(SoftwareBPs[i],0);
			SoftwareBPs[i].dwHandle = 0x1;
		}
	}

	for(size_t i = 0;i < MemoryBPs.size(); i++)
	{
		if(MemoryBPs[i].dwHandle != 0x1) continue;

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

			MemoryBPs[i].dwHandle = 0x3;
			emit OnNewBreakpointAdded(MemoryBPs[i],1);
			MemoryBPs[i].dwHandle = 0x1;
		}
	}

	for(size_t i = 0;i < HardwareBPs.size(); i++)
	{
		if(HardwareBPs[i].dwHandle != 0x1) continue;

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

			HardwareBPs[i].dwHandle = 0x3;
			emit OnNewBreakpointAdded(HardwareBPs[i],1);
			HardwareBPs[i].dwHandle = 0x1;
		}
	}
}