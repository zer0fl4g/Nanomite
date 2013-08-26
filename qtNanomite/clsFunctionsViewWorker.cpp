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
#include "clsFunctionsViewWorker.h"
#include "clsMemManager.h"
#include "clsHelperClass.h"

#include <Psapi.h>

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include <BeaEngine.h>

using namespace std;

clsFunctionsViewWorker::clsFunctionsViewWorker(QList<FunctionProcessingData> dataForProcessing)
{
	m_processingData = dataForProcessing;
	this->start();
}

clsFunctionsViewWorker::~clsFunctionsViewWorker()
{
	functionList.clear();
}

void clsFunctionsViewWorker::run()
{
	for(QList<FunctionProcessingData>::ConstIterator i = m_processingData.constBegin(); i != m_processingData.constEnd(); ++i)
	{
		GetValidMemoryParts((PTCHAR)i->currentModule,i->hProc);
		InsertSymbolsIntoLists(i->hProc);
	}

	qSort(functionList.begin(),functionList.end(),OffsetLessThan);
}

void clsFunctionsViewWorker::GetValidMemoryParts(PTCHAR lpCurrentName,HANDLE hProc)
{
	DWORD64 CurAddress = NULL;
	PTCHAR	lpFileName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH *sizeof(TCHAR)),
			lpCurrentNameTemp = clsHelperClass::reverseStrip(lpCurrentName,'\\'),
			lpCurrentFileName = NULL;

	MEMORY_BASIC_INFORMATION mbi;
	while(VirtualQueryEx(hProc,(LPVOID)CurAddress,&mbi,sizeof(mbi)))
	{
		if(GetMappedFileName(hProc,(LPVOID)CurAddress,lpFileName,MAX_PATH) > 0)
		{
			lpCurrentFileName = clsHelperClass::reverseStrip(lpFileName,'\\');
			if(lpCurrentFileName != NULL && wcslen(lpCurrentFileName) > 0)
			{
				if(wcscmp(lpCurrentFileName,lpCurrentNameTemp) == 0 &&
					((mbi.Protect & PAGE_EXECUTE) || (mbi.Protect & PAGE_EXECUTE_READ) || (mbi.Protect & PAGE_EXECUTE_READWRITE) || (mbi.Protect & PAGE_EXECUTE_WRITECOPY)))
				{
					ParseMemoryRangeForCallFunctions(hProc,(quint64)mbi.BaseAddress,mbi.RegionSize);
					ParseMemoryRangeForJumpFunctions(hProc,(quint64)mbi.BaseAddress,mbi.RegionSize);
				}				
				
				clsMemManager::CFree(lpCurrentFileName);
			}
		}
		CurAddress += mbi.RegionSize;
	}

	clsMemManager::CFree(lpCurrentNameTemp);
	clsMemManager::CFree(lpFileName);
}

void clsFunctionsViewWorker::ParseMemoryRangeForCallFunctions(HANDLE hProc,quint64 BaseAddress,quint64 Size)
{
	LPVOID lpBuffer = malloc(Size);
	if(lpBuffer == NULL) return;

	if(!ReadProcessMemory(hProc,(LPVOID)BaseAddress,lpBuffer,Size,NULL))
	{
		free(lpBuffer);
		return;
	}

	DISASM newDisAss;
	FunctionData newFunction;
	quint64 endOffset = BaseAddress + Size;
	int processID = GetProcessId(hProc);
	bool bContinueDisAs = true,
		isContained = false;
	int iLen = 0;

	memset(&newDisAss, 0, sizeof(DISASM));

	newDisAss.EIP = (quint64)lpBuffer;
	newDisAss.VirtualAddr = BaseAddress;
#ifdef _AMD64_
	newDisAss.Archi = 64;
#else
	newDisAss.Archi = 0;
#endif

	while(bContinueDisAs)
	{
		newDisAss.SecurityBlock = (int)endOffset - newDisAss.VirtualAddr;

		iLen = Disasm(&newDisAss);
		if (iLen == OUT_OF_BLOCK)
			bContinueDisAs = false;
		else if(iLen == UNKNOWN_OPCODE)
			iLen = 1;
		else
		{	
			if(newDisAss.Instruction.Opcode == 0x00 && iLen == 2)
				iLen = 1;

			if(iLen > 0 &&
				newDisAss.Instruction.BranchType == CallType &&
				newDisAss.Instruction.AddrValue >= BaseAddress &&
				newDisAss.Instruction.AddrValue <= endOffset) 
			{
				for(QList<FunctionData>::const_iterator functionIT = functionList.constBegin(); functionIT != functionList.constEnd(); ++functionIT)
				{
					if(functionIT->FunctionOffset == newDisAss.Instruction.AddrValue)
					{
						isContained = true;
						break;
					}
				}

				if(!isContained)
				{
					newFunction.FunctionOffset = newDisAss.Instruction.AddrValue;
					newFunction.FunctionSize = GetFunctionSizeFromCallPoint(hProc, newDisAss.Instruction.AddrValue, endOffset);
					newFunction.functionSymbol = "";
					newFunction.PID = processID;
					functionList.append(newFunction);
				}

				isContained = false;
			}
		}

		newDisAss.EIP += iLen;
		newDisAss.VirtualAddr += iLen;
			

		if (newDisAss.VirtualAddr >= endOffset)
			bContinueDisAs = false;
	}

	free(lpBuffer);
}

void clsFunctionsViewWorker::ParseMemoryRangeForJumpFunctions(HANDLE hProc,quint64 BaseAddress,quint64 Size)
{
	LPVOID lpBuffer = malloc(Size);
	if(lpBuffer == NULL) return;

	if(!ReadProcessMemory(hProc,(LPVOID)BaseAddress,lpBuffer,Size,NULL))
	{
		free(lpBuffer);
		return;
	}

	DISASM newDisAss;
	FunctionData newFunction;
	quint64 endOffset = BaseAddress + Size;
	int processID = GetProcessId(hProc);
	bool bContinueDisAs = true,
		isContained = false;
	int iLen = 0;

	memset(&newDisAss, 0, sizeof(DISASM));

	newDisAss.EIP = (quint64)lpBuffer;
	newDisAss.VirtualAddr = BaseAddress;
#ifdef _AMD64_
	newDisAss.Archi = 64;
#else
	newDisAss.Archi = 0;
#endif

	while(bContinueDisAs)
	{
		newDisAss.SecurityBlock = (int)endOffset - newDisAss.VirtualAddr;

		iLen = Disasm(&newDisAss);
		if (iLen == OUT_OF_BLOCK)
			bContinueDisAs = false;
		else if(iLen == UNKNOWN_OPCODE)
			iLen = 1;
		else
		{	
			if(newDisAss.Instruction.Opcode == 0x00 && iLen == 2)
				iLen = 1;

			if(iLen > 0 &&
				newDisAss.Instruction.BranchType == JmpType &&
				newDisAss.Instruction.AddrValue >= BaseAddress &&
				newDisAss.Instruction.AddrValue <= endOffset) 
			{
				for(QList<FunctionData>::const_iterator functionIT = functionList.constBegin(); functionIT != functionList.constEnd(); ++functionIT)
				{
					if(newDisAss.Instruction.AddrValue >= functionIT->FunctionOffset && newDisAss.Instruction.AddrValue <= (functionIT->FunctionOffset + functionIT->FunctionSize))
					{
						isContained = true;
						break;
					}
				}

				if(!isContained)
				{
					newFunction.FunctionOffset = newDisAss.Instruction.AddrValue;
					newFunction.FunctionSize = GetFunctionSizeFromCallPoint(hProc, newDisAss.Instruction.AddrValue, endOffset);
					newFunction.functionSymbol = "";
					newFunction.PID = processID;
					functionList.append(newFunction);
				}

				isContained = false;
			}
		}

		newDisAss.EIP += iLen;
		newDisAss.VirtualAddr += iLen;
			

		if (newDisAss.VirtualAddr >= endOffset)
			bContinueDisAs = false;
	}

	free(lpBuffer);
}

DWORD clsFunctionsViewWorker::GetFunctionSizeFromCallPoint(HANDLE processHandle, quint64 functionOffset, quint64 pageEnd)
{
	LPVOID lpBuffer = malloc(pageEnd - functionOffset);
	if(lpBuffer == NULL) return 0;

	if(!ReadProcessMemory(processHandle, (LPVOID)functionOffset, lpBuffer, pageEnd - functionOffset, NULL))
	{
		free(lpBuffer);
		return 0;
	}

	QList<JumpData> jumpsInFunction;
	DISASM newDisAss;
	JumpData newJump;
	bool bContinueDisAs = true,
		isNotFunctionEnd = false;
	int iLen = NULL,
		trashCounter = NULL;
	quint64 instructionCounter = NULL;

	memset(&newDisAss, 0, sizeof(DISASM));

	newDisAss.EIP = (quint64)lpBuffer;
	newDisAss.VirtualAddr = functionOffset;
#ifdef _AMD64_
	newDisAss.Archi = 64;
#else
	newDisAss.Archi = 0;
#endif
	

	while(bContinueDisAs)
	{
		newDisAss.SecurityBlock = (int)pageEnd - newDisAss.VirtualAddr;

		iLen = Disasm(&newDisAss);
		if (iLen == OUT_OF_BLOCK)
			bContinueDisAs = false;
		else if(iLen == UNKNOWN_OPCODE)
			iLen = 1;
		else
		{	
			if(newDisAss.Instruction.Opcode == 0x00 && iLen == 2)
				iLen = 1;

			if(iLen > 0)
			{
				instructionCounter++;

				if(newDisAss.Instruction.Opcode == 0x00 ||
					newDisAss.Instruction.Opcode == 0x90 ||
					newDisAss.Instruction.Opcode == 0xCC)
				{
					trashCounter++;

					if(trashCounter >= 4)
					{
						free(lpBuffer);
						return (newDisAss.VirtualAddr - functionOffset - trashCounter);
					}
				} 
				else if(newDisAss.Instruction.BranchType == RetType)
				{
					for(QList<JumpData>::iterator jumpIT = jumpsInFunction.begin(); jumpIT != jumpsInFunction.end(); ++jumpIT)
					{
						if(jumpIT->jumpTarget < functionOffset)
						{
							// a jump above our function entry
							// A
							// |
							// |
							free(lpBuffer);
							return jumpIT->jumpOffset - functionOffset + iLen;
						}
						else if(jumpIT->jumpTarget > newDisAss.VirtualAddr)
						{
							// a jump over the found ret
							// |
							// |
							// V
							isNotFunctionEnd = true;
						}
						else if(jumpIT->jumpTarget == newDisAss.VirtualAddr)
						{
							jumpIT = jumpsInFunction.erase(jumpIT);
						}

						if(jumpIT == jumpsInFunction.end())
							break;
					}

					if(!isNotFunctionEnd)
					{
						free(lpBuffer);
						return newDisAss.VirtualAddr - functionOffset + iLen;
					}
					else
					{
						//jumpsInFunction.clear();
						trashCounter = NULL;
						isNotFunctionEnd = false;
					}
				}
				else if(newDisAss.Instruction.BranchType == JmpType || 
					(newDisAss.Instruction.BranchType <= -1 && newDisAss.Instruction.BranchType >= -8) ||
					(newDisAss.Instruction.BranchType >= 1 && newDisAss.Instruction.BranchType <= 8))
				{
					if(instructionCounter == 1)
					{
						free(lpBuffer);
						return newDisAss.VirtualAddr - functionOffset + iLen;
					}

					if(newDisAss.Instruction.AddrValue != NULL)
					{
						newJump.jumpOffset = newDisAss.VirtualAddr;
						newJump.jumpTarget = newDisAss.Instruction.AddrValue;

						trashCounter = NULL;
						jumpsInFunction.append(newJump);
					}
				}
			}
		}

		newDisAss.EIP += iLen;
		newDisAss.VirtualAddr += iLen;

		if (newDisAss.VirtualAddr >= pageEnd)
			bContinueDisAs = false;
	}

	free(lpBuffer);
	return 0;
}

void clsFunctionsViewWorker::InsertSymbolsIntoLists(HANDLE hProc)
{
	wstring sModName,sFuncName;

	if(functionList.size() > 0)
	{
		for(quint64 i = 0; i < functionList.count(); i++)
		{
			if(functionList[i].functionSymbol.isEmpty())
			{
				clsHelperClass::LoadSymbolForAddr(sFuncName,sModName,functionList[i].FunctionOffset,hProc);
				if(sFuncName.length() > 0)
					functionList[i].functionSymbol = QString().fromStdWString(sFuncName);
				else
					functionList[i].functionSymbol = QString("sub_%1").arg(functionList[i].FunctionOffset,16,16,QChar('0'));
			}		
		}
	}
}

bool OffsetLessThan(const FunctionData &f1, const FunctionData &f2)
{
	return f1.FunctionOffset < f2.FunctionOffset;
}