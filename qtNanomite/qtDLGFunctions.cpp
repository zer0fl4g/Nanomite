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
		InsertSymbolsIntoLists(myMainWindow->coreDebugger->PIDs[i].hProc);
	}

	DisplayFunctionLists();
	return;
}

qtDLGFunctions::~qtDLGFunctions()
{

}

void qtDLGFunctions::GetValidMemoryParts(PTCHAR lpCurrentName,HANDLE hProc)
{
	DWORD64 ModuleBase = NULL,
			ModuleSize = NULL,
			CurAddress = NULL;
	PTCHAR	lpFileName = (PTCHAR)malloc(MAX_PATH *sizeof(TCHAR)),
			lpCurrentFileName = NULL;
	bool	bWeGotIt = false;

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
					if(!bWeGotIt)
					{
						bWeGotIt = true;
						ModuleBase = (DWORD64)mbi.BaseAddress;
					}

					ModuleSize += mbi.RegionSize;
				}
				else
				{
					if(bWeGotIt)
					{
						free(lpCurrentFileName);
						break;
					}
				}

				free(lpCurrentFileName);
			}
		}
		CurAddress += mbi.RegionSize;
	}

	ParseMemoryRangeForFunctions(hProc,ModuleBase,ModuleSize);
}

void qtDLGFunctions::ParseMemoryRangeForFunctions(HANDLE hProc,quint64 BaseAddress,quint64 Size)
{
	LPVOID lpBuffer = malloc(Size);
	DWORD	searchPattern	= NULL,
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

	//functionList.append(GetPossibleFunctionBeginning(BaseAddress,Size,searchPattern,lpBuffer));
	functionList.append(GetPossibleFunctionBeginning(BaseAddress,Size,searchPattern2,lpBuffer));
	functionList.append(GetPossibleFunctionBeginning(BaseAddress,Size,searchPattern3,lpBuffer));

	free(lpBuffer);
}

void qtDLGFunctions::InsertSymbolsIntoLists(HANDLE hProc)
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
			new QTableWidgetItem(QString("0")));

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

QList<FunctionData> qtDLGFunctions::GetPossibleFunctionBeginning(quint64 StartOffset,quint64 Size,DWORD SearchPattern,LPVOID lpBuffer)
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

		if(counter > 4)
		{
			// possible beginning
			FunctionData newFunction;
			newFunction.FunctionOffset = i + 1;
			newFunction.FunctionSize = GetPossibleFunctionEnding(i + 1,(StartOffset + Size) - i + 1,SearchPattern,(LPVOID)((quint64)lpBuffer + 1));

			if(newFunction.FunctionSize > 4)
				functions.append(newFunction);
		}
	}
	return functions;
}

quint64 qtDLGFunctions::GetPossibleFunctionEnding(quint64 BaseAddress,quint64 Size,DWORD SearchPattern,LPVOID lpBuffer)
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

		if(counter > 4)
			return i - BaseAddress - counter;
	}
	return 0;
}

void qtDLGFunctions::OnCustomContextMenu(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblFunctions->indexAt(qPoint).row();

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