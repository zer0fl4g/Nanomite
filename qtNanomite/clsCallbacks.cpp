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
			if(QString().compare(myMainWindow->dlgDetInfo->tblTIDs->item(i,0)->text(),QString().sprintf("%08X",dwPID)) == 0 &&
				QString().compare(myMainWindow->dlgDetInfo->tblTIDs->item(i,1)->text(),QString().sprintf("%08X",dwTID)) == 0)
			{
				myMainWindow->dlgDetInfo->tblTIDs->item(i,4)->setText("Terminated");
				myMainWindow->dlgDetInfo->tblTIDs->item(i,2)->setText(QString("%1").arg(dwExitCode,8,16,QChar('0')));
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
			if(QString().compare(myMainWindow->dlgDetInfo->tblPIDs->item(i,0)->text(),QString().sprintf("%08X",dwPID)) == 0)
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

	myMainWindow->dlgDetInfo->tblExceptions->scrollToBottom();

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

int clsCallbacks::OnCallStack(quint64 dwStackAddr,
						 quint64 dwReturnTo,wstring sReturnToFunc,wstring sModuleName,
						 quint64 dwEIP,wstring sFuncName,wstring sFuncModule,
						 wstring sSourceFilePath,int iSourceLineNum)
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();
	myMainWindow->tblCallstack->insertRow(myMainWindow->tblCallstack->rowCount());

	// Stack Address
	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,0,
		new QTableWidgetItem(QString("%1").arg(dwStackAddr,16,16,QChar('0'))));

	// Func Addr
	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,1,
		new QTableWidgetItem(QString("%1").arg(dwEIP,16,16,QChar('0'))));

	// <mod.func>
	if(sFuncName.length() > 0)
		myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,2,
			new QTableWidgetItem(QString::fromStdWString(sFuncModule).append(".").append(QString::fromStdWString(sFuncName))));
	else
		myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,2,
			new QTableWidgetItem(QString::fromStdWString(sFuncModule).append(".").append(QString("%1").arg(dwStackAddr,16,16,QChar('0')))));

	// Return To
	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,3,
		new QTableWidgetItem(QString("%1").arg(dwReturnTo,16,16,QChar('0'))));

	// Return To <mod.func>
	if(sFuncName.length() > 0)
		myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,4,
			new QTableWidgetItem(QString::fromStdWString(sModuleName).append(".").append(QString::fromStdWString(sReturnToFunc))));
	else
		myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,4,
			new QTableWidgetItem(QString::fromStdWString(sModuleName).append(".").append(QString("%1").arg(dwStackAddr,16,16,QChar('0')))));

	// Source Line
	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,5,
		new QTableWidgetItem(QString().sprintf("%d",iSourceLineNum)));
	
	// Source File
	myMainWindow->tblCallstack->setItem(myMainWindow->tblCallstack->rowCount() - 1,6,
		new QTableWidgetItem(QString::fromStdWString(sSourceFilePath)));

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

		// GetModuleName
		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
		GetWindowModuleFileName(hWnd,sTemp,MAX_PATH);
		qtDLGWindowView::pThis->tblWindowView->setItem(qtDLGWindowView::pThis->tblWindowView->rowCount() - 1,4,
			new QTableWidgetItem(QString::fromStdWString(sTemp)));

		free(sTemp);
	}
	return true;
}