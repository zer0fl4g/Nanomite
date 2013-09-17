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
#include <QShortcut>
#include <QClipboard>

#include "qtDLGNanomite.h"
#include "qtDLGRegEdit.h"
#include "qtDLGAssembler.h"
#include "qtDLGDisassembler.h"
#include "qtDLGExceptionAsk.h"

#include "clsHelperClass.h"
#include "clsDisassembler.h"
#include "clsAPIImport.h"
#include "clsDBInterface.h"
#include "clsMemManager.h"

#include <Psapi.h>

using namespace std;

qtDLGNanomite* qtDLGNanomite::qtDLGMyWindow = NULL;

qtDLGNanomite::qtDLGNanomite(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags),
	m_IsRestart(false)
{
	setupUi(this);

	setAcceptDrops(true);

	QApplication::setStyle(new QPlastiqueStyle);
	QFontDatabase::addApplicationFont(":/Fonts/Fonts/consola.ttf");
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());

	qRegisterMetaType<DWORD>("DWORD");
	qRegisterMetaType<quint64>("quint64");
	qRegisterMetaType<wstring>("wstring");
	qRegisterMetaType<BPStruct>("BPStruct");
	qRegisterMetaType<HANDLE>("HANDLE");

	clsAPIImport::LoadFunctions();

	coreBPManager = new clsBreakpointManager;
	coreDebugger = new clsDebugger(coreBPManager);
	coreDisAs = new clsDisassembler;
	PEManager = new clsPEManager;
	DBManager = new clsDBManager;
	dlgDetInfo = new qtDLGDetailInfo(this,Qt::Window);
	dlgDbgStr = new qtDLGDebugStrings(this,Qt::Window);
	dlgBPManager = new qtDLGBreakPointManager(this,Qt::Window);
	dlgTraceWindow = new qtDLGTrace(this,Qt::Window);
	dlgPatchManager = new qtDLGPatchManager(this,Qt::Window);
	qtNanomiteDisAsColor = new qtNanomiteDisAsColorSettings;	

	qtDLGMyWindow = this;
	lExceptionCount = 0;

	LoadWidgets();

	settings->CheckIfFirstRun();
	settings->LoadDebuggerSettings(coreDebugger);
	settings->LoadDisassemblerColor(qtNanomiteDisAsColor);
	settings->LoadRecentDebuggedFiles(m_recentDebuggedFiles);
	
	LoadRecentFileMenu(true);

	DisAsGUI = new qtDLGDisassembler(this);
	this->setCentralWidget(DisAsGUI);

	// Callbacks from Debugger Thread to GUI
	connect(coreDebugger,SIGNAL(OnThread(DWORD,DWORD,quint64,bool,DWORD,bool)),
		dlgDetInfo,SLOT(OnThread(DWORD,DWORD,quint64,bool,DWORD,bool)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnPID(DWORD,std::wstring,DWORD,quint64,bool)),
		dlgDetInfo,SLOT(OnPID(DWORD,std::wstring,DWORD,quint64,bool)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnException(std::wstring,std::wstring,quint64,quint64,DWORD,DWORD)),
		dlgDetInfo,SLOT(OnException(std::wstring,std::wstring,quint64,quint64,DWORD,DWORD)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDbgString(std::wstring,DWORD)),
		dlgDbgStr,SLOT(OnDbgString(std::wstring,DWORD)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDll(std::wstring,DWORD,quint64,bool)),
		dlgDetInfo,SLOT(OnDll(std::wstring,DWORD,quint64,bool)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnLog(std::wstring)),
		logView,SLOT(OnLog(std::wstring)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnCallStack(quint64,quint64,std::wstring,std::wstring,quint64,std::wstring,std::wstring,std::wstring,int)),
		callstackView,SLOT(OnCallStack(quint64,quint64,std::wstring,std::wstring,quint64,std::wstring,std::wstring,std::wstring,int)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(AskForException(DWORD)),this,SLOT(AskForException(DWORD)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDebuggerBreak()),this,SLOT(OnDebuggerBreak()),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDebuggerTerminated()),this,SLOT(OnDebuggerTerminated()),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnNewPID(std::wstring,int)),dlgBPManager,SLOT(UpdateCompleter(std::wstring,int)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(UpdateOffsetsPatches(HANDLE,int)),dlgPatchManager,SLOT(UpdateOffsetPatch(HANDLE,int)),Qt::QueuedConnection);

	connect(coreBPManager,SIGNAL(OnBreakpointAdded(BPStruct,int)),dlgBPManager,SLOT(OnUpdate(BPStruct,int)),Qt::QueuedConnection);
	connect(coreBPManager,SIGNAL(OnBreakpointDeleted(quint64)),dlgBPManager,SLOT(OnDelete(quint64)),Qt::QueuedConnection);

	// Callbacks from Debugger to PEManager
	connect(coreDebugger,SIGNAL(OnNewPID(std::wstring,int)),PEManager,SLOT(InsertPIDForFile(std::wstring,int)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(DeletePEManagerObject(std::wstring,int)),PEManager,SLOT(CloseFile(std::wstring,int)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(CleanPEManager()),PEManager,SLOT(CleanPEManager()),Qt::QueuedConnection);

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
	connect(actionDebug_Step_Out, SIGNAL(triggered()), this, SLOT(action_DebugStepOut()));
	connect(actionDebug_Step_Over, SIGNAL(triggered()), this, SLOT(action_DebugStepOver()));
	connect(actionOptions_About, SIGNAL(triggered()), this, SLOT(action_OptionsAbout()));
	connect(actionOptions_Options, SIGNAL(triggered()), this, SLOT(action_OptionsOptions()));
	connect(actionOptions_Update, SIGNAL(triggered()), this, SLOT(action_OptionsUpdate()));
	connect(actionWindow_Detail_Information, SIGNAL(triggered()), this, SLOT(action_WindowDetailInformation()));
	connect(actionWindow_Breakpoint_Manager, SIGNAL(triggered()), this, SLOT(action_WindowBreakpointManager()));
	connect(actionWindow_Show_Patches, SIGNAL(triggered()), this, SLOT(action_WindowPatches()));
	connect(actionWindow_Show_Memory, SIGNAL(triggered()), this, SLOT(action_WindowShowMemory()));
	connect(actionWindow_Show_Heap, SIGNAL(triggered()), this, SLOT(action_WindowShowHeap()));
	connect(actionWindow_Show_Strings, SIGNAL(triggered()), this, SLOT(action_WindowShowStrings()));
	connect(actionWindow_Show_Debug_Output, SIGNAL(triggered()), this, SLOT(action_WindowShowDebugOutput()));
	connect(actionWindow_Show_Handles, SIGNAL(triggered()), this, SLOT(action_WindowShowHandles()));
	connect(actionWindow_Show_Windows, SIGNAL(triggered()), this, SLOT(action_WindowShowWindows()));
	connect(actionWindow_Show_Functions, SIGNAL(triggered()), this, SLOT(action_WindowShowFunctions()));
	connect(actionWindow_Show_Privileges, SIGNAL(triggered()), this, SLOT(action_WindowShowPrivileges()));
	connect(action_Debug_Run_to_UserCode,SIGNAL(triggered()), this, SLOT(action_DebugRunToUserCode()));
	connect(actionDebug_Trace_Start, SIGNAL(triggered()), this, SLOT(action_DebugTraceStart()));
	connect(actionDebug_Trace_Stop, SIGNAL(triggered()), this, SLOT(action_DebugTraceStop()));
	connect(actionDebug_Trace_Show, SIGNAL(triggered()), this, SLOT(action_DebugTraceShow()));
	connect(actionWindow_Show_PEEditor, SIGNAL(triggered()), this, SLOT(action_WindowShowPEEditor()));

	// Callbacks to display disassembly
	connect(dlgTraceWindow,SIGNAL(OnDisplayDisassembly(quint64)),DisAsGUI,SLOT(OnDisplayDisassembly(quint64)));
	connect(cpuRegView,SIGNAL(OnDisplayDisassembly(quint64)),DisAsGUI,SLOT(OnDisplayDisassembly(quint64)));
	connect(dlgDetInfo,SIGNAL(ShowInDisassembler(quint64)),DisAsGUI,SLOT(OnDisplayDisassembly(quint64)));
	connect(coreDisAs,SIGNAL(DisAsFinished(quint64)),DisAsGUI,SLOT(OnDisplayDisassembly(quint64)),Qt::QueuedConnection);
	connect(dlgBPManager,SIGNAL(OnDisplayDisassembly(quint64)),DisAsGUI,SLOT(OnDisplayDisassembly(quint64)));

	// Callbacks from PatchManager to GUI
	connect(dlgPatchManager,SIGNAL(OnReloadDebugger()),this,SLOT(OnDebuggerBreak()));

	// Callbacks from Disassembler GUI to GUI
	connect(DisAsGUI,SIGNAL(OnDebuggerBreak()),this,SLOT(OnDebuggerBreak()));

	// Callbacks to StateBar
	connect(dlgTraceWindow,SIGNAL(OnUpdateStatusBar(int,quint64)),this,SLOT(UpdateStateBar(int,quint64)));

	actionDebug_Trace_Stop->setDisabled(true);

	ParseCommandLineArgs();
}

qtDLGNanomite::~qtDLGNanomite()
{
	settings->SaveDebuggerSettings(coreDebugger);
	settings->SaveDisassemblerColor(qtNanomiteDisAsColor);
	settings->SaveRecentDebuggedFiles(m_recentDebuggedFiles);

	delete coreBPManager;
	delete coreDebugger;
	delete coreDisAs;
	delete PEManager;
	delete DBManager;
	delete settings;
	delete dlgDetInfo;
	delete dlgDbgStr;
	delete dlgBPManager;
	delete dlgTraceWindow;
	delete dlgPatchManager;
	delete qtNanomiteDisAsColor;
	delete cpuRegView;
	delete callstackView;
	delete stackView;
	delete logView;
	delete DisAsGUI;
	delete m_pRecentFiles;
}

qtDLGNanomite* qtDLGNanomite::GetInstance()
{
	return qtDLGMyWindow;
}

void qtDLGNanomite::LoadWidgets()
{
	this->cpuRegView	= new qtDLGRegisters(this);
	this->callstackView = new qtDLGCallstack(this);
	this->stackView		= new qtDLGStack(this);
	this->logView		= new qtDLGLogView(this);

	this->addDockWidget(Qt::RightDockWidgetArea, this->cpuRegView);
	this->addDockWidget(Qt::BottomDockWidgetArea, this->callstackView);

	this->addDockWidget(Qt::BottomDockWidgetArea, this->stackView);
	this->addDockWidget(Qt::BottomDockWidgetArea, this->logView);

	settings = clsAppSettings::SharedInstance();

	if (!settings->RestoreWindowState(this))
	{
		this->splitDockWidget(this->callstackView, this->stackView, Qt::Vertical);
		this->splitDockWidget(this->stackView, this->logView, Qt::Horizontal);
	}
}

void qtDLGNanomite::OnDebuggerBreak()
{
	if(!coreDebugger->GetDebuggingState())
		UpdateStateBar(STATE_TERMINATE);
	else
	{
		// clear tracing stuff
		coreDebugger->SetTraceFlagForPID(coreDebugger->GetCurrentPID(),false);
		actionDebug_Trace_Stop->setEnabled(false);
		actionDebug_Trace_Start->setEnabled(true);
		qtDLGTrace::disableStatusBarTimer();

		// display callstack
		callstackView->tblCallstack->setRowCount(0);
		coreDebugger->ShowCallStack();

		// display Reg
		cpuRegView->LoadRegView(coreDebugger);

		quint64 dwEIP = NULL;
#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		if(clsAPIImport::pIsWow64Process)
			clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

		if(bIsWOW64)
		{
			dwEIP = coreDebugger->wowProcessContext.Eip;
			stackView->LoadStackView(coreDebugger->wowProcessContext.Esp,4);
		}
		else
		{
			dwEIP = coreDebugger->ProcessContext.Rip;
			stackView->LoadStackView(coreDebugger->ProcessContext.Rsp,8);
		}
#else
		dwEIP = coreDebugger->ProcessContext.Eip;
		stackView->LoadStackView(coreDebugger->ProcessContext.Esp,4);
#endif
		// Load SourceFile to Dlg
		wstring FilePath; 
		int LineNumber = NULL;

		clsHelperClass::LoadSourceForAddr(FilePath,LineNumber,dwEIP,coreDebugger->GetCurrentProcessHandle());
		if(FilePath.length() > 0 && LineNumber > 0)
			DisAsGUI->dlgSourceViewer->OnDisplaySource(QString::fromStdWString(FilePath),LineNumber);	

		// Update Disassembler
		if(!coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwEIP))
			DisAsGUI->OnDisplayDisassembly(dwEIP);

		// Update Toolbar
		UpdateStateBar(STATE_SUSPEND);
	}
}

void qtDLGNanomite::UpdateStateBar(int actionType, quint64 stepCount)
{
	QString qsStateMessage = QString().sprintf("\t\tPIDs: %d  TIDs: %d  DLLs: %d  Exceptions: %d State: ",
		coreDebugger->PIDs.size(),
		coreDebugger->TIDs.size(),
		coreDebugger->DLLs.size(),
		lExceptionCount);

	switch(actionType)
	{
	case 1: // Running
		stateBar->setStyleSheet("background-color: green");
		qsStateMessage.append("Running");
		break;
	case 2: // Suspended
		stateBar->setStyleSheet("background-color: yellow");
		qsStateMessage.append("Suspended");
		break;
	case 3: // Terminated
		stateBar->setStyleSheet("background-color: red");
		qsStateMessage.append("Terminated");
		break;
	case 4: // Tracing
		stateBar->setStyleSheet("background-color: green");
		qsStateMessage.append(QString("Tracing - %1/s").arg(stepCount));
		break;
	}

	stateBar->showMessage(qsStateMessage);
}

void qtDLGNanomite::CleanGUI(bool bKeepLogBox)
{
	stackView->tblStack->setRowCount(0);
	if(!bKeepLogBox)
		logView->tblLogBox->setRowCount(0);
	callstackView->tblCallstack->setRowCount(0);
	DisAsGUI->tblDisAs->setRowCount(0);
	cpuRegView->tblRegView->setRowCount(0);

	dlgDetInfo->tblPIDs->setRowCount(0);
	dlgDetInfo->tblTIDs->setRowCount(0);
	dlgDetInfo->tblExceptions->setRowCount(0);
	dlgDetInfo->tblModules->setRowCount(0);

	dlgTraceWindow->tblTraceLog->setRowCount(0);

	DisAsGUI->dlgSourceViewer->listSource->clear();

	dlgDbgStr->tblDebugStrings->setRowCount(0);

	lExceptionCount = 0;
}

void qtDLGNanomite::OnDebuggerTerminated()
{
	coreDisAs->SectionDisAs.clear();
	dlgBPManager->DeleteCompleterContent();
	qtDLGTrace::disableStatusBarTimer();
	qtDLGTrace::clearTraceData();
	actionDebug_Trace_Start->setEnabled(true);
	actionDebug_Trace_Stop->setEnabled(false);
	CleanGUI(true);
	this->setWindowTitle(QString("[Nanomite v 0.1]"));
	qtDLGPatchManager::ResetPatches();
	UpdateStateBar(STATE_TERMINATE);
	LoadRecentFileMenu();

	if(m_IsRestart)
	{
		m_IsRestart = false;
		action_DebugStart();
	}
}

void qtDLGNanomite::GenerateMenuCallback(QAction *qAction)
{
	m_iMenuProcessID = qAction->text().toULong(0,16);
}

void qtDLGNanomite::GenerateMenu(bool isAllEnabled)
{
	int activeProcessCount = 0;
	
	for(int i = 0; i < coreDebugger->PIDs.size(); i++)
	{
		if(coreDebugger->PIDs[i].bRunning)
			activeProcessCount++;
	}

	if(activeProcessCount > 1)
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

		if(isAllEnabled)
		{
			qAction = new QAction("All",this);
			menu.addAction(qAction);
		}

		connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(GenerateMenuCallback(QAction*)));
		menu.exec(QCursor::pos());
	}
	else
	{
		m_iMenuProcessID = coreDebugger->PIDs[0].dwPID;
	}
}

void qtDLGNanomite::dragEnterEvent(QDragEnterEvent* pEvent)
{
	if(pEvent->mimeData()->hasUrls()) {
        pEvent->acceptProposedAction();
    }
}

void qtDLGNanomite::dropEvent(QDropEvent* pEvent)
{ 
	if(pEvent->mimeData()->hasUrls())
    {
		coreDebugger->SetTarget(QString(pEvent->mimeData()->data("FileName")).toStdWString());
		action_DebugStart();
		pEvent->acceptProposedAction();
    }
}

void qtDLGNanomite::closeEvent(QCloseEvent* closeEvent)
{
	clsAppSettings::SharedInstance()->SaveWindowState(this);

	closeEvent->accept();
}

void qtDLGNanomite::ParseCommandLineArgs()
{
	PTCHAR currentCommandLine = GetCommandLineW();
	QStringList splittedCommandLine = QString::fromWCharArray(currentCommandLine,wcslen(currentCommandLine)).split(" ");

	for(QStringList::const_iterator i = splittedCommandLine.constBegin(); i != splittedCommandLine.constEnd(); ++i)
	{
		if(i->compare("-p") == 0)
		{
			i++;
			if(i == splittedCommandLine.constEnd()) return;
			int PID = i->toULong();

			HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,false,PID);
			if(hProc == NULL) return;

			PTCHAR processFile = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
			if(GetModuleFileNameEx(hProc,NULL,processFile,MAX_PATH) <= 0)
			{
				CloseHandle(hProc);
				clsMemManager::CFree(processFile);
				return;
			}
			QString procFile = QString::fromWCharArray(processFile,MAX_PATH);

			CloseHandle(hProc);
			clsMemManager::CFree(processFile);
			action_DebugAttachStart(PID,procFile);
			return;
		}
		else if(i->compare("-s") == 0)
		{
			i++;
			if(i == splittedCommandLine.constEnd()) return;

			coreDebugger->SetTarget(i->toStdWString());

			for(QStringList::const_iterator commandLineSearch = splittedCommandLine.constBegin(); commandLineSearch != splittedCommandLine.constEnd(); ++commandLineSearch)
			{
				if(commandLineSearch->compare("-c") == 0)
				{
					commandLineSearch++;
					if(commandLineSearch == splittedCommandLine.constEnd()) break;

					coreDebugger->SetCommandLine(commandLineSearch->toStdWString());
				}
			}

			action_DebugStart();
			return;
		}
	}
	return;
}

void qtDLGNanomite::AskForException(DWORD exceptionCode)
{
	qtDLGExceptionAsk *newException = new qtDLGExceptionAsk(exceptionCode, this, Qt::Window);
	connect(newException,SIGNAL(ContinueException(int)),coreDebugger,SLOT(HandleForException(int)),Qt::QueuedConnection);

	newException->exec();
}

void qtDLGNanomite::LoadRecentFileMenu(bool isFirstLoad)
{
	if(!isFirstLoad)
		delete m_pRecentFiles;
	
	m_pRecentFiles = new QMenu(menuFile);
	m_pRecentFiles->setTitle("Recent Files");

	for(int i = 0; i < 5; i++)
	{
		if(m_recentDebuggedFiles.value(i).length() > 0)
			m_pRecentFiles->addAction(new QAction(m_recentDebuggedFiles.value(i),this));
	}

	menuFile->addMenu(m_pRecentFiles);
	connect(m_pRecentFiles,SIGNAL(triggered(QAction*)),this,SLOT(DebugRecentFile(QAction*)));
}

void qtDLGNanomite::DebugRecentFile(QAction *qAction)
{
	if(!coreDebugger->GetDebuggingState())
	{
		coreBPManager->BreakpointClear();
		coreDebugger->ClearTarget();
		coreDebugger->ClearCommandLine();
		coreDebugger->SetTarget(qAction->text().toStdWString());
		action_DebugStart();
	}
	else
		QMessageBox::warning(this,"Nanomite","You have a process running. Please terminate this one first!",QMessageBox::Ok,QMessageBox::Ok);
}

void qtDLGNanomite::InsertRecentDebuggedFile(QString fileName)
{
	QStringList tempFileList;

	tempFileList.append(fileName);

	for(int i = 0; i < 4; i++)
	{
		if(!m_recentDebuggedFiles.value(i).contains(fileName))
			tempFileList.append(m_recentDebuggedFiles.value(i));
	}

	m_recentDebuggedFiles = tempFileList;
}