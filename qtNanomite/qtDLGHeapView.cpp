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
#include "qtDLGHeapView.h"
#include "qtDLGHexView.h"

#include "clsMemManager.h"
#include "clsMemDump.h"

#include <TlHelp32.h>

#include <QClipboard>

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
	connect(new QShortcut(QKeySequence("F5"),this),SIGNAL(activated()),this,SLOT(DisplayHeap()));

	tblHeapView->horizontalHeader()->resizeSection(0,75);
	tblHeapView->horizontalHeader()->resizeSection(1,75);
	tblHeapView->horizontalHeader()->resizeSection(2,135);
	tblHeapView->horizontalHeader()->resizeSection(3,135);
	tblHeapView->horizontalHeader()->resizeSection(4,135);

	myMainWindow = qtDLGNanomite::GetInstance();

	_iForEntry = 0;
	_iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(size_t i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == _iPID)
			_iForEntry = i; _iForEnd = i +1;
	}

	DisplayHeap();
}

qtDLGHeapView::~qtDLGHeapView()
{

}

void qtDLGHeapView::OnCustomContextMenuRequested(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblHeapBlocks->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	menu.addAction(new QAction("Send Offset To HexView",this));
	menu.addAction(new QAction("Dump Memory To File",this));
	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("HeapID",this));
	submenu->addAction(new QAction("Address",this));
	submenu->addAction(new QAction("Block Size",this));
	submenu->addAction(new QAction("Count",this));
	submenu->addAction(new QAction("Flags",this));

	connect(submenu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));	
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGHeapView::MenuCallback(QAction* pAction)
{
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
	else if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3:%4:%5:%6")
			.arg(tblHeapBlocks->item(_iSelectedRow,0)->text())
			.arg(tblHeapBlocks->item(_iSelectedRow,1)->text())
			.arg(tblHeapBlocks->item(_iSelectedRow,2)->text())
			.arg(tblHeapBlocks->item(_iSelectedRow,3)->text())
			.arg(tblHeapBlocks->item(_iSelectedRow,4)->text())
			.arg(tblHeapBlocks->item(_iSelectedRow,5)->text()));
	}
	else if(QString().compare(pAction->text(),"HeapID") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblHeapBlocks->item(_iSelectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"Address") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblHeapBlocks->item(_iSelectedRow,2)->text());
	}
	else if(QString().compare(pAction->text(),"Block Size") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblHeapBlocks->item(_iSelectedRow,3)->text());
	}
	else if(QString().compare(pAction->text(),"Block Count") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblHeapBlocks->item(_iSelectedRow,4)->text());
	}
	else if(QString().compare(pAction->text(),"Flags") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblHeapBlocks->item(_iSelectedRow,5)->text());
	}
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
			default:
				tblHeapBlocks->setItem(tblHeapBlocks->rowCount() - 1,5,
					new QTableWidgetItem(""));
				break;
			}

			he.dwSize = sizeof(HEAPENTRY32);
		}while(Heap32Next(&he));
	}
}

void qtDLGHeapView::DisplayHeap()
{
	tblHeapView->setRowCount(0);
	tblHeapBlocks->setRowCount(0);

	for(size_t i = _iForEntry; i < _iForEnd;i++)
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