#include "clsCallbacks.h"
#include "qtDLGNanomite.h"
#include "qtDLGWindowView.h"

using namespace std;

clsCallbacks::clsCallbacks() {}

clsCallbacks::~clsCallbacks(){}

int clsCallbacks::OnLog(tm *timeInfo,wstring sLog)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();
	myMainWindow->tblLogBox->insertRow(myMainWindow->tblLogBox->rowCount());
	
	myMainWindow->tblLogBox->setItem(myMainWindow->tblLogBox->rowCount() - 1,0,
		new QTableWidgetItem(QString().sprintf("[%i:%i:%i]",timeInfo->tm_hour,timeInfo->tm_min,timeInfo->tm_sec)));
	
	myMainWindow->tblLogBox->setItem(myMainWindow->tblLogBox->rowCount() - 1,1,
		new QTableWidgetItem(QString::fromStdWString(sLog)));
	
	myMainWindow->tblLogBox->scrollToBottom();
	return 0;
}

int clsCallbacks::OnThread(DWORD dwPID,DWORD dwTID,DWORD64 dwEP,bool bSuspended,DWORD dwExitCode,bool bFound)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	if(!bFound)
	{
		myMainWindow->dlgDetInfo->tblTIDs->insertRow(myMainWindow->dlgDetInfo->tblTIDs->rowCount());
		
		myMainWindow->dlgDetInfo->tblTIDs->setItem(myMainWindow->dlgDetInfo->tblTIDs->rowCount() - 1,0,
			new QTableWidgetItem(QString().sprintf("%08X",dwPID)));
		
		myMainWindow->dlgDetInfo->tblTIDs->setItem(myMainWindow->dlgDetInfo->tblTIDs->rowCount() - 1,1,
			new QTableWidgetItem(QString().sprintf("%08X",dwTID)));
	
		myMainWindow->dlgDetInfo->tblTIDs->setItem(myMainWindow->dlgDetInfo->tblTIDs->rowCount() - 1,2,
			new QTableWidgetItem(QString().sprintf("%016X",dwEP)));

		myMainWindow->dlgDetInfo->tblTIDs->setItem(myMainWindow->dlgDetInfo->tblTIDs->rowCount() - 1,4,
			new QTableWidgetItem("Running"));

		myMainWindow->dlgDetInfo->tblTIDs->scrollToBottom();
	}
	else
	{
		for(int i = 0; i < myMainWindow->dlgDetInfo->tblTIDs->rowCount();i++)
			if(QString().compare(myMainWindow->dlgDetInfo->tblTIDs->item(i,0)->text(),QString().sprintf("%08X",dwPID)) == 0 &&
				QString().compare(myMainWindow->dlgDetInfo->tblTIDs->item(i,1)->text(),QString().sprintf("%08X",dwTID)) == 0)
			{
				myMainWindow->dlgDetInfo->tblTIDs->item(i,4)->setText("Terminated");
				myMainWindow->dlgDetInfo->tblTIDs->item(i,2)->setText(QString().sprintf("%08X",dwExitCode));
			}
	}

	return 0;
}

int clsCallbacks::OnPID(DWORD dwPID,wstring sFile,DWORD dwExitCode,DWORD64 dwEP,bool bFound)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	if(!bFound)
	{
		myMainWindow->dlgDetInfo->tblPIDs->insertRow(myMainWindow->dlgDetInfo->tblPIDs->rowCount());
		
		myMainWindow->dlgDetInfo->tblPIDs->setItem(myMainWindow->dlgDetInfo->tblPIDs->rowCount() - 1,0,
			new QTableWidgetItem(QString().sprintf("%08X",dwPID)));
		
		myMainWindow->dlgDetInfo->tblPIDs->setItem(myMainWindow->dlgDetInfo->tblPIDs->rowCount() - 1,1,
			new QTableWidgetItem(QString().sprintf("%016X",dwEP)));

		myMainWindow->dlgDetInfo->tblPIDs->setItem(myMainWindow->dlgDetInfo->tblPIDs->rowCount() - 1,3,
			new QTableWidgetItem(QString::fromStdWString(sFile)));

		myMainWindow->dlgDetInfo->tblPIDs->scrollToBottom();
	}
	else
	{
		for(int i = 0; i < myMainWindow->dlgDetInfo->tblPIDs->rowCount();i++)
			if(QString().compare(myMainWindow->dlgDetInfo->tblPIDs->item(i,0)->text(),QString().sprintf("%08X",dwPID)) == 0)
				myMainWindow->dlgDetInfo->tblPIDs->setItem(i,2, new QTableWidgetItem(QString().sprintf("%08X",dwExitCode)));
	}
	return 0;
}

int clsCallbacks::OnException(wstring sFuncName,wstring sModName,DWORD64 dwOffset,DWORD64 dwExceptionCode,DWORD dwPID,DWORD dwTID)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();
	myMainWindow->lExceptionCount++;

	myMainWindow->dlgDetInfo->tblExceptions->insertRow(myMainWindow->dlgDetInfo->tblExceptions->rowCount());
		
	myMainWindow->dlgDetInfo->tblExceptions->setItem(myMainWindow->dlgDetInfo->tblExceptions->rowCount() - 1,0,
		new QTableWidgetItem(QString().sprintf("%016X",dwOffset)));
		
	myMainWindow->dlgDetInfo->tblExceptions->setItem(myMainWindow->dlgDetInfo->tblExceptions->rowCount() - 1,1,
		new QTableWidgetItem(QString().sprintf("%016X",dwExceptionCode)));

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

	myMainWindow->dlgDetInfo->tblExceptions->scrollToBottom();

	return 0;
}

int clsCallbacks::OnDll(wstring sDLLPath,DWORD dwPID,DWORD64 dwEP,bool bLoaded)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	if(bLoaded)
	{
		myMainWindow->dlgDetInfo->tblModules->insertRow(myMainWindow->dlgDetInfo->tblModules->rowCount());
		
		myMainWindow->dlgDetInfo->tblModules->setItem(myMainWindow->dlgDetInfo->tblModules->rowCount() - 1,0,
			new QTableWidgetItem(QString().sprintf("%08X",dwPID)));
		
		myMainWindow->dlgDetInfo->tblModules->setItem(myMainWindow->dlgDetInfo->tblModules->rowCount() - 1,1,
			new QTableWidgetItem(QString().sprintf("%016X",dwEP)));

		myMainWindow->dlgDetInfo->tblModules->setItem(myMainWindow->dlgDetInfo->tblModules->rowCount() - 1,2,
			new QTableWidgetItem("Loaded"));

		myMainWindow->dlgDetInfo->tblModules->setItem(myMainWindow->dlgDetInfo->tblModules->rowCount() - 1,3,
			new QTableWidgetItem(QString::fromStdWString(sDLLPath)));

		myMainWindow->dlgDetInfo->tblModules->scrollToBottom();
	}
	else
	{
		for(int i = 0; i < myMainWindow->dlgDetInfo->tblModules->rowCount();i++)
			if(QString().compare(myMainWindow->dlgDetInfo->tblModules->item(i,0)->text(),QString().sprintf("%08X",dwPID)) == 0 &&
				QString().compare(myMainWindow->dlgDetInfo->tblModules->item(i,1)->text(),QString().sprintf("%016X",dwEP)) == 0)
				myMainWindow->dlgDetInfo->tblModules->setItem(i,2, new QTableWidgetItem("Unloaded"));
	}
	return 0;
}

int clsCallbacks::OnCallStack(DWORD64 dwStackAddr,
						 DWORD64 dwReturnTo,wstring sReturnToFunc,wstring sModuleName,
						 DWORD64 dwEIP,wstring sFuncName,wstring sFuncModule,
						 wstring sSourceFilePath,int iSourceLineNum)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();
	myMainWindow->tblCallstack->insertRow(myMainWindow->tblCallstack->rowCount());

	// Stack Address
	QString strTemp = QString().sprintf("0x%016X",dwStackAddr);
	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,0,new QTableWidgetItem(strTemp));

	// Func Addr
	strTemp = QString().sprintf("0x%016X",dwEIP);
	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,1,new QTableWidgetItem(strTemp));

	// <mod.func>
	if(sFuncName.length() > 0)
		strTemp = QString::fromStdWString(sFuncModule).append(".").append(QString::fromStdWString(sFuncName));
	else
		strTemp = QString::fromStdWString(sFuncModule).append(".").append(QString().sprintf("0x%016X",dwStackAddr));

	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,2,new QTableWidgetItem(strTemp));

	// Return To
	strTemp = QString().sprintf("0x%016X",dwReturnTo);
	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,3,new QTableWidgetItem(strTemp));

	// Return To <mod.func>
		if(sFuncName.length() > 0)
		strTemp = QString::fromStdWString(sModuleName).append(".").append(QString::fromStdWString(sReturnToFunc));
	else
		strTemp = QString::fromStdWString(sModuleName).append(".").append(QString().sprintf("0x%016X",dwStackAddr));

	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,4,new QTableWidgetItem(strTemp));

	// Source Line
	strTemp = QString().sprintf("%d",iSourceLineNum);
	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,5,new QTableWidgetItem(strTemp));
	
	// Source File
	strTemp = QString::fromStdWString(sSourceFilePath);
	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,6,new QTableWidgetItem(strTemp));

	return 0;
}

bool CALLBACK clsCallbacks::EnumWindowCallBack(HWND hWnd,LPARAM lParam)
{
	DWORD dwHwPID = NULL;
	GetWindowThreadProcessId(hWnd,&dwHwPID);
	int iPid = (int)lParam;

	if(dwHwPID == iPid)
	{
		qtDLGWindowView::pThis->tblWindowView->insertRow(qtDLGWindowView::pThis->tblWindowView->rowCount());
		PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
		// PID
		qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,0,
			new QTableWidgetItem(QString().sprintf("%08X",dwHwPID)));

		// GetWindowName
		GetWindowText(hWnd,sTemp,MAX_PATH);
		qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,1,
			new QTableWidgetItem(QString::fromStdWString(sTemp)));

		// IsVisible
		qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,2,
			new QTableWidgetItem(QString().sprintf("%s",(IsWindowVisible(hWnd) ? "TRUE" : "FALSE"))));

		// hWnd
		qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,3,
			new QTableWidgetItem(QString().sprintf("%08X",hWnd)));

		// GetModuleName
		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
		GetWindowModuleFileName(hWnd,sTemp,MAX_PATH);
		qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,4,
			new QTableWidgetItem(QString::fromStdWString(sTemp)));

		free(sTemp);
	}
	return true;
}