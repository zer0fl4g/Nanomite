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
#include "qtDLGMemoryView.h"
#include "qtDLGHexView.h"

#include "clsMemManager.h"
#include "clsMemDump.h"

#include <Psapi.h>
#include <TlHelp32.h>

qtDLGMemoryView::qtDLGMemoryView(QWidget *parent, Qt::WFlags flags,qint32 processID)
	: QWidget(parent, flags),
	m_processID(processID),	
	m_processCountEntry(0)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);
	
	connect(tblMemoryView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenuRequested(QPoint)));
	connect(new QShortcut(QKeySequence("F5"),this),SIGNAL(activated()),this,SLOT(DisplayMemory()));
	connect(new QShortcut(Qt::Key_Escape,this),SIGNAL(activated()),this,SLOT(close()));

	// Init List
	tblMemoryView->horizontalHeader()->resizeSection(0,75);
	tblMemoryView->horizontalHeader()->resizeSection(1,135);
	tblMemoryView->horizontalHeader()->resizeSection(2,135);
	tblMemoryView->horizontalHeader()->resizeSection(3,150);
	tblMemoryView->horizontalHeader()->resizeSection(4,135);
	tblMemoryView->horizontalHeader()->setFixedHeight(21);

	// Display
	m_pMainWindow = qtDLGNanomite::GetInstance();

	m_processCountEnd = m_pMainWindow->coreDebugger->PIDs.size();
	for(size_t i = 0; i < m_pMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(m_pMainWindow->coreDebugger->PIDs[i].dwPID == m_processID)
		{
			m_processCountEntry = i;
			m_processCountEnd = i + 1;
			break;
		}
	}
	
	DisplayMemory();
}

qtDLGMemoryView::~qtDLGMemoryView()
{

}

void qtDLGMemoryView::OnCustomContextMenuRequested(QPoint qPoint)
{
	QMenu menu;

	m_selectedRow = tblMemoryView->indexAt(qPoint).row();
	if(m_selectedRow < 0) return;

	menu.addAction(new QAction("Send to HexView",this));
	menu.addAction(new QAction("Dump to File",this));
	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Base Address",this));
	submenu->addAction(new QAction("Size",this));
	submenu->addAction(new QAction("Module",this));
	submenu->addAction(new QAction("Type",this));
	submenu->addAction(new QAction("Access",this));

	menu.addMenu(submenu);

	DWORD currentProtection = GetPageProtectionFlags();
	if(currentProtection != -1)
	{
		QMenu *protectionMenu = menu.addMenu("Page Protection");

		QAction *execute = new QAction("PAGE_EXECUTE",this);
		if(currentProtection == PAGE_EXECUTE)
		{
			execute->setCheckable(true);
			execute->setChecked(true);
		}

		QAction *executeRead = new QAction("PAGE_EXECUTE_READ",this);
		if(currentProtection == PAGE_EXECUTE_READ)
		{
			executeRead->setCheckable(true);
			executeRead->setChecked(true);
		}

		QAction *executeReadWrite = new QAction("PAGE_EXECUTE_READWRITE",this);
		if(currentProtection == PAGE_EXECUTE_READWRITE)
		{
			executeReadWrite->setCheckable(true);
			executeReadWrite->setChecked(true);
		}

		QAction *executeWriteCopy = new QAction("PAGE_EXECUTE_WRITECOPY",this);
		if(currentProtection == PAGE_EXECUTE_WRITECOPY)
		{
			executeWriteCopy->setCheckable(true);
			executeWriteCopy->setChecked(true);
		}

		QAction *noaccess = new QAction("PAGE_NOACCESS",this);
		if(currentProtection == PAGE_NOACCESS)
		{
			noaccess->setCheckable(true);
			noaccess->setChecked(true);
		}

		QAction *readonly = new QAction("PAGE_READONLY",this);
		if(currentProtection == PAGE_READONLY)
		{
			readonly->setCheckable(true);
			readonly->setChecked(true);
		}

		QAction *writecopy = new QAction("PAGE_WRITECOPY",this);
		if(currentProtection == PAGE_WRITECOPY)
		{
			writecopy->setCheckable(true);
			writecopy->setChecked(true);
		}

		QAction *readwrite = new QAction("PAGE_READWRITE",this);
		if(currentProtection == PAGE_READWRITE)
		{
			readwrite->setCheckable(true);
			readwrite->setChecked(true);
		}

		protectionMenu->addAction(execute);
		protectionMenu->addAction(executeRead);
		protectionMenu->addAction(executeReadWrite);
		protectionMenu->addAction(executeWriteCopy);
		protectionMenu->addAction(noaccess);
		protectionMenu->addAction(readonly);
		protectionMenu->addAction(writecopy);
		protectionMenu->addAction(readwrite);
		menu.addMenu(protectionMenu);
	}

	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGMemoryView::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Send to HexView") == 0)
	{
		qtDLGHexView *newView = new qtDLGHexView(this,Qt::Window,tblMemoryView->item(m_selectedRow,0)->text().toULongLong(0,16),
			tblMemoryView->item(m_selectedRow,1)->text().toULongLong(0,16),
			tblMemoryView->item(m_selectedRow,2)->text().toULongLong(0,16));
		newView->show();
	}
	else if(QString().compare(pAction->text(),"Dump to File") == 0)
	{
		HANDLE hProc = clsDebugger::GetProcessHandleByPID(tblMemoryView->item(m_selectedRow,0)->text().toULongLong(0,16));

		clsMemDump memDump(hProc,
			(PTCHAR)tblMemoryView->item(m_selectedRow,3)->text().utf16(),
			tblMemoryView->item(m_selectedRow,1)->text().toULongLong(0,16),
			tblMemoryView->item(m_selectedRow,2)->text().toULongLong(0,16),
			this);
	}
	else if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3:%4:%5")
			.arg(tblMemoryView->item(m_selectedRow,0)->text())
			.arg(tblMemoryView->item(m_selectedRow,1)->text())
			.arg(tblMemoryView->item(m_selectedRow,2)->text())
			.arg(tblMemoryView->item(m_selectedRow,3)->text())
			.arg(tblMemoryView->item(m_selectedRow,4)->text()));
	}
	else if(QString().compare(pAction->text(),"Base Address") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblMemoryView->item(m_selectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"Size") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblMemoryView->item(m_selectedRow,2)->text());
	}
	else if(QString().compare(pAction->text(),"Module") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblMemoryView->item(m_selectedRow,3)->text());
	}
	else if(QString().compare(pAction->text(),"Type") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblMemoryView->item(m_selectedRow,4)->text());
	}
	else if(QString().compare(pAction->text(),"Access") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblMemoryView->item(m_selectedRow,5)->text());
	}
	else if(QString().compare(pAction->text(),"PAGE_EXECUTE") == 0)
		SetPageProctection(PAGE_EXECUTE);
	else if(QString().compare(pAction->text(),"PAGE_EXECUTE_READ") == 0)
		SetPageProctection(PAGE_EXECUTE_READ);
	else if(QString().compare(pAction->text(),"PAGE_EXECUTE_READWRITE") == 0)
		SetPageProctection(PAGE_EXECUTE_READWRITE);
	else if(QString().compare(pAction->text(),"PAGE_EXECUTE_WRITECOPY") == 0)
		SetPageProctection(PAGE_EXECUTE_WRITECOPY);
	else if(QString().compare(pAction->text(),"PAGE_NOACCESS") == 0)
		SetPageProctection(PAGE_NOACCESS);
	else if(QString().compare(pAction->text(),"PAGE_READONLY") == 0)
		SetPageProctection(PAGE_READONLY);
	else if(QString().compare(pAction->text(),"PAGE_WRITECOPY") == 0)
		SetPageProctection(PAGE_WRITECOPY);	
	else if(QString().compare(pAction->text(),"PAGE_READWRITE") == 0)
		SetPageProctection(PAGE_READWRITE);
}

void qtDLGMemoryView::DisplayMemory()
{
	PTCHAR sTemp = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	PTCHAR sTemp2 = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));

	MEMORY_BASIC_INFORMATION mbi;

	tblMemoryView->setRowCount(0);
	for(size_t i = m_processCountEntry; i < m_processCountEnd;i++)
	{
		quint64 dwAddress = NULL;
		while(VirtualQueryEx(m_pMainWindow->coreDebugger->PIDs[i].hProc,(LPVOID)dwAddress,&mbi,sizeof(mbi)))
		{
			tblMemoryView->insertRow(tblMemoryView->rowCount());

			// PID
			tblMemoryView->setItem(tblMemoryView->rowCount() -1,0,
				new QTableWidgetItem(QString().sprintf("%08X",m_pMainWindow->coreDebugger->PIDs[i].dwPID)));
			

			// Base Address
#ifdef _AMD64_
			wsprintf(sTemp,L"%016I64X",mbi.BaseAddress);
#else
			wsprintf(sTemp,L"%016X",mbi.BaseAddress);
#endif
			tblMemoryView->setItem(tblMemoryView->rowCount() -1,1,
				new QTableWidgetItem(QString().fromStdWString(sTemp)));

			// Size
			wsprintf(sTemp,L"%08X",mbi.RegionSize);
			tblMemoryView->setItem(tblMemoryView->rowCount() -1,2,
				new QTableWidgetItem(QString().fromStdWString(sTemp)));

			// Path
			size_t	iModPos = NULL,
					iModLen = NULL;

			memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
			memset(sTemp2,0,MAX_PATH * sizeof(TCHAR));

			iModLen = GetMappedFileName(m_pMainWindow->coreDebugger->PIDs[i].hProc,(LPVOID)dwAddress,sTemp2,MAX_PATH);
			if(iModLen > 0)
			{
				for(size_t i = iModLen; i > 0 ; i--)
				{
					if(sTemp2[i] == '\\')
					{
						iModPos = i;
						break;
					}
				}
						
				memcpy_s(sTemp,MAX_PATH,(LPVOID)&sTemp2[iModPos + 1],(iModLen - iModPos) * sizeof(TCHAR));

				tblMemoryView->setItem(tblMemoryView->rowCount() -1,3,
					new QTableWidgetItem(QString().fromStdWString(sTemp)));			
			}
			else
				tblMemoryView->setItem(tblMemoryView->rowCount() -1,3,
					new QTableWidgetItem(QString("")));		

			// Mem Type
			switch (mbi.State)
			{
			case MEM_FREE:			wsprintf(sTemp,L"%s",L"Free");		break;
			case MEM_RESERVE:       wsprintf(sTemp,L"%s",L"Reserve");	break;
			case MEM_COMMIT:
				switch (mbi.Type)
				{
				case MEM_FREE:		wsprintf(sTemp,L"%s",L"Free");     break;
				case MEM_RESERVE:   wsprintf(sTemp,L"%s",L"Reserve");  break;
				case MEM_IMAGE:		wsprintf(sTemp,L"%s",L"Image");    break;
				case MEM_MAPPED:    wsprintf(sTemp,L"%s",L"Mapped");   break;
				case MEM_PRIVATE:   wsprintf(sTemp,L"%s",L"Private");  break;
				}
				break;
			}
			tblMemoryView->setItem(tblMemoryView->rowCount() -1,4,
				new QTableWidgetItem(QString().fromStdWString(sTemp)));

			// Access
			wsprintf(sTemp,L"%s",L"Unknown");
			if(mbi.State == MEM_FREE) mbi.Protect = PAGE_NOACCESS;
			if(mbi.State == MEM_RESERVE) mbi.Protect = PAGE_NOACCESS;
			switch (mbi.Protect & ~(PAGE_GUARD | PAGE_NOCACHE | PAGE_WRITECOMBINE))
			{
			case PAGE_READONLY:          wsprintf(sTemp,L"%s",L"-R--"); break;
			case PAGE_READWRITE:         wsprintf(sTemp,L"%s",L"-RW-"); break;
			case PAGE_WRITECOPY:         wsprintf(sTemp,L"%s",L"-RWC"); break;
			case PAGE_EXECUTE:           wsprintf(sTemp,L"%s",L"E---"); break;
			case PAGE_EXECUTE_READ:      wsprintf(sTemp,L"%s",L"ER--"); break;
			case PAGE_EXECUTE_READWRITE: wsprintf(sTemp,L"%s",L"ERW-"); break;
			case PAGE_EXECUTE_WRITECOPY: wsprintf(sTemp,L"%s",L"ERWC"); break; 
			case PAGE_NOACCESS:          wsprintf(sTemp,L"%s",L"----"); break;
			}
			tblMemoryView->setItem(tblMemoryView->rowCount() -1,5,
				new QTableWidgetItem(QString().fromStdWString(sTemp)));

			dwAddress += mbi.RegionSize;
		}
	}
	clsMemManager::CFree(sTemp2);
	clsMemManager::CFree(sTemp);
}

DWORD qtDLGMemoryView::GetPageProtectionFlags()
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, false, tblMemoryView->item(m_selectedRow, 0)->text().toULongLong(0,16));
	if(hProcess == INVALID_HANDLE_VALUE || hProcess == NULL) return -1;

	MEMORY_BASIC_INFORMATION MBI;

	if(!VirtualQueryEx(hProcess, (LPVOID)tblMemoryView->item(m_selectedRow, 1)->text().toULongLong(0,16),&MBI,sizeof(MBI)))
	{
		CloseHandle(hProcess);
		return -1;
	}
	
	CloseHandle(hProcess);
	return MBI.Protect;
}

void qtDLGMemoryView::SetPageProctection(DWORD protectionFlag)
{
	HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION, false, tblMemoryView->item(m_selectedRow,0)->text().toULongLong(0,16));
	if(hProcess == INVALID_HANDLE_VALUE || hProcess == NULL) return;

	DWORD oldProtection = NULL;
	if(!VirtualProtectEx(hProcess, (LPVOID)tblMemoryView->item(m_selectedRow,1)->text().toULongLong(0,16),
		tblMemoryView->item(m_selectedRow,2)->text().toULongLong(0,16), protectionFlag, &oldProtection))
	{
		QMessageBox::critical(this,"Nanomite","Failed to change the Page protection!",QMessageBox::Ok,QMessageBox::Ok);
	}
	else
		DisplayMemory();

	CloseHandle(hProcess);
}