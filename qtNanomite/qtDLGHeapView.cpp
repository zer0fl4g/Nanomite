#include "qtDLGHeapView.h"
#include "qtDLGNanomite.h"
#include "qtDLGHexView.h"

#include "clsMemManager.h"
#include "clsMemDump.h"

#include <TlHelp32.h>

qtDLGHeapView::qtDLGHeapView(QWidget *parent, Qt::WFlags flags,int iPID)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);

	_iPID = iPID;
	_iSelectedRow = -1;

	connect(tblHeapBlocks,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenuRequested(QPoint)));
	connect(tblHeapView,SIGNAL(itemSelectionChanged()),this,SLOT(OnSelectionChanged()));

	tblHeapView->horizontalHeader()->resizeSection(0,75);
	tblHeapView->horizontalHeader()->resizeSection(1,75);
	tblHeapView->horizontalHeader()->resizeSection(2,135);
	tblHeapView->horizontalHeader()->resizeSection(3,135);
	tblHeapView->horizontalHeader()->resizeSection(4,135);

	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	int iForEntry = 0;
	int iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(int i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == _iPID)
			iForEntry = i; iForEnd = i +1;
	}

	for(int i = iForEntry; i < iForEnd;i++)
	{
		HEAPLIST32 heapList;
		heapList.dwSize = sizeof(HEAPLIST32);
		HANDLE hHeapSnap = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST,myMainWindow->coreDebugger->PIDs[i].dwPID);

		if(hHeapSnap != INVALID_HANDLE_VALUE)
		{
			if(Heap32ListFirst(hHeapSnap,&heapList))
			{
				do
				{
					quint64 commitedSize	= NULL,
							usedSize		= NULL,
							BlockCount		= NULL;
					HEAPENTRY32 he;
					ZeroMemory(&he, sizeof(HEAPENTRY32));
					he.dwSize = sizeof(HEAPENTRY32);

					if(Heap32First(&he,myMainWindow->coreDebugger->PIDs[i].dwPID,heapList.th32HeapID))
					{
						do
						{
							commitedSize += he.dwBlockSize;
							BlockCount++;

							he.dwSize = sizeof(HEAPENTRY32);
						}while(Heap32Next(&he));
					}

					tblHeapView->insertRow(tblHeapView->rowCount());
					// PID
					tblHeapView->setItem(tblHeapView->rowCount() - 1,0,
						new QTableWidgetItem(QString("%1").arg(heapList.th32ProcessID,8,16,QChar('0'))));
					// Base Offset
					tblHeapView->setItem(tblHeapView->rowCount() - 1,1,
						new QTableWidgetItem(QString("%1").arg(heapList.th32HeapID,8,16,QChar('0'))));
					// Used Size
					tblHeapView->setItem(tblHeapView->rowCount() - 1,2,
						new QTableWidgetItem(QString("%1").arg(usedSize,16,10,QChar('0'))));
					// Commited Size
					tblHeapView->setItem(tblHeapView->rowCount() - 1,3,
						new QTableWidgetItem(QString("%1").arg(commitedSize,16,10,QChar('0'))));
					// Block Count
					tblHeapView->setItem(tblHeapView->rowCount() - 1,4,
						new QTableWidgetItem(QString("%1").arg(BlockCount,16,10,QChar('0'))));
					// Flags
					tblHeapView->setItem(tblHeapView->rowCount() - 1,5,
						new QTableWidgetItem(QString("%1").arg(heapList.dwFlags,8,16,QChar('0'))));

					heapList.dwSize = sizeof(HEAPLIST32);
				}while(Heap32ListNext(hHeapSnap,&heapList));
			}
			CloseHandle(hHeapSnap);
		}
	}
}

qtDLGHeapView::~qtDLGHeapView()
{

}

void qtDLGHeapView::OnCustomContextMenuRequested(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblHeapBlocks->indexAt(qPoint).row();

	menu.addAction(new QAction("Send Offset To HexView",this));
	menu.addAction(new QAction("Dump Memory To File",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));


	menu.exec(QCursor::pos());
}

void qtDLGHeapView::MenuCallback(QAction* pAction)
{
	if(_iSelectedRow == -1) return;

	if(QString().compare(pAction->text(),"Send Offset To HexView") == 0)
	{
		qtDLGHexView *newView = new qtDLGHexView(this,Qt::Window,tblHeapBlocks->item(_iSelectedRow,0)->text().toULongLong(0,16),
			tblHeapBlocks->item(_iSelectedRow,2)->text().toULongLong(0,16),
			tblHeapBlocks->item(_iSelectedRow,3)->text().toULongLong(0,16));
		newView->show();
	}
	else if(QString().compare(pAction->text(),"Dump Memory To File") == 0)
	{
		HANDLE hProc = clsDebugger::GetProcessHandleByPID(tblHeapBlocks->item(_iSelectedRow,0)->text().toULongLong(0,16));

		clsMemDump memDump(hProc,
			L"Heap",
			tblHeapBlocks->item(_iSelectedRow,2)->text().toULongLong(0,16),
			tblHeapBlocks->item(_iSelectedRow,3)->text().toULongLong(0,16));
	}
	_iSelectedRow = -1;
}

void qtDLGHeapView::OnSelectionChanged()
{
	HEAPENTRY32 he;
	ZeroMemory(&he, sizeof(HEAPENTRY32));
	he.dwSize = sizeof(HEAPENTRY32);

	if(tblHeapView->selectedItems().size() <= 0) 
	{
		tblHeapBlocks->setRowCount(0);
		return;
	}

	quint64 PID = tblHeapView->selectedItems()[0]->text().toULongLong(0,16),
			heapID = tblHeapView->selectedItems()[1]->text().toULongLong(0,16),
			blockCount = NULL;

	if(Heap32First(&he,PID,heapID))
	{
		tblHeapBlocks->setRowCount(0);
		do
		{
			tblHeapBlocks->insertRow(tblHeapBlocks->rowCount());
			// PID
			tblHeapBlocks->setItem(tblHeapBlocks->rowCount() - 1,0,
				new QTableWidgetItem(QString().sprintf("%08X",PID)));

			// Heap ID
			tblHeapBlocks->setItem(tblHeapBlocks->rowCount() - 1,1,
				new QTableWidgetItem(QString().sprintf("%08X",he.th32HeapID)));

			// Base Address
			tblHeapBlocks->setItem(tblHeapBlocks->rowCount() - 1,2,
				new QTableWidgetItem(QString().sprintf("%08X",he.dwAddress)));

			// Allocated Size
			tblHeapBlocks->setItem(tblHeapBlocks->rowCount() - 1,3,
				new QTableWidgetItem(QString().sprintf("%08X",he.dwBlockSize)));

			// Block Count
			tblHeapBlocks->setItem(tblHeapBlocks->rowCount() - 1,4,
				new QTableWidgetItem(QString().sprintf("%d",++blockCount)));

			// Flags
			switch(he.dwFlags)
			{
			case LF32_FIXED:
				tblHeapBlocks->setItem(tblHeapBlocks->rowCount() - 1,5,
					new QTableWidgetItem(QString("LF32_FIXED")));
				break;
			case LF32_FREE:
				tblHeapBlocks->setItem(tblHeapBlocks->rowCount() - 1,5,
					new QTableWidgetItem(QString("LF32_FREE")));
				break;
			case LF32_MOVEABLE:
				tblHeapBlocks->setItem(tblHeapBlocks->rowCount() - 1,5,
					new QTableWidgetItem(QString("LF32_MOVEABLE")));
				break;
			}

			he.dwSize = sizeof(HEAPENTRY32);
		}while(Heap32Next(&he));
	}
}