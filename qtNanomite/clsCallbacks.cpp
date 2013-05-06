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
#include "clsCallbacks.h"
#include "clsMemManager.h"

#include "qtDLGNanomite.h"
#include "qtDLGWindowView.h"

#include "Psapi.h"

using namespace std;

clsCallbacks::clsCallbacks() {}

clsCallbacks::~clsCallbacks(){}

int clsCallbacks::OnThread(DWORD dwPID,DWORD dwTID,quint64 dwEP,bool bSuspended,DWORD dwExitCode,bool bFound)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	if(!bFound)
	{
		myMainWindow->dlgDetInfo->tblTIDs->insertRow(myMainWindow->dlgDetInfo->tblTIDs->rowCount());
		
		myMainWindow->dlgDetInfo->tblTIDs->setItem(myMainWindow->dlgDetInfo->tblTIDs->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(dwPID,8,16,QChar('0'))));
		
		myMainWindow->dlgDetInfo->tblTIDs->setItem(myMainWindow->dlgDetInfo->tblTIDs->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(dwTID,8,16,QChar('0'))));
	
		myMainWindow->dlgDetInfo->tblTIDs->setItem(myMainWindow->dlgDetInfo->tblTIDs->rowCount() - 1,2,
			new QTableWidgetItem(QString("%1").arg(dwEP,16,16,QChar('0'))));

		myMainWindow->dlgDetInfo->tblTIDs->setItem(myMainWindow->dlgDetInfo->tblTIDs->rowCount() - 1,4,
			new QTableWidgetItem("Running"));

		myMainWindow->dlgDetInfo->tblTIDs->scrollToBottom();
	} 
	else
	{
		for(int i = 0; i < myMainWindow->dlgDetInfo->tblTIDs->rowCount();i++)
			if(QString().compare(myMainWindow->dlgDetInfo->tblTIDs->item(i,0)->text(),QString("%1").arg(dwPID,8,16,QChar('0'))) == 0 &&
				QString().compare(myMainWindow->dlgDetInfo->tblTIDs->item(i,1)->text(),QString("%1").arg(dwTID,8,16,QChar('0'))) == 0)
			{
				myMainWindow->dlgDetInfo->tblTIDs->setItem(i,4,new QTableWidgetItem(QString("Terminated")));
				myMainWindow->dlgDetInfo->tblTIDs->setItem(i,3,new QTableWidgetItem(QString("%1").arg(dwExitCode,8,16,QChar('0'))));
			}
	}

	return 0;
}

int clsCallbacks::OnPID(DWORD dwPID,wstring sFile,DWORD dwExitCode,quint64 dwEP,bool bFound)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	if(!bFound)
	{
		myMainWindow->dlgDetInfo->tblPIDs->insertRow(myMainWindow->dlgDetInfo->tblPIDs->rowCount());
		
		myMainWindow->dlgDetInfo->tblPIDs->setItem(myMainWindow->dlgDetInfo->tblPIDs->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(dwPID,8,16,QChar('0'))));
		
		myMainWindow->dlgDetInfo->tblPIDs->setItem(myMainWindow->dlgDetInfo->tblPIDs->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(dwEP,16,16,QChar('0'))));

		myMainWindow->dlgDetInfo->tblPIDs->setItem(myMainWindow->dlgDetInfo->tblPIDs->rowCount() - 1,3,
			new QTableWidgetItem(QString::fromStdWString(sFile)));

		myMainWindow->dlgDetInfo->tblPIDs->scrollToBottom();
	}
	else
	{
		for(int i = 0; i < myMainWindow->dlgDetInfo->tblPIDs->rowCount();i++)
			if(QString().compare(myMainWindow->dlgDetInfo->tblPIDs->item(i,0)->text(),QString("%1").arg(dwPID,8,16,QChar('0'))) == 0)
				myMainWindow->dlgDetInfo->tblPIDs->setItem(i,2, new QTableWidgetItem(QString("%1").arg(dwExitCode,8,16,QChar('0'))));
	}
	return 0;
}

int clsCallbacks::OnException(wstring sFuncName,wstring sModName,quint64 dwOffset,quint64 dwExceptionCode,DWORD dwPID,DWORD dwTID)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();
	myMainWindow->lExceptionCount++;

	myMainWindow->dlgDetInfo->tblExceptions->insertRow(myMainWindow->dlgDetInfo->tblExceptions->rowCount());
		
	myMainWindow->dlgDetInfo->tblExceptions->setItem(myMainWindow->dlgDetInfo->tblExceptions->rowCount() - 1,0,
		new QTableWidgetItem(QString("%1").arg(dwOffset,16,16,QChar('0'))));
		
	myMainWindow->dlgDetInfo->tblExceptions->setItem(myMainWindow->dlgDetInfo->tblExceptions->rowCount() - 1,1,
		new QTableWidgetItem(QString("%1").arg(dwExceptionCode,16,16,QChar('0'))));

	myMainWindow->dlgDetInfo->tblExceptions->setItem(myMainWindow->dlgDetInfo->tblExceptions->rowCount() - 1,2,
		new QTableWidgetItem(QString().sprintf("%08X / %08X",dwPID,dwTID)));

	if(sFuncName.length() > 0 )
		myMainWindow->dlgDetInfo->tblExceptions->setItem(myMainWindow->dlgDetInfo->tblExceptions->rowCount() - 1,3,
			new QTableWidgetItem(QString::fromStdWString(sModName).append(".").append(QString::fromStdWString(sFuncName))));

	myMainWindow->dlgDetInfo->tblExceptions->scrollToBottom();

	return 0;
}

int clsCallbacks::OnDbgString(wstring sMessage,DWORD dwPID)
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

int clsCallbacks::OnDll(wstring sDLLPath,DWORD dwPID,quint64 dwEP,bool bLoaded)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	if(bLoaded)
	{
		myMainWindow->dlgDetInfo->tblModules->insertRow(myMainWindow->dlgDetInfo->tblModules->rowCount());
		
		myMainWindow->dlgDetInfo->tblModules->setItem(myMainWindow->dlgDetInfo->tblModules->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(dwPID,8,16,QChar('0'))));
		
		myMainWindow->dlgDetInfo->tblModules->setItem(myMainWindow->dlgDetInfo->tblModules->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(dwEP,16,16,QChar('0'))));

		myMainWindow->dlgDetInfo->tblModules->setItem(myMainWindow->dlgDetInfo->tblModules->rowCount() - 1,2,
			new QTableWidgetItem("Loaded"));

		myMainWindow->dlgDetInfo->tblModules->setItem(myMainWindow->dlgDetInfo->tblModules->rowCount() - 1,3,
			new QTableWidgetItem(QString::fromStdWString(sDLLPath)));

		myMainWindow->dlgDetInfo->tblModules->scrollToBottom();
	}
	else
	{
		for(int i = 0; i < myMainWindow->dlgDetInfo->tblModules->rowCount();i++)
			if(QString().compare(myMainWindow->dlgDetInfo->tblModules->item(i,0)->text(),QString("%1").arg(dwPID,8,16,QChar('0'))) == 0 &&
				QString().compare(myMainWindow->dlgDetInfo->tblModules->item(i,1)->text(),QString("%1").arg(dwEP,16,16,QChar('0'))) == 0)
				myMainWindow->dlgDetInfo->tblModules->setItem(i,2, new QTableWidgetItem("Unloaded"));
	}
	return 0;
}

bool CALLBACK clsCallbacks::EnumWindowCallBack(HWND hWnd,LPARAM lParam)
{
	DWORD dwHwPID = NULL;
	GetWindowThreadProcessId(hWnd,&dwHwPID);
	int iPid = (int)lParam;
	
	if(dwHwPID == iPid)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,false,dwHwPID);
		
		qtDLGWindowView::pThis->tblWindowView->insertRow(qtDLGWindowView::pThis->tblWindowView->rowCount());
		PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
		// PID
		qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(dwHwPID,8,16,QChar('0'))));

		// GetWindowName
		GetWindowText(hWnd,sTemp,MAX_PATH);
		qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,1,
			new QTableWidgetItem(QString::fromStdWString(sTemp)));

		// IsVisible
		qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,2,
			new QTableWidgetItem(QString().sprintf("%s",(IsWindowVisible(hWnd) ? "TRUE" : "FALSE"))));

		// hWnd
		qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,3,
			new QTableWidgetItem(QString("%1").arg((int)hWnd,8,16,QChar('0'))));

		// WndProc
		//LONG_PTR wndproc = GetWindowLongPtr(hWnd,GWLP_ID);
		//qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,4,
		//	new QTableWidgetItem(QString("%1").arg(wndproc,8,16,QChar('0'))));

		// GetModuleName
		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
		if(GetModuleFileNameEx(hProcess,NULL,sTemp,MAX_PATH) > 0)
			qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,5,
				new QTableWidgetItem(QString::fromStdWString(sTemp)));
		else
			qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,5,
				new QTableWidgetItem(""));

		CloseHandle(hProcess);
		free(sTemp);
	}
	return true;
}