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
#include "qtDLGDetailInfo.h"
#include "qtDLGPEEditor.h"
#include "qtDLGNanomite.h"
#include "qtDLGRegEdit.h"
#include "qtDLGTIBView.h"
#include "qtDLGPEBView.h"

#include "clsMemManager.h"
#include "clsHelperClass.h"
#include "clsPEManager.h"
#include "clsAPIImport.h"

#include <QtCore>
#include <QMenu>

using namespace std;

qtDLGDetailInfo::qtDLGDetailInfo(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setLayout(verticalLayout);

	// List DetInfo  Processes
	tblPIDs->horizontalHeader()->resizeSection(0,135);
	tblPIDs->horizontalHeader()->resizeSection(1,135);
	tblPIDs->horizontalHeader()->resizeSection(2,135);

	// List DetInfo  Threads
	tblTIDs->horizontalHeader()->resizeSection(0,135);
	tblTIDs->horizontalHeader()->resizeSection(1,135);
	tblTIDs->horizontalHeader()->resizeSection(2,135);
	tblTIDs->horizontalHeader()->resizeSection(3,135);

	// List DetInfo  Exceptions
	tblExceptions->horizontalHeader()->resizeSection(0,135);
	tblExceptions->horizontalHeader()->resizeSection(1,135);
	tblExceptions->horizontalHeader()->resizeSection(2,140);

	// List DetInfo  Modules
	tblModules->horizontalHeader()->resizeSection(0,135);
	tblModules->horizontalHeader()->resizeSection(1,135);
	tblModules->horizontalHeader()->resizeSection(2,135);

	connect(tblTIDs,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomTIDContextMenu(QPoint)));
	connect(tblPIDs,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomPIDContextMenu(QPoint)));
	connect(tblExceptions,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomExceptionContextMenu(QPoint)));
	connect(tblModules,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomModuleContextMenu(QPoint)));
}

qtDLGDetailInfo::~qtDLGDetailInfo()
{

}

void qtDLGDetailInfo::OnCustomPIDContextMenu(QPoint qPoint)
{
	if(tblPIDs->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblPIDs->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	_SelectedOffset = tblPIDs->item(_iSelectedRow,1)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	menu.addAction(new QAction("Show PBI/PEB",this));

	int ProcessPriority = GetProcessPriorityByPid(tblPIDs->item(_iSelectedRow,0)->text().toULongLong(0,16));
	if(ProcessPriority != 0)
	{
		QMenu *submenu = menu.addMenu("Set Process Priority");

		QAction *newProcessPrioReal = new QAction("Realtime",this);
		if(ProcessPriority == REALTIME_PRIORITY_CLASS)
		{
			newProcessPrioReal->setCheckable(true);
			newProcessPrioReal->setChecked(true);
		}

		QAction *newProcessPrioHigh = new QAction("High",this);
		if(ProcessPriority == HIGH_PRIORITY_CLASS)
		{
			newProcessPrioHigh->setCheckable(true);
			newProcessPrioHigh->setChecked(true);
		}

		QAction *newProcessPrioAbNormal = new QAction("Above Normal",this);
		if(ProcessPriority == ABOVE_NORMAL_PRIORITY_CLASS)
		{
			newProcessPrioAbNormal->setCheckable(true);
			newProcessPrioAbNormal->setChecked(true);
		}

		QAction *newProcessPrioNormal = new QAction("Normal",this);
		if(ProcessPriority == NORMAL_PRIORITY_CLASS)
		{
			newProcessPrioNormal->setCheckable(true);
			newProcessPrioNormal->setChecked(true);
		}

		QAction *newProcessPrioBeNormal = new QAction("Below Normal",this);
		if(ProcessPriority == BELOW_NORMAL_PRIORITY_CLASS)
		{
			newProcessPrioBeNormal->setCheckable(true);
			newProcessPrioBeNormal->setChecked(true);
		}

		QAction *newProcessPrioLowest = new QAction("Idle",this);
		if(ProcessPriority == IDLE_PRIORITY_CLASS)
		{
			newProcessPrioLowest->setCheckable(true);
			newProcessPrioLowest->setChecked(true);
		}

		submenu->addAction(newProcessPrioReal);
		submenu->addAction(newProcessPrioHigh);
		submenu->addAction(newProcessPrioAbNormal);
		submenu->addAction(newProcessPrioNormal);
		submenu->addAction(newProcessPrioBeNormal);
		submenu->addAction(newProcessPrioLowest);
		menu.addMenu(submenu);
	}

	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(PIDMenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::OnCustomTIDContextMenu(QPoint qPoint)
{
	if(tblTIDs->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblTIDs->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	_SelectedOffset = tblTIDs->item(_iSelectedRow,2)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	menu.addAction(new QAction("Show Registers",this));
	menu.addAction(new QAction("Show TBI/TEB",this));
	menu.addAction(new QAction("Suspend",this));
	menu.addAction(new QAction("Resume",this));

	int ThreadPriority = GetThreadPriorityByTid(tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16));
	if(ThreadPriority != THREAD_PRIORITY_ERROR_RETURN)
	{
		QMenu *submenu = menu.addMenu("Set Thread Priority");

		QAction *newThreadPrioHigh = new QAction("Highest",this);
		if(ThreadPriority == THREAD_PRIORITY_HIGHEST)
		{
			newThreadPrioHigh->setCheckable(true);
			newThreadPrioHigh->setChecked(true);
		}

		QAction *newThreadPrioAbNormal = new QAction("Above Normal",this);
		if(ThreadPriority == THREAD_PRIORITY_ABOVE_NORMAL)
		{
			newThreadPrioAbNormal->setCheckable(true);
			newThreadPrioAbNormal->setChecked(true);
		}

		QAction *newThreadPrioNormal = new QAction("Normal",this);
		if(ThreadPriority == THREAD_PRIORITY_NORMAL)
		{
			newThreadPrioNormal->setCheckable(true);
			newThreadPrioNormal->setChecked(true);
		}

		QAction *newThreadPrioBeNormal = new QAction("Below Normal",this);
		if(ThreadPriority == THREAD_PRIORITY_BELOW_NORMAL)
		{
			newThreadPrioBeNormal->setCheckable(true);
			newThreadPrioBeNormal->setChecked(true);
		}

		QAction *newThreadPrioLowest = new QAction("Lowest",this);
		if(ThreadPriority == THREAD_PRIORITY_BELOW_NORMAL)
		{
			newThreadPrioLowest->setCheckable(true);
			newThreadPrioLowest->setChecked(true);
		}

		submenu->addAction(newThreadPrioHigh);
		submenu->addAction(newThreadPrioAbNormal);
		submenu->addAction(newThreadPrioNormal);
		submenu->addAction(newThreadPrioBeNormal);
		submenu->addAction(newThreadPrioLowest);
		menu.addMenu(submenu);
	}
	
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::OnCustomExceptionContextMenu(QPoint qPoint)
{
	if(tblExceptions->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblExceptions->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	_SelectedOffset = tblExceptions->item(_iSelectedRow,0)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::OnCustomModuleContextMenu(QPoint qPoint)
{
	if(tblModules->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblModules->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	_SelectedOffset = tblModules->item(_iSelectedRow,1)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	menu.addAction(new QAction("Open Module in PE View",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::PIDMenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Show Offset in disassembler") == 0)
	{
		if(_SelectedOffset >= 0)
		{
			emit ShowInDisassembler(_SelectedOffset);
			_SelectedOffset = NULL;
		}
	}
	else if(QString().compare(pAction->text(),"Show PBI/PEB") == 0)
	{
		DWORD selectedPID = tblPIDs->item(_iSelectedRow,0)->text().toULongLong(0,16);
		HANDLE selectedProcessHandle = qtDLGNanomite::GetInstance()->coreDebugger->GetProcessHandleByPID(selectedPID);

		qtDLGPEBView *newPEB = new qtDLGPEBView(selectedProcessHandle, this, Qt::Window);
		newPEB->show();
	}
	else if(QString().compare(pAction->text(),"Realtime") == 0)
		SetProcessPriorityByPid(tblPIDs->item(_iSelectedRow,0)->text().toULongLong(0,16),REALTIME_PRIORITY_CLASS);
	else if(QString().compare(pAction->text(),"High") == 0)
		SetProcessPriorityByPid(tblPIDs->item(_iSelectedRow,0)->text().toULongLong(0,16),HIGH_PRIORITY_CLASS);
	else if(QString().compare(pAction->text(),"Above Normal") == 0)
		SetProcessPriorityByPid(tblPIDs->item(_iSelectedRow,0)->text().toULongLong(0,16),ABOVE_NORMAL_PRIORITY_CLASS);
	else if(QString().compare(pAction->text(),"Normal") == 0)
		SetProcessPriorityByPid(tblPIDs->item(_iSelectedRow,0)->text().toULongLong(0,16),NORMAL_PRIORITY_CLASS);
	else if(QString().compare(pAction->text(),"Below Normal") == 0)
		SetProcessPriorityByPid(tblPIDs->item(_iSelectedRow,0)->text().toULongLong(0,16),BELOW_NORMAL_PRIORITY_CLASS);
	else if(QString().compare(pAction->text(),"Idle") == 0)
		SetProcessPriorityByPid(tblPIDs->item(_iSelectedRow,0)->text().toULongLong(0,16),IDLE_PRIORITY_CLASS);
}

void qtDLGDetailInfo::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Open Module in PE View") == 0)
	{
		std::wstring temp = tblModules->item(_iSelectedRow,3)->text().toStdWString();
		emit OpenFileInPEManager(temp,-1);
		qtDLGPEEditor *dlgPEEditor = new qtDLGPEEditor(clsPEManager::GetInstance(),this,Qt::Window,-1,temp);
		dlgPEEditor->show();
	}
	else if(QString().compare(pAction->text(),"Suspend") == 0)
	{
		DWORD ThreadID = tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16);
		HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME,false,ThreadID);
		if(hThread == INVALID_HANDLE_VALUE) 
		{
			MessageBoxW(NULL,L"ERROR, have not been able to open this Thread!",L"Nanomite",MB_OK);
			return;
		}

		if(SuspendThread(hThread) != -1)
			tblTIDs->item(_iSelectedRow,4)->setText("Suspended");
		else
			MessageBoxW(NULL,L"ERROR, have not been able to suspend this Thread!",L"Nanomite",MB_OK);

		CloseHandle(hThread);
	}
	else if(QString().compare(pAction->text(),"Resume") == 0)
	{
		DWORD ThreadID = tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16);
		HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME,false,ThreadID);
		if(hThread == INVALID_HANDLE_VALUE) 
		{
			MessageBoxW(NULL,L"ERROR, have not been able to open this Thread!",L"Nanomite",MB_OK);
			return;
		}

		if(ResumeThread(hThread) != -1)
			tblTIDs->item(_iSelectedRow,4)->setText("Running");
		else
			MessageBoxW(NULL,L"ERROR, have not been able to resume this Thread!",L"Nanomite",MB_OK);

		CloseHandle(hThread);
	}
	else if(QString().compare(pAction->text(),"Show TBI/TEB") == 0)
	{
		DWORD	ThreadID = tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16),
				procID = tblTIDs->item(_iSelectedRow,0)->text().toULongLong(0,16);

		HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION,false,ThreadID);
		if(hThread == INVALID_HANDLE_VALUE) 
		{
			MessageBoxW(NULL,L"ERROR, have not been able to open this Thread!",L"Nanomite",MB_OK);
			return;
		}

		qtDLGTIBView *newTIBView = new qtDLGTIBView(qtDLGNanomite::GetInstance()->coreDebugger->GetProcessHandleByPID(procID),hThread,this,Qt::Window);
		newTIBView->show();

		CloseHandle(hThread);
	}
	else if(QString().compare(pAction->text(),"Show Registers") == 0)
	{
		DWORD ThreadID = tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16);
		HANDLE hThread = OpenThread(THREAD_GETSET_CONTEXT,false,ThreadID);
		HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION,false,tblTIDs->item(_iSelectedRow,0)->text().toULongLong(0,16));

		if(hThread == INVALID_HANDLE_VALUE && hProc == NULL) 
		{
			MessageBoxW(NULL,L"ERROR, have not been able to open this Thread!",L"Nanomite",MB_OK);
			return;
		}
		
		qtDLGRegEdit *newRegEditWindow;
		
		SuspendThread(hThread);

#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		if(clsAPIImport::pIsWow64Process)
			clsAPIImport::pIsWow64Process(hProc,&bIsWOW64);

		if(bIsWOW64)
		{
			WOW64_CONTEXT pContext;
			pContext.ContextFlags = CONTEXT_ALL;
			clsAPIImport::pWow64GetThreadContext(hThread,&pContext);

			newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&pContext,false);
		}
		else
		{
			CONTEXT pContext;
			pContext.ContextFlags = CONTEXT_ALL;
			GetThreadContext(hThread,&pContext);

			newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&pContext,true);
		}
#else
		{
			CONTEXT pContext;
			pContext.ContextFlags = CONTEXT_ALL;
			GetThreadContext(hThread,&pContext);

			newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&pContext,false);
		}
#endif		
		ResumeThread(hThread);
		
		newRegEditWindow->show();
		CloseHandle(hThread);
		CloseHandle(hProc);
	}
	else if(QString().compare(pAction->text(),"Highest") == 0)
		SetThreadPriorityByTid(tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16),THREAD_PRIORITY_HIGHEST);
	else if(QString().compare(pAction->text(),"Above Normal") == 0)
		SetThreadPriorityByTid(tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16),THREAD_PRIORITY_ABOVE_NORMAL);
	else if(QString().compare(pAction->text(),"Normal") == 0)
		SetThreadPriorityByTid(tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16),THREAD_PRIORITY_NORMAL);
	else if(QString().compare(pAction->text(),"Below Normal") == 0)
		SetThreadPriorityByTid(tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16),THREAD_PRIORITY_BELOW_NORMAL);
	else if(QString().compare(pAction->text(),"Lowest") == 0)
		SetThreadPriorityByTid(tblTIDs->item(_iSelectedRow,1)->text().toULongLong(0,16),THREAD_PRIORITY_LOWEST);
}

int qtDLGDetailInfo::OnThread(DWORD dwPID,DWORD dwTID,quint64 dwEP,bool bSuspended,DWORD dwExitCode,bool bFound)
{
	if(!bFound)
	{
		tblTIDs->insertRow(tblTIDs->rowCount());
		
		tblTIDs->setItem(tblTIDs->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(dwPID,8,16,QChar('0'))));
		
		tblTIDs->setItem(tblTIDs->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(dwTID,8,16,QChar('0'))));
	
		tblTIDs->setItem(tblTIDs->rowCount() - 1,2,
			new QTableWidgetItem(QString("%1").arg(dwEP,16,16,QChar('0'))));

		tblTIDs->setItem(tblTIDs->rowCount() - 1,4,
			new QTableWidgetItem("Running"));

		tblTIDs->scrollToBottom();
	} 
	else
	{
		for(int i = 0; i < tblTIDs->rowCount();i++)
			if(QString().compare(tblTIDs->item(i,0)->text(),QString("%1").arg(dwPID,8,16,QChar('0'))) == 0 &&
				QString().compare(tblTIDs->item(i,1)->text(),QString("%1").arg(dwTID,8,16,QChar('0'))) == 0)
			{
				tblTIDs->setItem(i,4,new QTableWidgetItem(QString("Terminated")));
				tblTIDs->setItem(i,3,new QTableWidgetItem(QString("%1").arg(dwExitCode,8,16,QChar('0'))));
			}
	}

	return 0;
}

int qtDLGDetailInfo::OnPID(DWORD dwPID,wstring sFile,DWORD dwExitCode,quint64 dwEP,bool bFound)
{

	if(!bFound)
	{
		tblPIDs->insertRow(tblPIDs->rowCount());
		
		tblPIDs->setItem(tblPIDs->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(dwPID,8,16,QChar('0'))));
		
		tblPIDs->setItem(tblPIDs->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(dwEP,16,16,QChar('0'))));

		tblPIDs->setItem(tblPIDs->rowCount() - 1,3,
			new QTableWidgetItem(QString::fromStdWString(sFile)));

		tblPIDs->scrollToBottom();
	}
	else
	{
		for(int i = 0; i < tblPIDs->rowCount();i++)
			if(QString().compare(tblPIDs->item(i,0)->text(),QString("%1").arg(dwPID,8,16,QChar('0'))) == 0)
				tblPIDs->setItem(i,2, new QTableWidgetItem(QString("%1").arg(dwExitCode,8,16,QChar('0'))));
	}
	return 0;
}

int qtDLGDetailInfo::OnException(wstring sFuncName,wstring sModName,quint64 dwOffset,quint64 dwExceptionCode,DWORD dwPID,DWORD dwTID)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();
	myMainWindow->lExceptionCount++;

	tblExceptions->insertRow(tblExceptions->rowCount());
		
	tblExceptions->setItem(tblExceptions->rowCount() - 1,0,
		new QTableWidgetItem(QString("%1").arg(dwOffset,16,16,QChar('0'))));
		
	tblExceptions->setItem(tblExceptions->rowCount() - 1,1,
		new QTableWidgetItem(QString("%1").arg(dwExceptionCode,16,16,QChar('0'))));

	tblExceptions->setItem(tblExceptions->rowCount() - 1,2,
		new QTableWidgetItem(QString().sprintf("%08X / %08X",dwPID,dwTID)));

	if(sFuncName.length() > 0 )
		tblExceptions->setItem(tblExceptions->rowCount() - 1,3,
			new QTableWidgetItem(QString::fromStdWString(sModName).append(".").append(QString::fromStdWString(sFuncName))));

	tblExceptions->scrollToBottom();

	return 0;
}

int qtDLGDetailInfo::OnDbgString(wstring sMessage,DWORD dwPID)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	myMainWindow->dlgDbgStr->tblDebugStrings->insertRow(myMainWindow->dlgDbgStr->tblDebugStrings->rowCount());
		
	myMainWindow->dlgDbgStr->tblDebugStrings->setItem(myMainWindow->dlgDbgStr->tblDebugStrings->rowCount() - 1,0,
		new QTableWidgetItem(QString().sprintf("%08X",dwPID)));
		
	myMainWindow->dlgDbgStr->tblDebugStrings->setItem(myMainWindow->dlgDbgStr->tblDebugStrings->rowCount() - 1,1,
		new QTableWidgetItem(QString::fromStdWString(sMessage)));

	myMainWindow->dlgDbgStr->tblDebugStrings->scrollToBottom();

	return 0;
}

int qtDLGDetailInfo::OnDll(wstring sDLLPath,DWORD dwPID,quint64 dwEP,bool bLoaded)
{
	if(bLoaded)
	{
		tblModules->insertRow(tblModules->rowCount());
		
		tblModules->setItem(tblModules->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(dwPID,8,16,QChar('0'))));
		
		tblModules->setItem(tblModules->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(dwEP,16,16,QChar('0'))));

		tblModules->setItem(tblModules->rowCount() - 1,2,
			new QTableWidgetItem("Loaded"));

		tblModules->setItem(tblModules->rowCount() - 1,3,
			new QTableWidgetItem(QString::fromStdWString(sDLLPath)));

		tblModules->scrollToBottom();
	}
	else
	{
		for(int i = 0; i < tblModules->rowCount();i++)
			if(QString().compare(tblModules->item(i,0)->text(),QString("%1").arg(dwPID,8,16,QChar('0'))) == 0 &&
				QString().compare(tblModules->item(i,1)->text(),QString("%1").arg(dwEP,16,16,QChar('0'))) == 0)
				tblModules->setItem(i,2, new QTableWidgetItem("Unloaded"));
	}
	return 0;
}

bool qtDLGDetailInfo::SetThreadPriorityByTid(DWORD ThreadID, int threadPrio)
{
	HANDLE hThread = OpenThread(THREAD_SET_INFORMATION,false,ThreadID);
	if(hThread == INVALID_HANDLE_VALUE) return false;

	bool bSuccess = false;
	if(!(bSuccess = SetThreadPriority(hThread,threadPrio)))
		MessageBoxW(NULL,L"ERROR, could not set the thread priority",L"Nanomite",MB_OK);
	CloseHandle(hThread);
	return bSuccess;
}

bool qtDLGDetailInfo::SetProcessPriorityByPid(DWORD PID, int processPrio)
{
	HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION,false,PID);
	if(hProc == INVALID_HANDLE_VALUE) return false;

	bool bSuccess = false;
	if(!(bSuccess = SetPriorityClass(hProc,processPrio)))
		MessageBoxW(NULL,L"ERROR, could not set the process priority",L"Nanomite",MB_OK);
	CloseHandle(hProc);
	return bSuccess;
}

int qtDLGDetailInfo::GetThreadPriorityByTid(DWORD ThreadID)
{
	HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION,false,ThreadID);
	if(hThread == INVALID_HANDLE_VALUE) return THREAD_PRIORITY_ERROR_RETURN;
	int iPriority = GetThreadPriority(hThread);
	CloseHandle(hThread);
	return iPriority;
}

int qtDLGDetailInfo::GetProcessPriorityByPid(DWORD PID)
{
	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION,false,PID);
	if(hProc == INVALID_HANDLE_VALUE) return 0;
	int iPriority = GetPriorityClass(hProc);
	CloseHandle(hProc);
	return iPriority;
}