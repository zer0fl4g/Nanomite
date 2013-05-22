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

#include "Psapi.h"

qtDLGWindowView* qtDLGWindowView::pThis = NULL;

qtDLGWindowView::qtDLGWindowView(QWidget *parent, Qt::WFlags flags,qint32 iPID)
	: QWidget(parent, flags)
{
	setupUi(this);
	pThis = this;
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);

	_iPID = iPID;

	// Init List
	tblWindowView->horizontalHeader()->resizeSection(0,75);
	tblWindowView->horizontalHeader()->resizeSection(1,300);
	tblWindowView->horizontalHeader()->resizeSection(2,75);
	tblWindowView->horizontalHeader()->resizeSection(3,135);

	// Display
	myMainWindow = qtDLGNanomite::GetInstance();

	_iForEntry = 0;
	_iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(size_t i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == _iPID)
			_iForEntry = i; _iForEnd = i + 1;
	}

	connect(new QShortcut(QKeySequence("F5"),this),SIGNAL(activated()),this,SLOT(EnumWindow()));

	EnumWindow();
}

qtDLGWindowView::~qtDLGWindowView()
{

}

void qtDLGWindowView::EnumWindow()
{
	tblWindowView->setRowCount(0);

	for(size_t i = _iForEntry; i < _iForEnd;i++)
	{
		EnumWindows((WNDENUMPROC)EnumWindowCallBack,(LPARAM)myMainWindow->coreDebugger->PIDs[i].dwPID);
	}
}

bool CALLBACK qtDLGWindowView::EnumWindowCallBack(HWND hWnd,LPARAM lParam)
{
	DWORD dwHwPID = NULL;
	GetWindowThreadProcessId(hWnd,&dwHwPID);
	int iPid = (int)lParam;
	
	if(dwHwPID == iPid)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,false,dwHwPID);
		
		pThis->tblWindowView->insertRow(pThis->tblWindowView->rowCount());
		PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
		// PID
		pThis->tblWindowView->setItem(pThis->tblWindowView->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(dwHwPID,8,16,QChar('0'))));

		// GetWindowName
		GetWindowText(hWnd,sTemp,MAX_PATH);
		pThis->tblWindowView->setItem(pThis->tblWindowView->rowCount() - 1,1,
			new QTableWidgetItem(QString::fromStdWString(sTemp)));

		// IsVisible
		pThis->tblWindowView->setItem(pThis->tblWindowView->rowCount() - 1,2,
			new QTableWidgetItem(QString().sprintf("%s",(IsWindowVisible(hWnd) ? "TRUE" : "FALSE"))));

		// hWnd
		pThis->tblWindowView->setItem(pThis->tblWindowView->rowCount() - 1,3,
			new QTableWidgetItem(QString("%1").arg((int)hWnd,8,16,QChar('0'))));

		// WndProc
		//LONG_PTR wndproc = GetWindowLongPtr(hWnd,GWLP_ID);
		//qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,4,
		//	new QTableWidgetItem(QString("%1").arg(wndproc,8,16,QChar('0'))));

		// GetModuleName
		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
		if(GetModuleFileNameEx(hProcess,NULL,sTemp,MAX_PATH) > 0)
			pThis->tblWindowView->setItem(pThis->tblWindowView->rowCount() - 1,5,
				new QTableWidgetItem(QString::fromStdWString(sTemp)));
		else
			pThis->tblWindowView->setItem(pThis->tblWindowView->rowCount() - 1,5,
				new QTableWidgetItem(""));

		CloseHandle(hProcess);
		free(sTemp);
	}
	return true;
}
