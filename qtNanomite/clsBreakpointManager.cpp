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
#include "../clsDebugger/clsDebugger.h"

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
	{
		clsMemManager::CFree(SoftwareBPs[i].moduleName);
		clsMemManager::CFree(SoftwareBPs[i].bOrgByte);
	}

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
				clsBreakpointSoftware::wSoftwareBP(newProcessID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwSize, &SoftwareBPs[i].bOrgByte);
			else if(SoftwareBPs[i].dwPID == newProcessID)
				clsBreakpointSoftware::wSoftwareBP(SoftwareBPs[i].dwPID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwSize, &SoftwareBPs[i].bOrgByte);
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
		{
			BPStruct *pCurrentBP;

			for (int i = 0; i < SoftwareBPs.size(); i++)
			{
				pCurrentBP = &SoftwareBPs[i];

				if(pCurrentBP->dwOffset == breakpointOffset)
				{
					clsBreakpointSoftware::dSoftwareBP(pCurrentBP->dwPID, pCurrentBP->dwOffset, pCurrentBP->dwSize, pCurrentBP->bOrgByte);
					clsMemManager::CFree(pCurrentBP->moduleName);
					clsMemManager::CFree(pCurrentBP->bOrgByte);

					SoftwareBPs.removeAt(i);

					break; // add allows only one so we can stop here
				}
			}
			break;
		}
	case MEMORY_BP:
		{
			BPStruct *pCurrentBP;

			for (int i = 0; i < MemoryBPs.size(); i++)
			{
				pCurrentBP = &MemoryBPs[i];

				if(pCurrentBP->dwOffset == breakpointOffset)
				{
					clsBreakpointMemory::dMemoryBP(pCurrentBP->dwPID, pCurrentBP->dwOffset, pCurrentBP->dwSize, pCurrentBP->dwOldProtection);
					clsMemManager::CFree(pCurrentBP->moduleName);
					clsMemManager::CFree(pCurrentBP->bOrgByte);

					MemoryBPs.removeAt(i);

					break; // add allows only one so we can stop here
				}
			}
			break;
		}
	case HARDWARE_BP:
		{
			BPStruct *pCurrentBP;

			for (int i = 0; i < HardwareBPs.size(); i++)
			{
				pCurrentBP = &HardwareBPs[i];

				if(pCurrentBP->dwOffset == breakpointOffset)
				{
					clsBreakpointHardware::dHardwareBP(pCurrentBP->dwPID, pCurrentBP->dwOffset, pCurrentBP->dwSlot);
					clsMemManager::CFree(pCurrentBP->moduleName);
					clsMemManager::CFree(pCurrentBP->bOrgByte);

					HardwareBPs.removeAt(i);

					break; // add allows only one so we can stop here
				}
			}
			break;
		}
	}
	
	emit OnBreakpointDeleted(breakpointOffset);

	return true;
}

bool clsBreakpointManager::BreakpointAdd(DWORD breakpointType, DWORD typeFlag, DWORD processID, DWORD64 breakpointOffset, int breakpointSize, DWORD breakpointHandleType, DWORD breakpointDataType)
{
	if(breakpointOffset == NULL) return false;

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
		BPStruct newBP = { 0 };

		switch(breakpointType)
		{
		case SOFTWARE_BP:
			{
				/*if(breakpointDataType == BP_SW_LONGINT3)
				{
					breakpointSize = 2;

					if(!clsBreakpointSoftware::wSoftwareBP(processID, breakpointOffset, breakpointSize, &newBP.bOrgByte, BP_SW_LONGINT3))
						break;

					breakpointOffset += 1;
				}
				else*/ if(breakpointDataType == BP_SW_UD2)
				{
					breakpointSize = 2;

					if(!clsBreakpointSoftware::wSoftwareBP(processID, breakpointOffset, breakpointSize, &newBP.bOrgByte, BP_SW_UD2))
						break;
				}
				else if(breakpointDataType == BP_SW_HLT)
				{
					if(!clsBreakpointSoftware::wSoftwareBP(processID, breakpointOffset, breakpointSize, &newBP.bOrgByte, BP_SW_HLT))
						break;
				}
				else
				{
					if(!clsBreakpointSoftware::wSoftwareBP(processID, breakpointOffset, breakpointSize, &newBP.bOrgByte))
						break;
				}

				newBP.dwOffset = breakpointOffset;
				newBP.dwHandle = breakpointHandleType;
				newBP.dwSize = breakpointSize;
				newBP.dwDataType = breakpointDataType;
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

				if(!clsBreakpointMemory::wMemoryBP(processID, breakpointOffset, breakpointSize, typeFlag, &oldProtection))
					break;
				
				newBP.dwOffset = breakpointOffset;
				newBP.dwHandle = breakpointHandleType;
				newBP.dwSize = breakpointSize;
				newBP.dwPID = processID;
				newBP.dwTypeFlag = typeFlag;
				newBP.dwOldProtection = oldProtection;
				newBP.moduleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
				ZeroMemory(newBP.moduleName,MAX_PATH * sizeof(TCHAR));
				
				if(breakpointHandleType == BP_KEEP)
					newBP.dwBaseOffset = clsHelperClass::CalcOffsetForModule(newBP.moduleName, newBP.dwOffset, newBP.dwPID);
				
				MemoryBPs.append(newBP);
				emit OnBreakpointAdded(newBP,MEMORY_BP);
				bRetValue = true;

				break;
			}
		case HARDWARE_BP:
			{
				if(HardwareBPs.size() == 4)
					break;
				
				newBP.dwOffset = breakpointOffset;
				newBP.dwHandle = breakpointHandleType;
				newBP.dwSize = breakpointSize;
				newBP.dwPID = processID;
				newBP.dwTypeFlag = typeFlag;
				newBP.moduleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
				ZeroMemory(newBP.moduleName,MAX_PATH * sizeof(TCHAR));
			
				if(breakpointHandleType == BP_KEEP)
					newBP.dwBaseOffset = clsHelperClass::CalcOffsetForModule(newBP.moduleName, newBP.dwOffset, newBP.dwPID);

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

				if(!clsBreakpointHardware::wHardwareBP(processID, breakpointOffset, breakpointSize, newBP.dwSlot, typeFlag))
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

void clsBreakpointManager::BreakpointUpdateOffsets(HANDLE processHandle, DWORD processID)
{
	for(int i = 0;i < SoftwareBPs.size(); i++)
	{
		if(SoftwareBPs[i].dwHandle != BP_KEEP) continue;

		BreakpointRebase(&SoftwareBPs[i], SOFTWARE_BP, processHandle, processID);
	}

	for(int i = 0;i < MemoryBPs.size(); i++)
	{
		if(MemoryBPs[i].dwHandle != BP_KEEP) continue;

		BreakpointRebase(&MemoryBPs[i], MEMORY_BP, processHandle, processID);
	}

	for(int i = 0;i < HardwareBPs.size(); i++)
	{
		if(HardwareBPs[i].dwHandle != BP_KEEP) continue;

		BreakpointRebase(&HardwareBPs[i], HARDWARE_BP, processHandle, processID);
	}
}

void clsBreakpointManager::BreakpointRebase(BPStruct *pCurrentBP, int bpType, HANDLE processHandle, DWORD processID)
{
	DWORD64 newBaseOffset = clsHelperClass::CalcOffsetForModule(pCurrentBP->moduleName, pCurrentBP->dwOffset, pCurrentBP->dwPID);
	if(newBaseOffset != pCurrentBP->dwBaseOffset)
	{
		pCurrentBP->dwOldOffset = pCurrentBP->dwOffset;
		pCurrentBP->dwOffset = (pCurrentBP->dwOffset - pCurrentBP->dwBaseOffset) + newBaseOffset;
		pCurrentBP->dwBaseOffset = newBaseOffset;

		pCurrentBP->dwHandle = BP_OFFSETUPDATE;
		emit OnBreakpointAdded(*pCurrentBP, bpType);
		pCurrentBP->dwHandle = BP_KEEP;
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
				clsBreakpointSoftware::dSoftwareBP(it->dwPID, it->dwOffset, it->dwSize, it->bOrgByte);
		}
	}
	else
	{
		for (QList<BPStruct>::iterator it = pThis->SoftwareBPs.begin();it != pThis->SoftwareBPs.end(); ++it)
		{
			if((it->dwPID == processID || it->dwPID == -1) && it->bRestoreBP == false)
				clsBreakpointSoftware::wSoftwareBP(it->dwPID, it->dwOffset, it->dwSize, &it->bOrgByte, it->dwDataType);
		}
	}
}

bool clsBreakpointManager::BreakpointInsert(DWORD breakpointType, DWORD typeFlag, DWORD processID, DWORD64 breakpointOffset, int breakpointSize, DWORD breakpointHandleType, DWORD breakpointDataType)
{
	return pThis->BreakpointAdd(breakpointType, typeFlag, processID, breakpointOffset, breakpointSize, breakpointHandleType, breakpointDataType);
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
			clsMemManager::CFree(it->bOrgByte);

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
	BPStruct *pTempBP;

	if(takeAll)
		tempSearchPID = -1;

	switch(breakpointType)
	{
	case SOFTWARE_BP:
		{
			for(int i = 0; i < SoftwareBPs.size(); i++)
			{
				pTempBP = &SoftwareBPs[i];

				if(pTempBP->dwOffset == breakpointOffset && 
					(pTempBP->dwPID == processID || pTempBP->dwPID == tempSearchPID))
				{
					*pBreakpointSearched = pTempBP;

					return true;
				}
			}

			break;
		}
	case MEMORY_BP:
		{
			for(int i = 0; i < MemoryBPs.size(); i++)
			{
				pTempBP = &MemoryBPs[i];

				if(pTempBP->dwOffset == breakpointOffset && 
					(pTempBP->dwPID == processID || pTempBP->dwPID == tempSearchPID))
				{
					*pBreakpointSearched = pTempBP;

					return true;
				}
			}

			break;
		}
	case HARDWARE_BP:
		{
			for(int i = 0; i < HardwareBPs.size(); i++)
			{
				pTempBP = &HardwareBPs[i];

				if(pTempBP->dwOffset == breakpointOffset && 
					(pTempBP->dwPID == processID || pTempBP->dwPID == tempSearchPID))
				{
					*pBreakpointSearched = pTempBP;

					return true;
				}
			}

			break;
		}
	}

	return false;
}