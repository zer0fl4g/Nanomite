#include "qtDLGHeapView.h"
#include "qtDLGNanomite.h"
#include "qtDLGHexView.h"

#include <TlHelp32.h>

qtDLGHeapView::qtDLGHeapView(QWidget *parent, Qt::WFlags flags,int iPID)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);

	_iPID = iPID;


	connect(tblHeapView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenuRequested(QPoint)));

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
		int iHeapCount = 0;
		HANDLE hHeapSnap = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST,myMainWindow->coreDebugger->PIDs[i].dwPID);

		heapList.dwSize = sizeof(HEAPLIST32);

		if(hHeapSnap != INVALID_HANDLE_VALUE)
		{
			if(Heap32ListFirst(hHeapSnap,&heapList))
			{
				do
				{
					HEAPENTRY32 he;
					ZeroMemory(&he, sizeof(HEAPENTRY32));
					he.dwSize = sizeof(HEAPENTRY32);

					if(Heap32First(&he,myMainWindow->coreDebugger->PIDs[i].dwPID,heapList.th32HeapID))
					{
						do
						{
							tblHeapView->insertRow(tblHeapView->rowCount());
							// PID
							tblHeapView->setItem(tblHeapView->rowCount() - 1,0, new QTableWidgetItem(QString().sprintf("%08X",myMainWindow->coreDebugger->PIDs[i].dwPID)));

							// Heap ID
							tblHeapView->setItem(tblHeapView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("%08X",he.th32HeapID)));

							// Base Address
							tblHeapView->setItem(tblHeapView->rowCount() - 1,2,new QTableWidgetItem(QString().sprintf("%08X",he.dwAddress)));

							// Allocated Size
							tblHeapView->setItem(tblHeapView->rowCount() - 1,3,new QTableWidgetItem(QString().sprintf("%08X",he.dwBlockSize)));

							// Block Count
							tblHeapView->setItem(tblHeapView->rowCount() - 1,4,new QTableWidgetItem(QString().sprintf("%d",++iHeapCount)));

							// Flags
							tblHeapView->setItem(tblHeapView->rowCount() - 1,5,new QTableWidgetItem(QString().sprintf("%08X",he.dwFlags)));

							he.dwSize = sizeof(HEAPENTRY32);
						} while( Heap32Next(&he) );
					}
					heapList.dwSize = sizeof(HEAPLIST32);
				} while (Heap32ListNext(hHeapSnap,&heapList));
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
	QAction *qAction;
	QMenu menu;

	_iSelectedRow = tblHeapView->indexAt(qPoint).row();

	qAction = new QAction("Send Offset To HexView",this);
	menu.addAction(qAction);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));


	menu.exec(QCursor::pos());
}

void qtDLGHeapView::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Send Offset To HexView") == 0)
	{
		qtDLGHexView *newView = new qtDLGHexView(this,Qt::Window,tblHeapView->item(_iSelectedRow,0)->text().toULongLong(0,16),
			tblHeapView->item(_iSelectedRow,2)->text().toULongLong(0,16),
			tblHeapView->item(_iSelectedRow,3)->text().toULongLong(0,16));
		newView->show();
	}
}