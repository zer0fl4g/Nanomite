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
#include "qtDLGWindowView.h"

#include "clsMemManager.h"

#include "Psapi.h"

qtDLGWindowView* qtDLGWindowView::pThis = NULL;

qtDLGWindowView::qtDLGWindowView(QWidget *parent, Qt::WFlags flags,qint32 processID)
	: QWidget(parent, flags),
	m_processID(processID),
	m_processCountEntry(0)
{
	setupUi(this);
	pThis = this;
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);

	// Init List
	tblWindowView->horizontalHeader()->resizeSection(0,75);
	tblWindowView->horizontalHeader()->resizeSection(1,300);
	tblWindowView->horizontalHeader()->resizeSection(2,75);
	tblWindowView->horizontalHeader()->resizeSection(3,135);
	tblWindowView->horizontalHeader()->setFixedHeight(21);

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

	connect(new QShortcut(QKeySequence("F5"),this),SIGNAL(activated()),this,SLOT(EnumWindow()));
	connect(new QShortcut(Qt::Key_Escape,this),SIGNAL(activated()),this,SLOT(close()));

	EnumWindow();
}

qtDLGWindowView::~qtDLGWindowView()
{

}

void qtDLGWindowView::EnumWindow()
{
	tblWindowView->setRowCount(0);

	for(size_t i = m_processCountEntry; i < m_processCountEnd;i++)
	{
		EnumWindows((WNDENUMPROC)EnumWindowCallBack,(LPARAM)m_pMainWindow->coreDebugger->PIDs[i].dwPID);
	}
}

bool CALLBACK qtDLGWindowView::EnumWindowCallBack(HWND hWnd,LPARAM lParam)
{
	DWORD dwHwPID = NULL;
	GetWindowThreadProcessId(hWnd,&dwHwPID);
	int processID = (int)lParam;
	
	if(dwHwPID == processID)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,false,dwHwPID);
		
		pThis->tblWindowView->insertRow(pThis->tblWindowView->rowCount());
		PTCHAR sTemp = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));

		// PID
		pThis->tblWindowView->setItem(pThis->tblWindowView->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(dwHwPID,8,16,QChar('0'))));

		// GetWindowName
		if(GetWindowText(hWnd,sTemp,MAX_PATH) > 0)
			pThis->tblWindowView->setItem(pThis->tblWindowView->rowCount() - 1,1,
				new QTableWidgetItem(QString::fromStdWString(sTemp)));

		// IsVisible
		pThis->tblWindowView->setItem(pThis->tblWindowView->rowCount() - 1,2,
			new QTableWidgetItem(QString().sprintf("%s",(IsWindowVisible(hWnd) ? "TRUE" : "FALSE"))));

		// hWnd
		pThis->tblWindowView->setItem(pThis->tblWindowView->rowCount() - 1,3,
			new QTableWidgetItem(QString("%1").arg((int)hWnd,8,16,QChar('0'))));

		// GetModuleName
		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
		if(GetModuleFileNameEx(hProcess,NULL,sTemp,MAX_PATH) > 0)
			pThis->tblWindowView->setItem(pThis->tblWindowView->rowCount() - 1,4,
				new QTableWidgetItem(QString::fromStdWString(sTemp)));

		CloseHandle(hProcess);
		clsMemManager::CFree(sTemp);
	}
	return true;
}
