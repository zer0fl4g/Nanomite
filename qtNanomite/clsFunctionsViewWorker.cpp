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

#include <TlHelp32.h>
#include <Psapi.h>
#include <string>

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
		InsertSymbolsIntoLists(i->hProc,i->PID);
	}
	return;
}

QList<FunctionData> clsFunctionsViewWorker::GetPossibleFunctionBeginning(int PID, quint64 StartOffset,quint64 Size,DWORD SearchPattern,LPVOID lpBuffer,int SpaceLen)
{
	QList<FunctionData> functions;

	for(quint64 i = StartOffset; i < (StartOffset + Size); i++)
	{
		int counter = 0;
		if(memcmp(lpBuffer,&SearchPattern,0x1) == 0)
		{
			lpBuffer = (LPVOID)((quint64)lpBuffer + 1);
			counter++;

			while(memcmp(lpBuffer,&SearchPattern,0x1) == 0)
			{
				lpBuffer = (LPVOID)((quint64)lpBuffer + 1);
				counter++;i++;
			}
		}
		else
			lpBuffer = (LPVOID)((quint64)lpBuffer + 1);

		if(counter > SpaceLen)
		{
			// possible beginning
			FunctionData newFunction;
			newFunction.PID = PID;
			newFunction.FunctionOffset = i + 1;
			newFunction.FunctionSize = GetPossibleFunctionEnding(i + 1,(StartOffset + Size) - i + 1,SearchPattern,(LPVOID)((quint64)lpBuffer + 1),SpaceLen);

			if(newFunction.FunctionSize > 4)
				functions.append(newFunction);
		}
	}
	return functions;
}

quint64 clsFunctionsViewWorker::GetPossibleFunctionEnding(quint64 BaseAddress,quint64 Size,DWORD SearchPattern,LPVOID lpBuffer,int SpaceLen)
{
	for(quint64 i = BaseAddress; i < (BaseAddress + Size); i++)
	{
		int counter = 0;
		if(memcmp(lpBuffer,&SearchPattern,0x1) == 0)
		{
			lpBuffer = (LPVOID)((quint64)lpBuffer + 1);
			counter++;

			while(memcmp(lpBuffer,&SearchPattern,0x1) == 0)
			{
				lpBuffer = (LPVOID)((quint64)lpBuffer + 1);
				counter++;i++;		
			}
		}
		else
			lpBuffer = (LPVOID)((quint64)lpBuffer + 1);

		if(counter > SpaceLen)
			return i - BaseAddress - counter;
	}
	return 0;
}

void clsFunctionsViewWorker::GetValidMemoryParts(PTCHAR lpCurrentName,HANDLE hProc)
{
	DWORD64 CurAddress = NULL;
	PTCHAR	lpFileName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH *sizeof(TCHAR)),
			lpCurrentFileName = NULL;

	lpCurrentName = clsHelperClass::reverseStrip(lpCurrentName,'\\');

	MODULEENTRY32 pModEntry;
	pModEntry.dwSize = sizeof(MODULEENTRY32);
	MEMORY_BASIC_INFORMATION mbi;

	while(VirtualQueryEx(hProc,(LPVOID)CurAddress,&mbi,sizeof(mbi)))
	{
		if(GetMappedFileName(hProc,(LPVOID)CurAddress,lpFileName,MAX_PATH) > 0)
		{
			lpCurrentFileName = clsHelperClass::reverseStrip(lpFileName,'\\');
			if(lpCurrentFileName != NULL && wcslen(lpCurrentFileName) > 0)
			{
				if(wcscmp(lpCurrentFileName,lpCurrentName) == 0)
				{
					ParseMemoryRangeForFunctions(hProc,(quint64)mbi.BaseAddress,mbi.RegionSize);
				}				
			}
			free(lpCurrentFileName);
		}
		CurAddress += mbi.RegionSize;
	}
	clsMemManager::CFree(lpFileName);
}

void clsFunctionsViewWorker::ParseMemoryRangeForFunctions(HANDLE hProc,quint64 BaseAddress,quint64 Size)
{
	LPVOID lpBuffer = malloc(Size);
	DWORD	//searchPattern	= NULL,
			searchPattern2	= 0x90909090,
			searchPattern3	= 0xCCCCCCCC;
	//		dwOldProtection = NULL,
	//		dwNewProtection = PAGE_EXECUTE_READWRITE;

	if(lpBuffer == NULL) return;
//	if(!VirtualProtectEx(hProc,(LPVOID)BaseAddress,Size,dwNewProtection,&dwOldProtection))
//		return;

	if(!ReadProcessMemory(hProc,(LPVOID)BaseAddress,lpBuffer,Size,NULL))
	{
		free(lpBuffer);
		return;
	}
	
	//VirtualProtectEx(hProc,(LPVOID)BaseAddress,Size,dwOldProtection,&dwNewProtection);

	//functionList.append(GetPossibleFunctionBeginning(BaseAddress,Size,searchPattern,lpBuffer,4));
	functionList.append(GetPossibleFunctionBeginning(GetProcessId(hProc),BaseAddress,Size,searchPattern2,lpBuffer,4));
	functionList.append(GetPossibleFunctionBeginning(GetProcessId(hProc),BaseAddress,Size,searchPattern3,lpBuffer,4));

	free(lpBuffer);
}

void clsFunctionsViewWorker::InsertSymbolsIntoLists(HANDLE hProc,WORD PID)
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
				{
					wstring *sFuncTemp = &sFuncName;
					functionList[i].functionSymbol = QString().fromStdWString(*sFuncTemp);
					functionList[i].PID = PID;
				}
				else
					functionList[i].functionSymbol = QString("sub_%1").arg(functionList[i].FunctionOffset,16,16,QChar('0'));
			}		
		}
	}
}
