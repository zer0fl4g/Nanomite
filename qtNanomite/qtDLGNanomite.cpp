#include <QShortcut>

#include "qtDLGNanomite.h"
#include "qtDLGAbout.h"
#include "qtDLGAttach.h"
#include "qtDLGOption.h"
#include "qtDLGMemoryView.h"
#include "qtDLGHeapView.h"
#include "qtDLGStringView.h"
#include "qtDLGHandleView.h"
#include "qtDLGWindowView.h"
#include "qtDLGHexView.h"
#include "qtDLGRegEdit.h"

#include "clsCallbacks.h"
#include "clsHelperClass.h"
#include "clsDisassembler.h"
#include "clsAPIImport.h"
#include "clsSymbolAndSyntax.h"

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

using namespace std;

qtDLGNanomite* qtDLGNanomite::qtDLGMyWindow = NULL;

qtDLGNanomite::qtDLGNanomite(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	setupUi(this);
	QWidget *midWid = new QWidget;
	midWid->setLayout(mainLayout);
	setCentralWidget(midWid);
	
	QApplication::setStyle(new QPlastiqueStyle);

	qRegisterMetaType<DWORD>("DWORD");
	qRegisterMetaType<quint64>("quint64");
	qRegisterMetaType<wstring>("wstring");
	qRegisterMetaType<BPStruct>("BPStruct");

	clsAPIImport::LoadFunctions();

	coreDebugger = new clsDebugger;
	coreDisAs = new clsDisassembler;
	clsCallbacks *NanomiteCallbacks = new clsCallbacks;
	dlgDetInfo = new qtDLGDetailInfo(this,Qt::Window);
	dlgDbgStr = new qtDLGDebugStrings(this,Qt::Window);
	dlgBPManager = new qtDLGBreakPointManager(this,Qt::Window);

	qtNanomiteDisAsColor = new qtNanomiteDisAsColorSettings;

	//
	qtNanomiteDisAsColor->colorBP = "red";
	qtNanomiteDisAsColor->colorCall = "green";
	qtNanomiteDisAsColor->colorStack = "dark green";
	qtNanomiteDisAsColor->colorJump = "blue";
	qtNanomiteDisAsColor->colorMove = "grey";
	//
	InitListSizes();

	qtDLGMyWindow = this;
	lExceptionCount = 0;
	clsHelperClass::ReadFromSettingsFile(coreDebugger,qtNanomiteDisAsColor);

	// Callbacks from Debugger Thread to GUI
	connect(coreDebugger,SIGNAL(OnThread(DWORD,DWORD,quint64,bool,DWORD,bool)),
		NanomiteCallbacks,SLOT(OnThread(DWORD,DWORD,quint64,bool,DWORD,bool)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnPID(DWORD,std::wstring,DWORD,quint64,bool)),
		NanomiteCallbacks,SLOT(OnPID(DWORD,std::wstring,DWORD,quint64,bool)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnException(std::wstring,std::wstring,quint64,quint64,DWORD,DWORD)),
		NanomiteCallbacks,SLOT(OnException(std::wstring,std::wstring,quint64,quint64,DWORD,DWORD)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDbgString(std::wstring,DWORD)),
		NanomiteCallbacks,SLOT(OnDbgString(std::wstring,DWORD)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDll(std::wstring,DWORD,quint64,bool)),
		NanomiteCallbacks,SLOT(OnDll(std::wstring,DWORD,quint64,bool)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnLog(tm*,std::wstring)),
		NanomiteCallbacks,SLOT(OnLog(tm*,std::wstring)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnCallStack(quint64,quint64,std::wstring,std::wstring,quint64,std::wstring,std::wstring,std::wstring,int)),
		NanomiteCallbacks,SLOT(OnCallStack(quint64,quint64,std::wstring,std::wstring,quint64,std::wstring,std::wstring,std::wstring,int)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDebuggerBreak()),this,SLOT(OnDebuggerBreak()),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDebuggerTerminated()),this,SLOT(OnDebuggerTerminated()),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnNewBreakpointAdded(BPStruct,int)),dlgBPManager,SLOT(OnUpdate(BPStruct,int)),Qt::QueuedConnection);

	// Callbacks from Disassambler Thread to GUI
	connect(coreDisAs,SIGNAL(DisAsFinished(quint64)),this,SLOT(OnDisplayDisassembly(quint64)),Qt::QueuedConnection);

	// Actions for the MainMenu and Toolbar
	connect(actionFile_OpenNew, SIGNAL(triggered()), this, SLOT(action_FileOpenNewFile()));
	connect(actionFile_AttachTo, SIGNAL(triggered()), this, SLOT(action_FileAttachTo()));
	connect(actionFile_Detach, SIGNAL(triggered()), this, SLOT(action_FileDetach()));
	connect(actionFile_Exit, SIGNAL(triggered()), this, SLOT(action_FileTerminateGUI()));
	connect(actionDebug_Start, SIGNAL(triggered()), this, SLOT(action_DebugStart()));
	connect(actionDebug_Stop, SIGNAL(triggered()), this, SLOT(action_DebugStop()));
	connect(actionDebug_Restart, SIGNAL(triggered()), this, SLOT(action_DebugRestart()));
	connect(actionDebug_Suspend, SIGNAL(triggered()), this, SLOT(action_DebugSuspend()));
	connect(actionDebug_Step_In, SIGNAL(triggered()), this, SLOT(action_DebugStepIn()));
	connect(actionDebug_Step_Over, SIGNAL(triggered()), this, SLOT(action_DebugStepOver()));
	connect(actionOptions_About, SIGNAL(triggered()), this, SLOT(action_OptionsAbout()));
	connect(actionOptions_Options, SIGNAL(triggered()), this, SLOT(action_OptionsOptions()));
	connect(actionWindow_Detail_Information, SIGNAL(triggered()), this, SLOT(action_WindowDetailInformation()));
	connect(actionWindow_Breakpoint_Manager, SIGNAL(triggered()), this, SLOT(action_WindowBreakpointManager()));
	connect(actionWindow_Show_Memory, SIGNAL(triggered()), this, SLOT(action_WindowShowMemory()));
	connect(actionWindow_Show_Heap, SIGNAL(triggered()), this, SLOT(action_WindowShowHeap()));
	connect(actionWindow_Show_Strings, SIGNAL(triggered()), this, SLOT(action_WindowShowStrings()));
	connect(actionWindow_Show_Debug_Output, SIGNAL(triggered()), this, SLOT(action_WindowShowDebugOutput()));
	connect(actionWindow_Show_Handles, SIGNAL(triggered()), this, SLOT(action_WindowShowHandles()));
	connect(actionWindow_Show_Windows, SIGNAL(triggered()), this, SLOT(action_WindowShowWindows()));

	// Actions on Window Events
	connect(tblDisAs,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomDisassemblerContextMenu(QPoint)));
	connect(tblRegView,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnRegViewChangeRequest(QTableWidgetItem *)));
	connect(tblRegView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomRegViewContextMenu(QPoint)));
	connect(scrollStackView,SIGNAL(valueChanged(int)),this,SLOT(OnStackScroll(int)));
	connect(scrollDisAs,SIGNAL(valueChanged(int)),this,SLOT(OnDisAsScroll(int)));

	// GUI Shortcuts
	connect(new QShortcut(QKeySequence("F2"),this),SIGNAL(activated()),this,SLOT(OnF2BreakPointPlace()));
	connect(new QShortcut(QKeySequence("F4"),this),SIGNAL(activated()),this,SLOT(OnPrintMemoryLeaks()));
}

qtDLGNanomite::~qtDLGNanomite()
{

}

qtDLGNanomite* qtDLGNanomite::GetInstance()
{
	return qtDLGMyWindow;
}

void qtDLGNanomite::action_FileTerminateGUI()
{
	if(coreDebugger->GetDebuggingState())
		coreDebugger->StopDebuggingAll();
	close();
}

void qtDLGNanomite::action_FileOpenNewFile()
{
	if(coreDebugger->GetDebuggingState())
		coreDebugger->StopDebuggingAll();

	coreDebugger->RemoveBPs();
	clsHelperClass::MenuLoadNewFile(coreDebugger);

	CleanGUI();
	UpdateStateBar(0x3);
}

void qtDLGNanomite::action_FileAttachTo()
{
	if(!coreDebugger->GetDebuggingState())
	{
		 qtDLGAttach newAttaching;
		 connect(&newAttaching,SIGNAL(StartAttachedDebug(int)),this,SLOT(action_DebugAttachStart(int)));
		 newAttaching.exec();
	}
}

void qtDLGNanomite::action_FileDetach()
{
	if(coreDebugger->GetDebuggingState())
	{
		if(!coreDebugger->DetachFromProcess())
			MessageBox(NULL,L"Failed to detach from Process!",L"Nanomite",MB_OK);
		else
			UpdateStateBar(0x3);
	}
}

void qtDLGNanomite::action_DebugStart()
{
	if(!coreDebugger->GetDebuggingState())
	{
		CleanGUI();

		if(!coreDebugger->IsTargetSet())
				if(!clsHelperClass::MenuLoadNewFile(coreDebugger))
					return;

		coreDebugger->start();

		UpdateStateBar(0x1);
	}
	else
	{
		coreDebugger->ResumeDebugging();
		UpdateStateBar(0x1);
	}
}

void qtDLGNanomite::action_DebugAttachStart(int iPID)
{
	if(!coreDebugger->GetDebuggingState())
	{
		CleanGUI();

		coreDebugger->AttachToProcess(iPID);
		coreDebugger->start();

		UpdateStateBar(0x1);
	}
}

void qtDLGNanomite::action_DebugStop()
{
	if(coreDebugger->GetDebuggingState())
		coreDebugger->StopDebuggingAll();
}

void qtDLGNanomite::action_DebugRestart()
{
	if(coreDebugger->GetDebuggingState())
		coreDebugger->StopDebuggingAll();

	CleanGUI();
	if(coreDebugger->IsTargetSet())
		coreDebugger->start();
}

void qtDLGNanomite::action_DebugSuspend()
{
	_iMenuPID = -1;
	actionDebug_Suspend->setDisabled(true);

	GenerateMenu();

	if(_iMenuPID == 0)
	{
		if(coreDebugger->GetDebuggingState())
		{
			coreDebugger->SuspendDebuggingAll();
			UpdateStateBar(0x2);
		}
	}
	else
	{
		if(coreDebugger->GetDebuggingState())
		{
			coreDebugger->SuspendDebugging(_iMenuPID);
			UpdateStateBar(0x2);
		}
	}

	actionDebug_Suspend->setEnabled(true);
	_iMenuPID = -1;
}

void qtDLGNanomite::action_DebugStepIn()
{
	if(coreDebugger->GetDebuggingState())
		coreDebugger->StepIn();
}

void qtDLGNanomite::action_DebugStepOver()
{
	quint64 dwEIP = NULL;
#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);
	
	if(bIsWOW64)
		dwEIP = coreDebugger->wowProcessContext.Eip;
	else
		dwEIP = coreDebugger->ProcessContext.Rip;
#else
	dwEIP = coreDebugger->ProcessContext.Eip;
#endif
	QMap<QString,DisAsDataRow>::const_iterator i = coreDisAs->SectionDisAs.constFind(QString("%1").arg(dwEIP,16,16,QChar('0')).toUpper());
	++i;
	if((QMapData::Node *)i == (QMapData::Node *)coreDisAs->SectionDisAs.constEnd())
		coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwEIP);
	
	coreDebugger->StepOver(i.value().Offset.toULongLong(0,16));
}

void qtDLGNanomite::action_OptionsAbout()
{
	qtDLGAbout DLGAbout;
	DLGAbout.exec();
}

void qtDLGNanomite::action_OptionsOptions()
{
	qtDLGOption newOption;
	newOption.exec();
}

void qtDLGNanomite::action_WindowDetailInformation()
{
	dlgDetInfo->show();
}

void qtDLGNanomite::action_WindowBreakpointManager()
{
	dlgBPManager->show();
}

void qtDLGNanomite::action_WindowShowMemory()
{
	if(coreDebugger->GetDebuggingState())
	{
		_iMenuPID = -1;
		actionWindow_Show_Memory->setDisabled(true);
	
		GenerateMenu();

		if(_iMenuPID >= 0)
		{
			qtDLGMemoryView *dlgMemory = new qtDLGMemoryView(this,Qt::Window,_iMenuPID);
			dlgMemory->show();
		}

		actionWindow_Show_Memory->setEnabled(true);
		_iMenuPID = -1;
	}
}

void qtDLGNanomite::action_WindowShowHeap()
{
	if(coreDebugger->GetDebuggingState())
	{
		_iMenuPID = -1;
		actionWindow_Show_Heap->setDisabled(true);
	
		GenerateMenu();

		if(_iMenuPID >= 0)
		{
			qtDLGHeapView *dlgHeap = new qtDLGHeapView(this,Qt::Window,_iMenuPID);
			dlgHeap->show();
		}

		actionWindow_Show_Heap->setEnabled(true);
		_iMenuPID = -1;
	}
}

void qtDLGNanomite::action_WindowShowStrings()
{
	if(coreDebugger->GetDebuggingState())
	{
		_iMenuPID = -1;
		actionWindow_Show_Strings->setDisabled(true);
	
		GenerateMenu();

		if(_iMenuPID >= 0)
		{
			qtDLGStringView *dlgString = new qtDLGStringView(this,Qt::Window,_iMenuPID);
			dlgString->show();
		}

		actionWindow_Show_Strings->setEnabled(true);
		_iMenuPID = -1;
	}
}

void qtDLGNanomite::action_WindowShowDebugOutput()
{
	dlgDbgStr->show();
}

void qtDLGNanomite::action_WindowShowHandles()
{
	if(coreDebugger->GetDebuggingState())
	{
		_iMenuPID = -1;
		actionWindow_Show_Handles->setDisabled(true);
	
		GenerateMenu();

		if(_iMenuPID >= 0)
		{
			qtDLGHandleView *dlgHandle = new qtDLGHandleView(this,Qt::Window,_iMenuPID);
			dlgHandle->show();
		}

		actionWindow_Show_Handles->setEnabled(true);
		_iMenuPID = -1;
	}
}

void qtDLGNanomite::action_WindowShowWindows()
{
	if(coreDebugger->GetDebuggingState())
	{
		_iMenuPID = -1;
		actionWindow_Show_Windows->setDisabled(true);
	
		GenerateMenu();

		if(_iMenuPID >= 0)
		{
			qtDLGWindowView *dlgWindows = new qtDLGWindowView(this,Qt::Window,_iMenuPID);
			dlgWindows->show();
		}

		actionWindow_Show_Windows->setEnabled(true);
		_iMenuPID = -1;
	}
}

void qtDLGNanomite::OnDebuggerBreak()
{
	quint64 dwEIP = NULL;
	if(!coreDebugger->GetDebuggingState())
		UpdateStateBar(0x3);
	else
	{
		tblCallstack->setRowCount(0);
		coreDebugger->ShowCallStack();

		LoadRegView();

#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		if(clsAPIImport::pIsWow64Process)
			clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);
		
		if(bIsWOW64)
		{
			dwEIP = coreDebugger->wowProcessContext.Eip;
			LoadStackView(coreDebugger->wowProcessContext.Esp,4);
		}
		else
		{
			dwEIP = coreDebugger->ProcessContext.Rip;
			LoadStackView(coreDebugger->ProcessContext.Rsp,8);
		}
#else
		dwEIP = coreDebugger->ProcessContext.Eip;
		LoadStackView(coreDebugger->ProcessContext.Esp,4);
#endif	
		if(!coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwEIP))
			OnDisplayDisassembly(dwEIP);
		UpdateStateBar(0x2);
	}
}

void qtDLGNanomite::UpdateStateBar(DWORD dwAction)
{
	QString qsStateMessage = QString().sprintf("\t\tPIDs: %d  TIDs: %d  DLLs: %d  Exceptions: %d State: ",
		coreDebugger->PIDs.size(),
		coreDebugger->TIDs.size(),
		coreDebugger->DLLs.size(),
		lExceptionCount);

	switch(dwAction)
	{
	case 0x1: // Running
		qsStateMessage.append("Running");
		break;
	case 0x2: // Suspended
		qsStateMessage.append("Suspended");
		break;
	case 0x3: // Terminated
		qsStateMessage.append("Terminated");
		break;
	}
	stateBar->showMessage(qsStateMessage);
}

void qtDLGNanomite::LoadStackView(quint64 dwESP, DWORD dwStackSize)
{
	bool bCheckVar = false;
	SIZE_T dwBytesRead = NULL;
	wstring sFuncName,sModName;
	LPBYTE bBuffer;
	PTCHAR sTemp;
	HANDLE hProcess = coreDebugger->GetCurrentProcessHandle();
	DWORD dwOldProtect = NULL,
		dwNewProtect = PAGE_EXECUTE_READWRITE,
		dwRowCount = (tblStack->verticalHeader()->height() + 4) / 28,
		dwSize = (dwRowCount * 2) * dwStackSize;
	quint64	dwStartOffset = dwESP - dwStackSize * dwRowCount,
		dwEndOffset = dwESP + dwStackSize * dwRowCount;

	if(!coreDebugger->GetDebuggingState())
		return;

	if(hProcess == INVALID_HANDLE_VALUE)
		return;

	if(!VirtualProtectEx(hProcess,(LPVOID)dwStartOffset,dwSize,dwNewProtect,&dwOldProtect))
		return;

	bBuffer = (LPBYTE)malloc(dwSize);
	if(bBuffer == NULL)
		return;

	if(!ReadProcessMemory(hProcess,(LPVOID)dwStartOffset,(LPVOID)bBuffer,dwSize,&dwBytesRead))
		return;

	sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

	tblStack->setRowCount(0);
	for(size_t i = 0;i < (dwSize / dwStackSize); i++)
	{
		tblStack->insertRow(tblStack->rowCount());
		int itemIndex = tblStack->rowCount();

		// Current Offset
		wsprintf(sTemp,L"0x%016I64X",(dwStartOffset + i * dwStackSize));
		tblStack->setItem(itemIndex - 1,0,new QTableWidgetItem(QString::fromWCharArray(sTemp)));

		// Value
		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		IsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

		if(bIsWOW64)
			for(int id = 3;id != -1;id--)
				wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * dwStackSize + id)));
		else
			for(int id = 7;id != -1;id--)
				wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * dwStackSize + id)));

#else
		for(int id = 3;id != -1;id--)
			wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * dwStackSize + id)));
#endif
		tblStack->setItem(itemIndex - 1,1,new QTableWidgetItem(QString::fromWCharArray(sTemp)));

		// Comment
		clsHelperClass::LoadSymbolForAddr(sFuncName,sModName,QString::fromWCharArray(sTemp).toULongLong(0,16),
			coreDebugger->GetCurrentProcessHandle());
		if(sFuncName.length() > 0 && sModName.length() > 0)
			tblStack->setItem(itemIndex - 1,2,
			new QTableWidgetItem(QString::fromStdWString(sModName).append(".").append(QString::fromStdWString(sFuncName))));
		else if(sFuncName.length() > 0)
			tblStack->setItem(itemIndex - 1,2,new QTableWidgetItem(QString::fromStdWString(sFuncName)));	
	}

	bCheckVar = VirtualProtectEx(hProcess,(LPVOID)dwStartOffset,dwSize,dwOldProtect,NULL);
	free(bBuffer);
	free(sTemp);
}

void qtDLGNanomite::InitListSizes()
{
	// List StackView
	tblStack->horizontalHeader()->resizeSection(0,135);
	tblStack->horizontalHeader()->resizeSection(1,135);

	// List LogBox
	tblLogBox->horizontalHeader()->resizeSection(0,85);

	// List CallStack
	tblCallstack->horizontalHeader()->resizeSection(0,135);
	tblCallstack->horizontalHeader()->resizeSection(1,135);
	tblCallstack->horizontalHeader()->resizeSection(2,300);
	tblCallstack->horizontalHeader()->resizeSection(3,135);
	tblCallstack->horizontalHeader()->resizeSection(4,300);
	tblCallstack->horizontalHeader()->resizeSection(5,75);

	// List DisAs
	tblDisAs->horizontalHeader()->resizeSection(0,135);
	tblDisAs->horizontalHeader()->resizeSection(1,250);
	tblDisAs->horizontalHeader()->resizeSection(2,250);

	// List Register
	tblRegView->horizontalHeader()->resizeSection(0,125);

	// List DetInfo  Processes
	dlgDetInfo->tblPIDs->horizontalHeader()->resizeSection(0,135);
	dlgDetInfo->tblPIDs->horizontalHeader()->resizeSection(1,135);
	dlgDetInfo->tblPIDs->horizontalHeader()->resizeSection(2,135);

	// List DetInfo  Threads
	dlgDetInfo->tblTIDs->horizontalHeader()->resizeSection(0,135);
	dlgDetInfo->tblTIDs->horizontalHeader()->resizeSection(1,135);
	dlgDetInfo->tblTIDs->horizontalHeader()->resizeSection(2,135);
	dlgDetInfo->tblTIDs->horizontalHeader()->resizeSection(3,135);

	// List DetInfo  Exceptions
	dlgDetInfo->tblExceptions->horizontalHeader()->resizeSection(0,135);
	dlgDetInfo->tblExceptions->horizontalHeader()->resizeSection(1,135);
	dlgDetInfo->tblExceptions->horizontalHeader()->resizeSection(2,140);

	// List DetInfo  Modules
	dlgDetInfo->tblModules->horizontalHeader()->resizeSection(0,135);
	dlgDetInfo->tblModules->horizontalHeader()->resizeSection(1,135);
	dlgDetInfo->tblModules->horizontalHeader()->resizeSection(2,135);

	// List Debug Strings
	dlgDbgStr->tblDebugStrings->horizontalHeader()->resizeSection(0,75);

	// List BP Manager
	dlgBPManager->tblBPs->horizontalHeader()->resizeSection(0,75);
	dlgBPManager->tblBPs->horizontalHeader()->resizeSection(1,135);
	dlgBPManager->tblBPs->horizontalHeader()->resizeSection(2,135);
	dlgBPManager->tblBPs->horizontalHeader()->resizeSection(3,50);

	this->setStyleSheet("background: rgb(230, 235, 230)");
}

void qtDLGNanomite::OnStackScroll(int iValue)
{
	if(iValue == 5) return;

	DWORD dwStackSize = NULL;
	quint64 dwOffset = NULL;
	QString strTemp;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64) dwStackSize = 4;	
	else dwStackSize = 8;
#else
	dwStackSize = 4;
#endif

	if(tblStack->rowCount() > 0)
		strTemp = tblStack->item(0,0)->text();
	if(tblStack->rowCount() <= 0 || QString().compare(strTemp,"0") == 0)
#ifdef _AMD64_
		dwOffset = coreDebugger->ProcessContext.Rsp;
#else
		dwOffset = coreDebugger->ProcessContext.Esp;
#endif
	else
		dwOffset = strTemp.toULongLong(0,16);

	if(iValue < 5)
		LoadStackView(dwOffset,dwStackSize);
	else
	{
		if(((tblStack->verticalHeader()->height() + 4) / 14) % 2)
			LoadStackView(dwOffset + (dwStackSize * ((tblStack->verticalHeader()->height() + 4) / 14)),dwStackSize);
		else
			LoadStackView(dwOffset + (dwStackSize * (((tblStack->verticalHeader()->height() + 4) / 14) + 1)),dwStackSize);
	}
	scrollStackView->setValue(5);
}

void qtDLGNanomite::OnDisAsScroll(int iValue)
{
	if(iValue == 5 || tblDisAs->rowCount() <= 10) return;
	else if(iValue < 5)
		OnDisplayDisassembly(tblDisAs->item(0,0)->text().toULongLong(0,16));
	else
	{
		if(tblDisAs->rowCount() <= 25)
		{
			scrollDisAs->setValue(5);
			return;
		}

		OnDisplayDisassembly(tblDisAs->item(10,0)->text().toULongLong(0,16));
	}
	scrollDisAs->setValue(5);
}

void qtDLGNanomite::LoadRegView()
{
	tblRegView->setRowCount(0);
	DWORD dwEFlags = NULL;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;

	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64)
	{
		dwEFlags = coreDebugger->wowProcessContext.EFlags;
		// EAX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EAX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Eax,16,16,QChar('0'))));
		// EBX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EBX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Ebx,16,16,QChar('0'))));
		// ECX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ECX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Ecx,16,16,QChar('0'))));
		// EDX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EDX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Edx,16,16,QChar('0'))));
		// ESP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ESP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Esp,16,16,QChar('0'))));
		// EBP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EBP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Ebp,16,16,QChar('0'))));
		// ESI
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ESI"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Esi,16,16,QChar('0'))));
		// EIP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EIP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Eip,16,16,QChar('0'))));
		// EFlags
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EFlags"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.EFlags,16,16,QChar('0'))));
	}
	else
	{
		dwEFlags = coreDebugger->ProcessContext.EFlags;
		// RAX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RAX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rax,16,16,QChar('0'))));
		// RBX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RBX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rbx,16,16,QChar('0'))));
		// RCX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RCX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rcx,16,16,QChar('0'))));
		// RDX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RDX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rdx,16,16,QChar('0'))));
		// RSP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RSP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rsp,16,16,QChar('0'))));
		// RBP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RBP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rbp,16,16,QChar('0'))));
		// RSI
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RSI"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rsi,16,16,QChar('0'))));
		// RIP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RIP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rip,16,16,QChar('0'))));
		// R8
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R8"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R8,16,16,QChar('0'))));
		// R9
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R9"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R9,16,16,QChar('0'))));
		// R10
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R10"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R10,16,16,QChar('0'))));
		// R11
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R11"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R11,16,16,QChar('0'))));
		// R12
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R12"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R12,16,16,QChar('0'))));
		// R13
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R13"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R13,16,16,QChar('0'))));
		// R14
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R14"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R14,16,16,QChar('0'))));
		// R15
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R15"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R15,16,16,QChar('0'))));
		// EFlags
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EFlags"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.EFlags,8,16,QChar('0'))));
	}
#else
	dwEFlags = coreDebugger->ProcessContext.EFlags;
	// EAX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EAX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Eax,16,16,QChar('0'))));
	// EBX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EBX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Ebx,16,16,QChar('0'))));
	// ECX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ECX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Ecx,16,16,QChar('0'))));
	// EDX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EDX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Edx,16,16,QChar('0'))));
	// ESP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ESP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Esp,16,16,QChar('0'))));
	// EBP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EBP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Ebp,16,16,QChar('0'))));
	// ESI
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ESI"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Esi,16,16,QChar('0'))));
	// EIP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EIP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Eip,16,16,QChar('0'))));
	// EFlags
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EFlags"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.EFlags,16,16,QChar('0'))));
#endif

	BOOL bCF = false, // Carry Flag
		 bPF = false, // Parity Flag
		 bAF = false, // Auxiliarty carry flag
		 bZF = false, // Zero Flag
		 bSF = false, // Sign Flag
		 bTF = false, // Trap Flag
		 bIF = false, // Interrupt Flag
		 bDF = false, // Direction Flag
		 bOF = false; // Overflow Flag

	bOF = (dwEFlags & 0x800) ? true : false;
	bDF = (dwEFlags & 0x400) ? true : false;
	bTF = (dwEFlags & 0x100) ? true : false;
	bSF = (dwEFlags & 0x80) ? true : false;
	bZF = (dwEFlags & 0x40) ? true : false;
	bAF = (dwEFlags & 0x10) ? true : false;
	bPF = (dwEFlags & 0x4) ? true : false;
	bCF = (dwEFlags & 0x1) ? true : false;

	// OF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("OF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bOF)));
	// DF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("DF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bDF)));
	// TF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("TF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bTF)));
	// SF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("SF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bSF)));
	// ZF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ZF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bZF)));
	// AF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("AF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bAF)));
	// PF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("PF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bPF)));
	// CF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("CF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bCF)));
}

void qtDLGNanomite::CleanGUI()
{
	tblStack->setRowCount(0);
	tblLogBox->setRowCount(0);
	tblCallstack->setRowCount(0);
	tblDisAs->setRowCount(0);
	tblRegView->setRowCount(0);
	
	dlgDetInfo->tblPIDs->setRowCount(0);
	dlgDetInfo->tblTIDs->setRowCount(0);
	dlgDetInfo->tblExceptions->setRowCount(0);
	dlgDetInfo->tblModules->setRowCount(0);

	dlgDbgStr->tblDebugStrings->setRowCount(0);

	lExceptionCount = 0;
}

void qtDLGNanomite::OnDebuggerTerminated()
{
	UpdateStateBar(0x3);
	coreDisAs->SectionDisAs.clear();
}

void qtDLGNanomite::GenerateMenuCallback(QAction *qAction)
{
	_iMenuPID = qAction->text().toULong(0,16);
}

void qtDLGNanomite::GenerateMenu()
{
	QAction *qAction;
	QMenu menu;

	for(int i = 0; i < coreDebugger->PIDs.size(); i++)
	{
		if(coreDebugger->PIDs[i].bRunning)
		{
			qAction = new QAction(QString().sprintf("%08X",coreDebugger->PIDs[i].dwPID),this);
			menu.addAction(qAction);
		}
	}
	menu.addSeparator();
	qAction = new QAction("All",this);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(GenerateMenuCallback(QAction*)));
	menu.addAction(qAction);
	menu.exec(QCursor::pos());
}

void qtDLGNanomite::OnDisplayDisassembly(quint64 dwEIP)
{
	if(coreDisAs->SectionDisAs.count() > 0 && dwEIP != 0)
	{
		tblDisAs->setRowCount(0);

		int iLines = 0;
		QMap<QString,DisAsDataRow>::const_iterator i = coreDisAs->SectionDisAs.constFind(QString("%1").arg(dwEIP,16,16,QChar('0')).toUpper());
		QMap<QString,DisAsDataRow>::const_iterator iEnd = coreDisAs->SectionDisAs.constEnd();--iEnd;

		if((QMapData::Node *)i == (QMapData::Node *)coreDisAs->SectionDisAs.constEnd())
		{
			coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwEIP);
			return;
		}

		for(int iBack = 0; iBack <= 5; iBack++)
		{
			if(!i.value().Offset.isEmpty() && i.value().Offset.compare(coreDisAs->SectionDisAs.begin().value().Offset) == 0)
			{
				coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),i.value().Offset.toULongLong(0,16));
				return;
			}			
			--i;
		}

		
		quint64 itemStyle;
		while(iLines <= ((tblDisAs->verticalHeader()->height() + 4) / 14) - 2)
		{
			itemStyle = i.value().itemStyle;
			tblDisAs->insertRow(tblDisAs->rowCount());

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 0,
				new QTableWidgetItem(i.value().Offset));
			if(itemStyle & COLOR_BP)
				tblDisAs->item(tblDisAs->rowCount() - 1,0)->setForeground(QColor(qtNanomiteDisAsColor->colorBP));

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 1,
				new QTableWidgetItem(i.value().OpCodes));

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 2,
				new QTableWidgetItem(i.value().ASM));
			if(itemStyle & COLOR_CALLS)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorCall));
			else if(itemStyle & COLOR_JUMP)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorJump));
			else if(itemStyle & COLOR_MOVE)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorMove));
			else if(itemStyle & COLOR_STACK)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorStack));

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 3,
				new QTableWidgetItem(i.value().Comment));

			if(!i.value().Offset.isEmpty() && i.value().Offset.compare(iEnd.value().Offset) == 0)
			{
				coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),iEnd.value().Offset.toULongLong(0,16));
				return;
			}
			
			++iLines;++i;
		}
	}
}

void qtDLGNanomite::OnCustomRegViewContextMenu(QPoint qPoint)
{
	QAction *qAction;
	QMenu menu;

	_iSelectedRow = tblRegView->indexAt(qPoint).row();

	qAction = new QAction("Send to Disassembler",this);
	menu.addAction(qAction);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGNanomite::OnCustomDisassemblerContextMenu(QPoint qPoint)
{
	QAction *qAction;
	QMenu menu;

	_iSelectedRow = tblDisAs->indexAt(qPoint).row();

	qAction = new QAction("Goto Offset",this);
	menu.addAction(qAction);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGNanomite::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
	{
		if(!coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),tblRegView->item(_iSelectedRow,1)->text().toULongLong(0,16)))
			OnDisplayDisassembly(tblRegView->item(_iSelectedRow,1)->text().toULongLong(0,16));
	}
	else if(QString().compare(pAction->text(),"Goto Offset") == 0)
	{
		bool bOk = false;
		QString strNewOffset = QInputDialog::getText(this,"Please give a Offset:","VA:",QLineEdit::Normal,NULL,&bOk);
		
		if(bOk && !strNewOffset.isEmpty())
			if(!coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),strNewOffset.toULongLong(0,16)))
				OnDisplayDisassembly(tblRegView->item(_iSelectedRow,1)->text().toULongLong(0,16));		
	}
}

void qtDLGNanomite::OnRegViewChangeRequest(QTableWidgetItem *pItem)
{
	if(!coreDebugger->GetDebuggingState())
		return;
	
	qtDLGRegEdit *newRegEditWindow;
#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		if(clsAPIImport::pIsWow64Process)
			clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);
		
		if(bIsWOW64)
			newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&coreDebugger->wowProcessContext,false);
		else
			newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&coreDebugger->ProcessContext,true);			
#else
	newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&coreDebugger->ProcessContext,false);			
#endif	

	connect(newRegEditWindow,SIGNAL(OnUpdateRegView()),this,SLOT(LoadRegView()));
	newRegEditWindow->exec();
	// show qtDLGRegEdit
}

void qtDLGNanomite::resizeEvent(QResizeEvent *event)
{
	OnDebuggerBreak();
}

void qtDLGNanomite::OnF2BreakPointPlace()
{
	QList<QTableWidgetItem *> currentSelectedItems = tblDisAs->selectedItems();
	if(!coreDebugger->GetDebuggingState() || currentSelectedItems.count() <= 0) return;

	quint64 dwSelectedVA = currentSelectedItems.value(0)->text().toULongLong(0,16);
	if(coreDebugger->AddBreakpointToList(NULL,DR_EXECUTE,-1,dwSelectedVA,NULL,true))
		currentSelectedItems.value(0)->setForeground(Qt::red);

	return;
}

void qtDLGNanomite::OnPrintMemoryLeaks()
{
	_CrtDumpMemoryLeaks();
}