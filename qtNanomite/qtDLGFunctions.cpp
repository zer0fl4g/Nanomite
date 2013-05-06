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
#include "qtDLGFunctions.h"
#include "qtDLGNanomite.h"

#include "clsMemManager.h"
#include "clsHelperClass.h"

#include <TlHelp32.h>
#include <Psapi.h>

using namespace std;

qtDLGFunctions::qtDLGFunctions(QWidget *parent, Qt::WFlags flags,qint32 iPID)
	: QWidget(parent, flags)
{
	this->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);

	_iPID = iPID;

	connect(tblFunctions,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenu(QPoint)));

	// Init List
	tblFunctions->horizontalHeader()->resizeSection(0,75);
	tblFunctions->horizontalHeader()->resizeSection(1,200);
	tblFunctions->horizontalHeader()->resizeSection(2,135);

	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	int iForEntry = 0;
	int iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(int i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == _iPID)
			iForEntry = i; iForEnd = i + 1;
	}

	for(int i = iForEntry; i < iForEnd;i++)
	{
		GetValidMemoryParts((PTCHAR)myMainWindow->coreDebugger->GetTarget().c_str(),myMainWindow->coreDebugger->PIDs[i].hProc);
		InsertSymbolsIntoLists(myMainWindow->coreDebugger->PIDs[i].hProc,
			myMainWindow->coreDebugger->PIDs[i].dwPID);
	}

	DisplayFunctionLists();
	return;
}

qtDLGFunctions::~qtDLGFunctions()
{

}

void qtDLGFunctions::GetValidMemoryParts(PTCHAR lpCurrentName,HANDLE hProc)
{
	DWORD64 CurAddress = NULL;
	PTCHAR	lpFileName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH *sizeof(TCHAR)),
			lpCurrentFileName = NULL;

	lpCurrentName = clsHelperClass::reverseStrip(lpCurrentName,'/');

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

void qtDLGFunctions::ParseMemoryRangeForFunctions(HANDLE hProc,quint64 BaseAddress,quint64 Size)
{
	LPVOID lpBuffer = malloc(Size);
	DWORD	//searchPattern	= NULL,
			searchPattern2	= 0x90909090,
			searchPattern3	= 0xCCCCCCCC,
			dwOldProtection = NULL,
			dwNewProtection = PAGE_EXECUTE_READWRITE;

	if(lpBuffer == NULL) return;
	if(!VirtualProtectEx(hProc,(LPVOID)BaseAddress,Size,dwNewProtection,&dwOldProtection))
		return;
	if(!ReadProcessMemory(hProc,(LPVOID)BaseAddress,lpBuffer,Size,NULL))
		return;

	VirtualProtectEx(hProc,(LPVOID)BaseAddress,Size,dwOldProtection,&dwNewProtection);

	//functionList.append(GetPossibleFunctionBeginning(BaseAddress,Size,searchPattern,lpBuffer,4));
	functionList.append(GetPossibleFunctionBeginning(BaseAddress,Size,searchPattern2,lpBuffer,4));
	functionList.append(GetPossibleFunctionBeginning(BaseAddress,Size,searchPattern3,lpBuffer,4));

	free(lpBuffer);
}

void qtDLGFunctions::InsertSymbolsIntoLists(HANDLE hProc,WORD PID)
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

void qtDLGFunctions::DisplayFunctionLists()
{
	for(quint64 i = 0; i < functionList.count(); i++)
	{
		tblFunctions->insertRow(tblFunctions->rowCount());
		// PID
		tblFunctions->setItem(tblFunctions->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(functionList[i].PID,8,16,QChar('0'))));

		// Func Name
		tblFunctions->setItem(tblFunctions->rowCount() - 1,1,
			new QTableWidgetItem(functionList[i].functionSymbol));

		// Func Offset
		tblFunctions->setItem(tblFunctions->rowCount() - 1,2,
			new QTableWidgetItem(QString("%1").arg(functionList[i].FunctionOffset,16,16,QChar('0'))));

		// Func Size
		tblFunctions->setItem(tblFunctions->rowCount() - 1,3,
			new QTableWidgetItem(QString("%1").arg(functionList[i].FunctionSize,8,16,QChar('0'))));
	}
}

QList<FunctionData> qtDLGFunctions::GetPossibleFunctionBeginning(quint64 StartOffset,quint64 Size,DWORD SearchPattern,LPVOID lpBuffer,int SpaceLen)
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
			newFunction.FunctionOffset = i + 1;
			newFunction.FunctionSize = GetPossibleFunctionEnding(i + 1,(StartOffset + Size) - i + 1,SearchPattern,(LPVOID)((quint64)lpBuffer + 1),SpaceLen);

			if(newFunction.FunctionSize > 4)
				functions.append(newFunction);
		}
	}
	return functions;
}

quint64 qtDLGFunctions::GetPossibleFunctionEnding(quint64 BaseAddress,quint64 Size,DWORD SearchPattern,LPVOID lpBuffer,int SpaceLen)
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

void qtDLGFunctions::OnCustomContextMenu(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblFunctions->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	menu.addAction(new QAction("Send to Disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGFunctions::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
	{
		emit ShowInDisAs(tblFunctions->item(_iSelectedRow,2)->text().toULongLong(0,16));
	}
}