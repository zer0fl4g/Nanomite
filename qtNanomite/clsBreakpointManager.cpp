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
#include "clsBreakpointManager.h"
#include "clsHelperClass.h"

clsBreakpointManager* clsBreakpointManager::pThis = NULL;

clsBreakpointManager::clsBreakpointManager()
{
	pThis = this;
}

clsBreakpointManager::~clsBreakpointManager()
{
	for(int i = 0; i < SoftwareBPs.size(); i++)
		clsMemManager::CFree(SoftwareBPs[i].moduleName);

	for(int i = 0; i < MemoryBPs.size(); i++)
		clsMemManager::CFree(MemoryBPs[i].moduleName);

	for(int i = 0; i < HardwareBPs.size(); i++)	
		clsMemManager::CFree(HardwareBPs[i].moduleName);

	SoftwareBPs.clear();
	HardwareBPs.clear();
	MemoryBPs.clear();

	pThis = NULL;
}

bool clsBreakpointManager::BreakpointInit(DWORD newProcessID, bool isThread)
{
	if(!isThread)
	{
		for(int i = 0;i < SoftwareBPs.size(); i++)
		{
			if(SoftwareBPs[i].dwPID == -1)
				clsBreakpointSoftware::wSoftwareBP(newProcessID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwSize,SoftwareBPs[i].bOrgByte);
			else if(SoftwareBPs[i].dwPID == newProcessID)
				clsBreakpointSoftware::wSoftwareBP(SoftwareBPs[i].dwPID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwSize,SoftwareBPs[i].bOrgByte);
		}

		for(int i = 0;i < MemoryBPs.size(); i++)
		{
			if(MemoryBPs[i].dwPID == -1)
				clsBreakpointMemory::wMemoryBP(newProcessID,MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].dwTypeFlag,&MemoryBPs[i].dwOldProtection);
			else if(MemoryBPs[i].dwPID == newProcessID)
				clsBreakpointMemory::wMemoryBP(MemoryBPs[i].dwPID,MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].dwTypeFlag,&MemoryBPs[i].dwOldProtection);
		}
	}

	for(int i = 0;i < HardwareBPs.size(); i++)
	{
		if(HardwareBPs[i].dwPID == -1)
			clsBreakpointHardware::wHardwareBP(newProcessID,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSize,HardwareBPs[i].dwSlot,HardwareBPs[i].dwTypeFlag);
		else if(HardwareBPs[i].dwPID == newProcessID)
			clsBreakpointHardware::wHardwareBP(HardwareBPs[i].dwPID,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSize,HardwareBPs[i].dwSlot,HardwareBPs[i].dwTypeFlag);
	}

	return true;
}

bool clsBreakpointManager::BreakpointClear()
{
	for(int i = 0; i < SoftwareBPs.size(); i++)
		BreakpointRemove(SoftwareBPs[i].dwOffset, SOFTWARE_BP);

	for(int i = 0; i < MemoryBPs.size(); i++)
		BreakpointRemove(MemoryBPs[i].dwOffset, MEMORY_BP);

	for(int i = 0; i < HardwareBPs.size(); i++)	
		BreakpointRemove(HardwareBPs[i].dwOffset, HARDWARE_BP);

	SoftwareBPs.clear();
	HardwareBPs.clear();
	MemoryBPs.clear();

	return true;
}

bool clsBreakpointManager::BreakpointRemove(DWORD64 breakpointOffset, DWORD breakpointType) //,DWORD dwPID)
{ 
	switch(breakpointType)
	{
	case SOFTWARE_BP:
		for (QList<BPStruct>::iterator it = SoftwareBPs.begin(); it != SoftwareBPs.end(); ++it)
		{
			if(it->dwOffset == breakpointOffset /* && it->dwPID == dwPID */)
			{
				clsBreakpointSoftware::dSoftwareBP(it->dwPID,it->dwOffset,it->dwSize,it->bOrgByte);
				clsMemManager::CFree(it->moduleName);

				SoftwareBPs.erase(it);
				it = SoftwareBPs.begin();

				if(it == SoftwareBPs.end())
					break;
			}
		}
		break;

	case MEMORY_BP:
		for (QList<BPStruct>::iterator it = MemoryBPs.begin();it != MemoryBPs.end(); ++it)
		{
			if(it->dwOffset == breakpointOffset /* && it->dwPID == dwPID */)
			{
				clsBreakpointMemory::dMemoryBP(it->dwPID,it->dwOffset,it->dwSize,it->dwOldProtection);
				clsMemManager::CFree(it->moduleName);

				MemoryBPs.erase(it);
				it = MemoryBPs.begin();

				if(it == MemoryBPs.end())
					break;
			}
		}
		break;

	case HARDWARE_BP:
		for (QList<BPStruct>::iterator it = HardwareBPs.begin();it != HardwareBPs.end(); ++it)
		{
			if(it->dwOffset == breakpointOffset /* && it->dwPID == dwPID */)
			{
				clsBreakpointHardware::dHardwareBP(it->dwPID,it->dwOffset,it->dwSlot);
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

bool clsBreakpointManager::BreakpointAdd(DWORD breakpointType, DWORD typeFlag, DWORD processID, DWORD64 breakpointOffset, DWORD breakpointHandleType)
{
	bool	bExists		= false,
			bRetValue	= false;

	for(int i = 0;i < SoftwareBPs.size();i++)
	{
		if(SoftwareBPs[i].dwOffset == breakpointOffset/* && SoftwareBPs[i].dwPID == dwPID */)
		{
			bExists = true;
			break;
		}
	}
	for(int i = 0;i < MemoryBPs.size();i++)
	{
		if(MemoryBPs[i].dwOffset == breakpointOffset /* && MemoryBPs[i].dwPID == dwPID*/)
		{
			bExists = true;
			break;
		}
	}
	for(int i = 0;i < HardwareBPs.size();i++)
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

				if(!clsBreakpointSoftware::wSoftwareBP(processID, breakpointOffset, dwSize, newBP.bOrgByte))
					break;

				newBP.dwOffset = breakpointOffset;
				newBP.dwHandle = breakpointHandleType;
				newBP.dwSize = dwSize;
				newBP.dwPID = processID;
				newBP.moduleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
				ZeroMemory(newBP.moduleName,MAX_PATH * sizeof(TCHAR));

				if(breakpointHandleType == BP_KEEP)
					newBP.dwBaseOffset = clsHelperClass::CalcOffsetForModule(newBP.moduleName,newBP.dwOffset,newBP.dwPID);

				SoftwareBPs.append(newBP);

				emit OnBreakpointAdded(newBP,SOFTWARE_BP);
				bRetValue = true;

				break;
			}
		case MEMORY_BP:
			{
				DWORD oldProtection = NULL;

				if(!clsBreakpointMemory::wMemoryBP(processID, breakpointOffset, dwSize, typeFlag, &oldProtection))
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
				
				MemoryBPs.append(newBP);
				emit OnBreakpointAdded(newBP,MEMORY_BP);
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
				for(int i = 0;i < HardwareBPs.size();i++)
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

				if(!clsBreakpointHardware::wHardwareBP(processID, breakpointOffset, dwSize, newBP.dwSlot, typeFlag))
				{
					clsMemManager::CFree(newBP.moduleName);
					break;
				}

				HardwareBPs.append(newBP);
				emit OnBreakpointAdded(newBP, HARDWARE_BP);
				bRetValue = true;

				break;
			}
		}
	}

	return bRetValue;
}

void clsBreakpointManager::BreakpointUpdateOffsets()
{
	for(int i = 0;i < SoftwareBPs.size(); i++)
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
			emit OnBreakpointAdded(SoftwareBPs[i], SOFTWARE_BP);
			SoftwareBPs[i].dwHandle = BP_KEEP;
		}
	}

	for(int i = 0;i < MemoryBPs.size(); i++)
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
			emit OnBreakpointAdded(MemoryBPs[i], MEMORY_BP);
			MemoryBPs[i].dwHandle = BP_KEEP;
		}
	}

	for(int i = 0;i < HardwareBPs.size(); i++)
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
			emit OnBreakpointAdded(HardwareBPs[i], HARDWARE_BP);
			HardwareBPs[i].dwHandle = BP_KEEP;
		}
	}
}

bool clsBreakpointManager::IsOffsetAnBP(quint64 Offset)
{
	for(int i = 0; i < pThis->SoftwareBPs.size(); i++)
		if(pThis->SoftwareBPs[i].dwOffset == Offset && pThis->SoftwareBPs[i].dwHandle == BP_KEEP)
			return true;

	for(int i = 0; i < pThis->MemoryBPs.size(); i++)
		if(pThis->MemoryBPs[i].dwOffset == Offset)
			return true;

	for(int i = 0; i < pThis->HardwareBPs.size(); i++)
		if(pThis->HardwareBPs[i].dwOffset == Offset)
			return true;
	return false;
}

void clsBreakpointManager::RemoveSBPFromMemory(bool isDisable, DWORD processID)
{
	if(isDisable)
	{
		for (QList<BPStruct>::iterator it = pThis->SoftwareBPs.begin();it != pThis->SoftwareBPs.end(); ++it)
		{
			if((it->dwPID == processID || it->dwPID == -1) && it->bRestoreBP == false)
				clsBreakpointSoftware::dSoftwareBP(it->dwPID,it->dwOffset,it->dwSize,it->bOrgByte);
		}
	}
	else
	{
		for (QList<BPStruct>::iterator it = pThis->SoftwareBPs.begin();it != pThis->SoftwareBPs.end(); ++it)
		{
			if((it->dwPID == processID || it->dwPID == -1) && it->bRestoreBP == false)
				clsBreakpointSoftware::wSoftwareBP(it->dwPID,it->dwOffset,it->dwSize,it->bOrgByte);
		}
	}
}

bool clsBreakpointManager::BreakpointInsert(DWORD breakpointType, DWORD typeFlag, DWORD processID, DWORD64 breakpointOffset, DWORD breakpointHandleType)
{
	return pThis->BreakpointAdd(breakpointType, typeFlag, processID, breakpointOffset, breakpointHandleType);
}

bool clsBreakpointManager::BreakpointDelete(DWORD64 breakpointOffset, DWORD breakpointType)
{
	return pThis->BreakpointRemove(breakpointOffset, breakpointType);
}

void clsBreakpointManager::BreakpointCleanup()
{
	for(QList<BPStruct>::iterator it = SoftwareBPs.begin(); it != SoftwareBPs.end(); ++it)
	{
		if(it->dwHandle != BP_KEEP)
		{
			clsMemManager::CFree(it->moduleName);
			SoftwareBPs.erase(it);
			it = SoftwareBPs.begin();

			if(it == SoftwareBPs.end())
				break;
		}
	}
}

bool clsBreakpointManager::BreakpointFind(DWORD64 breakpointOffset, int breakpointType, DWORD processID, bool takeAll, BPStruct** pBreakpointSearched)
{
	DWORD tempSearchPID = processID;

	if(takeAll)
		tempSearchPID = -1;

	switch(breakpointType)
	{
	case SOFTWARE_BP:
		{
			for(int i = 0; i < SoftwareBPs.size(); i++)
			{
				if(SoftwareBPs[i].dwOffset == breakpointOffset && 
					(SoftwareBPs[i].dwPID == processID || SoftwareBPs[i].dwPID == tempSearchPID))
				{
					*pBreakpointSearched = &SoftwareBPs[i];

					return true;
				}
			}

			break;
		}
	case MEMORY_BP:
		{
			for(int i = 0; i < MemoryBPs.size(); i++)
			{
				if(MemoryBPs[i].dwOffset == breakpointOffset && 
					(MemoryBPs[i].dwPID == processID || MemoryBPs[i].dwPID == tempSearchPID))
				{
					*pBreakpointSearched = &MemoryBPs[i];

					return true;
				}
			}

			break;
		}
	case HARDWARE_BP:
		{
			for(int i = 0; i < HardwareBPs.size(); i++)
			{
				if(HardwareBPs[i].dwOffset == breakpointOffset && 
					(HardwareBPs[i].dwPID == processID || HardwareBPs[i].dwPID == tempSearchPID))
				{
					*pBreakpointSearched = &HardwareBPs[i];

					return true;
				}
			}

			break;
		}
	}

	return false;
}