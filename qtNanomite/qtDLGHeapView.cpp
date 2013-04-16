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
	 // // Create debug buffer
	 // PDEBUG_BUFFER db = (PDEBUG_BUFFER)clsAPIImport::pRtlCreateQueryDebugBuffer(0,false); 
	 // HeapBlock hb = {0,0,0,0};
	 // int iHeapCount = 0;

	 // // Get process heap data
	 // clsAPIImport::pRtlQueryProcessDebugInformation(myMainWindow->coreDebugger->PIDs[i].dwPID, PDI_HEAPS | PDI_HEAP_BLOCKS,(ULONG)db);

	 // ULONG heapNodeCount = db->HeapInformation ? *PULONG(db->HeapInformation) : 0;

	 // PDEBUG_HEAP_INFORMATION heapInfo = PDEBUG_HEAP_INFORMATION(PULONG(db->HeapInformation) + 1);

	 // // Go through each of the heap nodes and dispaly the information
	 // for (unsigned int iNode = 0; iNode < heapNodeCount; iNode++) 
	 // {
		//  // Now enumerate all blocks within this heap node...
		//  memset(&hb,0,sizeof(hb));

		//  if(GetFirstHeapBlock(&heapInfo[iNode],&hb))
		//  {
		//	do
		//	{
		//		tblHeapView->insertRow(tblHeapView->rowCount());
		//		// PID
		//		tblHeapView->setItem(tblHeapView->rowCount() - 1,0,
		//			new QTableWidgetItem(QString().sprintf("%08X",myMainWindow->coreDebugger->PIDs[i].dwPID)));

		//		// Heap ID
		//		tblHeapView->setItem(tblHeapView->rowCount() - 1,1,
		//			new QTableWidgetItem(QString().sprintf("%08X",heapInfo[iNode].Base)));

		//		// Base Address
		//		tblHeapView->setItem(tblHeapView->rowCount() - 1,2,
		//			new QTableWidgetItem(QString().sprintf("%08X",hb.dwAddress)));

		//		// Block Size
		//		tblHeapView->setItem(tblHeapView->rowCount() - 1,3,
		//			new QTableWidgetItem(QString().sprintf("%08X",hb.dwSize)));

		//		// Block Count
		//		tblHeapView->setItem(tblHeapView->rowCount() - 1,4,
		//			new QTableWidgetItem(QString().sprintf("%d",++iHeapCount)));

		//		// Flags
		//		switch(hb.dwFlags)
		//		{
		//		case LF32_FIXED:
		//			tblHeapView->setItem(tblHeapView->rowCount() - 1,5,
		//				new QTableWidgetItem(QString("LF32_FIXED")));
		//			break;
		//		case LF32_FREE:
		//			tblHeapView->setItem(tblHeapView->rowCount() - 1,5,
		//				new QTableWidgetItem(QString("LF32_FREE")));
		//			break;
		//		case LF32_MOVEABLE:
		//			tblHeapView->setItem(tblHeapView->rowCount() - 1,5,
		//				new QTableWidgetItem(QString("LF32_MOVEABLE")));
		//			break;
		//		}

		//	}while(GetNextHeapBlock(&heapInfo[iNode],&hb));

		//	//printf("\n Base Address = 0x%.8x", heapInfo[i].Base);
		//	//printf("\n Block count = %d", heapInfo[i].BlockCount);
		//	//printf("\n Committed Size= 0x%.8x", heapInfo[i].Committed);
		//	//printf("\n Allocated Size = 0x%.8x", heapInfo[i].Allocated);
		//	//printf("\n Flags = 0x%.8x", heapInfo[i].Flags);
		//}
	 // }
	 // // Clean up the buffer
	 // clsAPIImport::pRtlDestroyQueryDebugBuffer((ULONG)db);

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
							tblHeapView->setItem(tblHeapView->rowCount() - 1,0,
								new QTableWidgetItem(QString().sprintf("%08X",myMainWindow->coreDebugger->PIDs[i].dwPID)));

							// Heap ID
							tblHeapView->setItem(tblHeapView->rowCount() - 1,1,
								new QTableWidgetItem(QString().sprintf("%08X",he.th32HeapID)));

							// Base Address
							tblHeapView->setItem(tblHeapView->rowCount() - 1,2,
								new QTableWidgetItem(QString().sprintf("%08X",he.dwAddress)));

							// Allocated Size
							tblHeapView->setItem(tblHeapView->rowCount() - 1,3,
								new QTableWidgetItem(QString().sprintf("%08X",he.dwBlockSize)));

							// Block Count
							tblHeapView->setItem(tblHeapView->rowCount() - 1,4,
								new QTableWidgetItem(QString().sprintf("%d",++iHeapCount)));

							// Flags
							switch(he.dwFlags)
							{
							case LF32_FIXED:
								tblHeapView->setItem(tblHeapView->rowCount() - 1,5,
									new QTableWidgetItem(QString("LF32_FIXED")));
								break;
							case LF32_FREE:
								tblHeapView->setItem(tblHeapView->rowCount() - 1,5,
									new QTableWidgetItem(QString("LF32_FREE")));
								break;
							case LF32_MOVEABLE:
								tblHeapView->setItem(tblHeapView->rowCount() - 1,5,
									new QTableWidgetItem(QString("LF32_MOVEABLE")));
								break;
							}

							he.dwSize = sizeof(HEAPENTRY32);
						}while(Heap32Next(&he));
					}
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

	_iSelectedRow = tblHeapView->indexAt(qPoint).row();

	menu.addAction(new QAction("Send Offset To HexView",this));
	menu.addAction(new QAction("Dump Memory To File",this));
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
	else if(QString().compare(pAction->text(),"Dump Memory To File") == 0)
	{
		HANDLE hProc = clsDebugger::GetProcessHandleByPID(tblHeapView->item(_iSelectedRow,0)->text().toULongLong(0,16));

		clsMemDump memDump(hProc,
			L"Heap",
			tblHeapView->item(_iSelectedRow,2)->text().toULongLong(0,16),
			tblHeapView->item(_iSelectedRow,3)->text().toULongLong(0,16));
	}
}
//
//bool qtDLGHeapView::GetFirstHeapBlock(PDEBUG_HEAP_INFORMATION curHeapNode, HeapBlock *hb)
//{
//  int *block;
//
//  hb->reserved = 0;
//  hb->dwAddress = 0;
//  hb->dwFlags = 0;
//
//  block = (int*) curHeapNode->Blocks;
//
//  while( ( *(block+1) & 2 ) == 2 )
//  {
//    hb->reserved++;
//    hb->dwAddress = (void *) ( *(block+3) + curHeapNode->Granularity );
//    block = block + 4;
//    hb->dwSize = *block;
//  }
//
//  // Update the flags...
//  USHORT flags = *(block+1);
//
//  if( ( flags & 0xF1 ) != 0 || ( flags & 0x0200 ) != 0 )
//    hb->dwFlags = 1;
//  else if( (flags & 0x20) != 0 )
//         hb->dwFlags = 4;
//       else if( (flags & 0x0100) != 0 )
//              hb->dwFlags = 2;
//
//   return TRUE;
//}
//
//bool qtDLGHeapView::GetNextHeapBlock(PDEBUG_HEAP_INFORMATION curHeapNode, HeapBlock *hb)
//{
//  int *block;
//
//  hb->reserved++;
//  block = (int*) curHeapNode->Blocks;
//
//  // Make it point to next block address entry
//  block = block + hb->reserved * 4; 
//
//  if( hb->reserved > curHeapNode->BlockCount)
//        return false;
//
//  if((*(block + 1) & 2) == 2 )
//  {
//    do
//    {
//      // new address = curBlockAddress + Granularity ;
//      hb->dwAddress = (void *) ( *(block+3) + curHeapNode->Granularity );
//
//      // If all the blocks have been enumerated....exit
//      if( hb->reserved > curHeapNode->BlockCount)
//        return false;
//
//      hb->reserved++;
//      block = (int*)(block + 4); //move to next block
//      hb->dwSize = *block;
//     }
//     while((*(block+1)& 2) == 2);
//  }
//  else
//  {
//    // New Address = prev Address + prev block size ;
//    hb->dwAddress = (void*) ( (int)hb->dwAddress + hb->dwSize );
//    hb->dwSize = *block;
//  }
//
//  // Update the flags...
//  USHORT flags = *( block+1);
//
//  if( ( flags & 0xF1 ) != 0 || ( flags & 0x0200 ) != 0 )
//    hb->dwFlags = 1;
//  else if( (flags & 0x20) != 0 )
//         hb->dwFlags = 4;
//       else if( (flags & 0x0100) != 0 )
//              hb->dwFlags = 2;
//
//  return TRUE;
//}