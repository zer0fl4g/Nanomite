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

#include "qtDLGProcessPrivilege.h"

#include "clsMemManager.h"

#include <QClipboard>

qtDLGProcessPrivilege::qtDLGProcessPrivilege(QWidget *parent, Qt::WFlags flags, qint32 processID)
	: QWidget(parent, flags),
	m_processID(processID),
	m_processLoopEntry(0)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);

	connect(tblProcPriv,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenuRequested(QPoint)));
	connect(new QShortcut(QKeySequence("F5"),this),SIGNAL(activated()),this,SLOT(DisplayPrivileges()));
	connect(new QShortcut(Qt::Key_Escape,this),SIGNAL(activated()),this,SLOT(close()));

	// Init List
	tblProcPriv->horizontalHeader()->resizeSection(0,75);
	tblProcPriv->horizontalHeader()->resizeSection(1,250);
	tblProcPriv->horizontalHeader()->resizeSection(2,135);
	tblProcPriv->horizontalHeader()->setFixedHeight(21);

	// Display
	m_pMainWindow = qtDLGNanomite::GetInstance();

	m_processLoopEnd = m_pMainWindow->coreDebugger->PIDs.size();
	for(size_t i = 0; i < m_pMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(m_pMainWindow->coreDebugger->PIDs[i].dwPID == m_processID)
			m_processLoopEntry = i; m_processLoopEnd = i + 1;
	}

	DisplayPrivileges();
}

qtDLGProcessPrivilege::~qtDLGProcessPrivilege()
{

}

void qtDLGProcessPrivilege::DisplayPrivileges()
{
	tblProcPriv->setRowCount(0);

	for(size_t i = m_processLoopEntry; i < m_processLoopEnd;i++)
	{
		HANDLE	hProcess	= m_pMainWindow->coreDebugger->PIDs[i].hProc,
				hToken		= NULL;
		
		if(!OpenProcessToken(hProcess, TOKEN_READ, &hToken))
		{
			continue;
		}

		DWORD bufferSize = NULL;
		PTOKEN_PRIVILEGES pTokenBuffer = NULL;
		if(!GetTokenInformation(hToken, TokenPrivileges, (LPVOID)pTokenBuffer, NULL, &bufferSize))
		{
			if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			{
				CloseHandle(hToken);
				continue;
			}

			pTokenBuffer = (PTOKEN_PRIVILEGES)clsMemManager::CAlloc(bufferSize);
		}

		if(!GetTokenInformation(hToken, TokenPrivileges, (LPVOID)pTokenBuffer, bufferSize, &bufferSize))
		{
			clsMemManager::CFree(pTokenBuffer);
			CloseHandle(hToken);
			continue;
		}

		PTCHAR pPrivilegeName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
		DWORD nameSize = NULL;
		int	PID	= m_pMainWindow->coreDebugger->PIDs[i].dwPID;
		
		for(DWORD i = 0; i < pTokenBuffer->PrivilegeCount; i++)
		{
			memset(pPrivilegeName, 0, MAX_PATH * sizeof(TCHAR));
			nameSize = MAX_PATH;

			if(LookupPrivilegeName(NULL, &pTokenBuffer->Privileges[i].Luid,pPrivilegeName, &nameSize) == 0)
			{
				continue;
			}

			tblProcPriv->insertRow(tblProcPriv->rowCount());
			tblProcPriv->setItem(tblProcPriv->rowCount() - 1, 0,
				new QTableWidgetItem(QString("%1").arg(PID,8,16,QChar('0'))));

			tblProcPriv->setItem(tblProcPriv->rowCount() - 1, 1,
				new QTableWidgetItem(QString::fromWCharArray(pPrivilegeName,nameSize)));

			switch(pTokenBuffer->Privileges[i].Attributes)
			{
			case SE_PRIVILEGE_ENABLED:
				tblProcPriv->setItem(tblProcPriv->rowCount() - 1, 2,
					new QTableWidgetItem("Enabled"));
				break;

			case SE_PRIVILEGE_ENABLED_BY_DEFAULT:
				tblProcPriv->setItem(tblProcPriv->rowCount() - 1, 2,
					new QTableWidgetItem("Enabled by Default"));
				break;

			case (SE_PRIVILEGE_ENABLED_BY_DEFAULT | SE_PRIVILEGE_ENABLED):
				tblProcPriv->setItem(tblProcPriv->rowCount() - 1, 2,
					new QTableWidgetItem("Enabled by Default"));
				break;

			default:
				tblProcPriv->setItem(tblProcPriv->rowCount() - 1, 2,
					new QTableWidgetItem("Disabled"));
			}
		}

		clsMemManager::CFree(pPrivilegeName);
		clsMemManager::CFree(pTokenBuffer);
		CloseHandle(hToken);
	}
}

void qtDLGProcessPrivilege::OnCustomContextMenuRequested(QPoint qPoint)
{
	QMenu menu;

	m_selectedRow = tblProcPriv->indexAt(qPoint).row();
	if(m_selectedRow < 0) return;

	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Privilege Name",this));
	submenu->addAction(new QAction("State",this));

	menu.addMenu(submenu);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGProcessPrivilege::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3")
			.arg(tblProcPriv->item(m_selectedRow,0)->text())
			.arg(tblProcPriv->item(m_selectedRow,1)->text())
			.arg(tblProcPriv->item(m_selectedRow,2)->text()));
	}
	else if(QString().compare(pAction->text(),"Privilege Name") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblProcPriv->item(m_selectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"State") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblProcPriv->item(m_selectedRow,2)->text());
	}
}