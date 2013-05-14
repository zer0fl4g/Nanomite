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

#include "qtDLGNanomite.h"
#include "qtDLGRegEdit.h"
#include "qtDLGAssembler.h"

#include "clsCallbacks.h"
#include "clsHelperClass.h"
#include "clsDisassembler.h"
#include "clsAPIImport.h"
#include "clsSymbolAndSyntax.h"
#include "clsDBInterface.h"

#include "clsMemManager.h"
#include "clsAppSettings.h"

#include <QClipboard>

using namespace std;

qtDLGNanomite* qtDLGNanomite::qtDLGMyWindow = NULL;

qtDLGNanomite::qtDLGNanomite(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	setupUi(this);

	setAcceptDrops(true);

	QApplication::setStyle(new QPlastiqueStyle);
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());

	qRegisterMetaType<DWORD>("DWORD");
	qRegisterMetaType<quint64>("quint64");
	qRegisterMetaType<wstring>("wstring");
	qRegisterMetaType<BPStruct>("BPStruct");
	qRegisterMetaType<HANDLE>("HANDLE");

	LoadWidgets();
	
	clsAPIImport::LoadFunctions();

	coreDebugger = new clsDebugger;
	coreDisAs = new clsDisassembler;
	PEManager = new clsPEManager;
	DBManager = new clsDBManager;
	clsCallbacks *NanomiteCallbacks = new clsCallbacks;
	dlgDetInfo = new qtDLGDetailInfo(this,Qt::Window);
	dlgDbgStr = new qtDLGDebugStrings(this,Qt::Window);
	dlgBPManager = new qtDLGBreakPointManager(this,Qt::Window);
	dlgSourceViewer = new qtDLGSourceViewer(this,Qt::Window);
	dlgTraceWindow = new qtDLGTrace(this,Qt::Window);
	dlgPatchManager = new qtDLGPatchManager(this,Qt::Window);
	qtNanomiteDisAsColor = new qtNanomiteDisAsColorSettings;	

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

	connect(coreDebugger,SIGNAL(OnLog(std::wstring)),
		logView,SLOT(OnLog(std::wstring)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnCallStack(quint64,quint64,std::wstring,std::wstring,quint64,std::wstring,std::wstring,std::wstring,int)),
		callstackView,SLOT(OnCallStack(quint64,quint64,std::wstring,std::wstring,quint64,std::wstring,std::wstring,std::wstring,int)),Qt::QueuedConnection);

	connect(coreDebugger,SIGNAL(OnDebuggerBreak()),this,SLOT(OnDebuggerBreak()),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDebuggerTerminated()),this,SLOT(OnDebuggerTerminated()),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnNewBreakpointAdded(BPStruct,int)),dlgBPManager,SLOT(OnUpdate(BPStruct,int)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnBreakpointDeleted(quint64)),dlgBPManager,SLOT(OnDelete(quint64)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnNewPID(std::wstring,int)),dlgBPManager,SLOT(UpdateCompleter(std::wstring,int)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(UpdateOffsetsPatches(HANDLE,int)),dlgPatchManager,SLOT(UpdateOffsetPatch(HANDLE,int)),Qt::QueuedConnection);

	// Callbacks from DetailInfo to PEManager
	connect(dlgDetInfo,SIGNAL(OpenFileInPEManager(std::wstring,int)),PEManager,SLOT(InsertPIDForFile(std::wstring,int)));
	// Callbacks from DetailView to GUI
	connect(dlgDetInfo,SIGNAL(ShowInDisassembler(quint64)),this,SLOT(OnDisplayDisassembly(quint64)));

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
	connect(actionDebug_Step_Out, SIGNAL(triggered()), this, SLOT(action_DebugStepOut()));
	connect(actionDebug_Step_Over, SIGNAL(triggered()), this, SLOT(action_DebugStepOver()));
	connect(actionOptions_About, SIGNAL(triggered()), this, SLOT(action_OptionsAbout()));
	connect(actionOptions_Options, SIGNAL(triggered()), this, SLOT(action_OptionsOptions()));
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
	connect(action_Debug_Run_to_UserCode,SIGNAL(triggered()), this, SLOT(action_DebugRunToUserCode()));
	connect(actionDebug_Trace_Start, SIGNAL(triggered()), this, SLOT(action_DebugTraceStart()));
	connect(actionDebug_Trace_Stop, SIGNAL(triggered()), this, SLOT(action_DebugTraceStop()));
	connect(actionDebug_Trace_Show, SIGNAL(triggered()), this, SLOT(action_DebugTraceShow()));
	connect(actionWindow_Show_PEEditor, SIGNAL(triggered()), this, SLOT(action_WindowShowPEEditor()));

	// Actions on Window Events
	connect(tblDisAs,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomDisassemblerContextMenu(QPoint)));
	connect(scrollDisAs,SIGNAL(valueChanged(int)),this,SLOT(OnDisAsScroll(int)));
	connect(callstackView,SIGNAL(DisplaySource(QString,int)),dlgSourceViewer,SLOT(OnDisplaySource(QString,int)));

	// GUI Shortcuts
	connect(new QShortcut(QKeySequence("F2"),this),SIGNAL(activated()),this,SLOT(OnF2BreakPointPlace()));
	connect(new QShortcut(QKeySequence::InsertParagraphSeparator,this),SIGNAL(activated()),this,SLOT(OnDisAsReturnPressed()));
	connect(new QShortcut(QKeySequence("Backspace"),this),SIGNAL(activated()),this,SLOT(OnDisAsReturn()));

	// Callbacks from Debugger to PEManager
	connect(coreDebugger,SIGNAL(OnNewPID(std::wstring,int)),PEManager,SLOT(InsertPIDForFile(std::wstring,int)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(DeletePEManagerObject(std::wstring,int)),PEManager,SLOT(CloseFile(std::wstring,int)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(CleanPEManager()),PEManager,SLOT(CleanPEManager()),Qt::QueuedConnection);

	// Callbacks from TraceView to GUI
	connect(dlgTraceWindow,SIGNAL(OnDisplayDisassembly(quint64)),this,SLOT(OnDisplayDisassembly(quint64)));

	// Callbacks from PatchManager to GUI
	connect(dlgPatchManager,SIGNAL(OnReloadDebugger()),this,SLOT(OnDebuggerBreak()));
	connect(dlgPatchManager,SIGNAL(OnShowInDisassembler(quint64)),this,SLOT(OnDisplayDisassembly(quint64)));

	// eventFilter for mouse scroll
	tblDisAs->installEventFilter(this);
    tblDisAs->viewport()->installEventFilter(this);
	
	// List DisAs
	tblDisAs->horizontalHeader()->resizeSection(0,135);
	tblDisAs->horizontalHeader()->resizeSection(1,250);
	tblDisAs->horizontalHeader()->resizeSection(2,310);
	tblDisAs->horizontalHeader()->resizeSection(3,310);

	actionDebug_Trace_Stop->setDisabled(true);
}

qtDLGNanomite::~qtDLGNanomite()
{
	delete coreDebugger;
	delete coreDisAs;
	delete PEManager;
	delete DBManager;
	delete dlgDetInfo;
	delete dlgDbgStr;
	delete dlgBPManager;
	delete dlgSourceViewer;
	delete dlgTraceWindow;
	delete dlgPatchManager;
	delete qtNanomiteDisAsColor;
	delete cpuRegView;
	delete callstackView;
	delete stackView;
	delete logView;
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

	this->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, this->cpuRegView);
	this->addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, this->callstackView);

	this->addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, this->stackView);
	this->addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, this->logView);

	clsAppSettings *settings = clsAppSettings::SharedInstance();

	if (!settings->RestoreWindowState(this))
	{
		this->splitDockWidget(this->callstackView, this->stackView, Qt::Orientation::Vertical);
		this->splitDockWidget(this->stackView, this->logView, Qt::Orientation::Horizontal);
	}
}

void qtDLGNanomite::OnDebuggerBreak()
{
	quint64 dwEIP = NULL;
	if(!coreDebugger->GetDebuggingState())
		UpdateStateBar(0x3);
	else
	{
		// display callstack
		callstackView->tblCallstack->setRowCount(0);
		coreDebugger->ShowCallStack();

		// display Reg
		cpuRegView->LoadRegView(coreDebugger);

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
			dlgSourceViewer->OnDisplaySource(QString::fromStdWString(FilePath),LineNumber);	

		// Update Disassembler
		if(!coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwEIP))
			OnDisplayDisassembly(dwEIP);

		// Update Toolbar
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

void qtDLGNanomite::CleanGUI(bool bKeepLogBox)
{
	stackView->tblStack->setRowCount(0);
	if(!bKeepLogBox)
		logView->tblLogBox->setRowCount(0);
	callstackView->tblCallstack->setRowCount(0);
	tblDisAs->setRowCount(0);
	cpuRegView->tblRegView->setRowCount(0);

	dlgDetInfo->tblPIDs->setRowCount(0);
	dlgDetInfo->tblTIDs->setRowCount(0);
	dlgDetInfo->tblExceptions->setRowCount(0);
	dlgDetInfo->tblModules->setRowCount(0);

	dlgTraceWindow->tblTraceLog->setRowCount(0);

	dlgSourceViewer->listSource->clear();

	dlgDbgStr->tblDebugStrings->setRowCount(0);

	lExceptionCount = 0;
}

void qtDLGNanomite::OnDebuggerTerminated()
{
	coreDisAs->SectionDisAs.clear();
	dlgBPManager->DeleteCompleterContent();
	qtDLGTrace::clearTraceData();
	actionDebug_Trace_Start->setEnabled(true);
	CleanGUI(true);
	this->setWindowTitle(QString("[Nanomite v 0.1] - MainWindow"));
	UpdateStateBar(0x3);
}

void qtDLGNanomite::GenerateMenuCallback(QAction *qAction)
{
	_iMenuPID = qAction->text().toULong(0,16);
}

void qtDLGNanomite::GenerateMenu(bool isAllEnabled)
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
		qAction = new QAction("All",this);
	
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(GenerateMenuCallback(QAction*)));
	menu.addAction(qAction);
	menu.exec(QCursor::pos());
}

void qtDLGNanomite::OnDisplayDisassembly(quint64 dwEIP)
{
	if(coreDisAs->SectionDisAs.count() > 0 && dwEIP != 0)
	{
		bool IsAlreadyEIPSet = false;
		int iLines = 0;

		QMap<QString,DisAsDataRow>::const_iterator i = coreDisAs->SectionDisAs.constFind(QString("%1").arg(dwEIP,16,16,QChar('0')).toUpper());
		QMap<QString,DisAsDataRow>::const_iterator iEnd = coreDisAs->SectionDisAs.constEnd();--iEnd;

		if((QMapData::Node *)i == (QMapData::Node *)coreDisAs->SectionDisAs.constEnd())
		{
			coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwEIP);
			return;
		}

		if((QMapData::Node *)i != (QMapData::Node *)coreDisAs->SectionDisAs.constBegin())
		{
			for(int iBack = 0; iBack <= 5; iBack++)
			{
				if(!i.value().Offset.isEmpty() && i.value().Offset.compare(coreDisAs->SectionDisAs.begin().value().Offset) == 0)
				{
					coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),i.value().Offset.toULongLong(0,16));
					return;
				}			
				--i;
			}
		}

		tblDisAs->setRowCount(0);

		quint64 itemStyle;
		while(iLines <= ((tblDisAs->verticalHeader()->height() + 4) / 11) - 1)
		{
			itemStyle = i.value().itemStyle;
			tblDisAs->insertRow(tblDisAs->rowCount());

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 0,new QTableWidgetItem(i.value().Offset));
			if(clsDebugger::IsOffsetAnBP(i.value().Offset.toULongLong(0,16)))
				tblDisAs->item(tblDisAs->rowCount() - 1,0)->setForeground(QColor(qtNanomiteDisAsColor->colorBP));

			if(!IsAlreadyEIPSet && clsDebugger::IsOffsetEIP(i.value().Offset.toULongLong(0,16)))
			{
				tblDisAs->item(tblDisAs->rowCount() - 1,0)->setBackground(QColor("Magenta"));
				IsAlreadyEIPSet = true;
			}

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 1,new QTableWidgetItem(i.value().OpCodes));

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 2,new QTableWidgetItem(i.value().ASM));
			if(itemStyle & COLOR_CALLS)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorCall));
			else if(itemStyle & COLOR_JUMP)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorJump));
			else if(itemStyle & COLOR_MOVE)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorMove));
			else if(itemStyle & COLOR_STACK)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorStack));
			else if(itemStyle & COLOR_MATH)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorMath));

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 3,new QTableWidgetItem(i.value().Comment));

			if(!i.value().Offset.isEmpty() && i.value().Offset.compare(iEnd.value().Offset) == 0)
			{
				coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),tblDisAs->item(4,0)->text().toULongLong(0,16),true);
				return;
			}

			++iLines;++i;
		}

		// Update Window Title
		wstring ModName,FuncName;
		clsHelperClass::LoadSymbolForAddr(FuncName,ModName,dwEIP,coreDebugger->GetCurrentProcessHandle());
		this->setWindowTitle(QString("[Nanomite v 0.1] - MainWindow -[PID: %1 - TID: %2]- %3.%4").arg(coreDebugger->GetCurrentPID(),6,16,QChar('0')).arg(coreDebugger->GetCurrentTID(),6,16,QChar('0')).arg(QString().fromStdWString(ModName),QString().fromStdWString(FuncName)));
	}
	else
	{
		coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwEIP);
	}
}

void qtDLGNanomite::OnCustomDisassemblerContextMenu(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblDisAs->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;


	menu.addAction(new QAction("Goto Offset",this));
	menu.addAction(new QAction("Edit Instruction",this));
	menu.addAction(new QAction("Show Source",this));
	menu.addAction(new QAction("Set R/EIP to this",this));
	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Offset",this));
	submenu->addAction(new QAction("OpCodes",this));
	submenu->addAction(new QAction("Mnemonic",this));
	submenu->addAction(new QAction("Comment",this));

	connect(submenu,SIGNAL(triggered(QAction*)),this,SLOT(CustomDisassemblerMenuCallback(QAction*)));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(CustomDisassemblerMenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGNanomite::CustomDisassemblerMenuCallback(QAction* pAction)
{
	if(!coreDebugger->GetDebuggingState()) return;

	if(QString().compare(pAction->text(),"Set R/EIP to this") == 0)
	{
#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		if(clsAPIImport::pIsWow64Process)
			clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

		if(bIsWOW64)
			coreDebugger->wowProcessContext.Eip = tblDisAs->item(_iSelectedRow,0)->text().toULongLong(0,16);
		else
			coreDebugger->ProcessContext.Rip = tblDisAs->item(_iSelectedRow,0)->text().toULongLong(0,16);
#else
		coreDebugger->ProcessContext.Eip = tblDisAs->item(_iSelectedRow,0)->text().toULongLong(0,16);
#endif
		OnDebuggerBreak();
	}
	else if(QString().compare(pAction->text(),"Edit Instruction") == 0)
	{
		qtDLGAssembler *dlgAssembler = new qtDLGAssembler(this,Qt::Window,coreDebugger->GetCurrentProcessHandle(),tblDisAs->item(_iSelectedRow,0)->text().toULongLong(0,16),coreDisAs,PEManager->is64BitFile(L"\\\\",coreDebugger->GetCurrentPID()));
		connect(dlgAssembler,SIGNAL(OnReloadDebugger()),this,SLOT(OnDebuggerBreak()));
		dlgAssembler->show();
	}
	else if(QString().compare(pAction->text(),"Goto Offset") == 0)
	{
		bool bOk = false;
		QString strNewOffset = QInputDialog::getText(this,"Please give a Offset:","VA:",QLineEdit::Normal,NULL,&bOk);

		if(bOk && !strNewOffset.isEmpty())
			if(!coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),strNewOffset.toULongLong(0,16)))
				OnDisplayDisassembly(strNewOffset.toULongLong(0,16));		
	}
	else if(QString().compare(pAction->text(),"Show Source") == 0)
	{
		if(dlgSourceViewer->IsSourceAvailable)
			dlgSourceViewer->show();
		else
			MessageBoxW(NULL,L"Sorry, there is no source available!",L"Nanomite",MB_OK);
	}
	else if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3:%4")
			.arg(tblDisAs->item(_iSelectedRow,0)->text())
			.arg(tblDisAs->item(_iSelectedRow,1)->text())
			.arg(tblDisAs->item(_iSelectedRow,2)->text())
			.arg(tblDisAs->item(_iSelectedRow,3)->text()));
	}
	else if(QString().compare(pAction->text(),"Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblDisAs->item(_iSelectedRow,0)->text());
	}
	else if(QString().compare(pAction->text(),"OpCodes") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblDisAs->item(_iSelectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"Mnemonic") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblDisAs->item(_iSelectedRow,2)->text());
	}
	else if(QString().compare(pAction->text(),"Comment") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblDisAs->item(_iSelectedRow,3)->text());
	}
}

void qtDLGNanomite::resizeEvent(QResizeEvent *event)
{
	OnDebuggerBreak();
}

void qtDLGNanomite::OnF2BreakPointPlace()
{
	QList<QTableWidgetItem *> currentSelectedItems = tblDisAs->selectedItems();
	if(currentSelectedItems.count() <= 0) return;

	quint64 dwSelectedVA = currentSelectedItems.value(0)->text().toULongLong(0,16);
	if(coreDebugger->AddBreakpointToList(NULL,DR_EXECUTE,-1,dwSelectedVA,NULL,true))
		currentSelectedItems.value(0)->setForeground(QColor(qtNanomiteDisAsColor->colorBP));
	else
	{// exists
		coreDebugger->RemoveBPFromList(dwSelectedVA,NULL);
		currentSelectedItems.value(0)->setForeground(QColor("Black"));
	}	
	return;
}

void qtDLGNanomite::OnDisAsReturnPressed()
{
	QList<QTableWidgetItem *> currentSelectedItems = tblDisAs->selectedItems();
	if(coreDebugger->GetCurrentProcessHandle() == NULL || currentSelectedItems.count() <= 0) return;

	quint64 dwSelectedVA = NULL;
	QString tempSelectedString = currentSelectedItems.value(2)->text();

	if(tempSelectedString.contains("ptr") || tempSelectedString.contains("ptr"))
		dwSelectedVA = tempSelectedString.split(" ")[3].replace("h","").replace("[","").replace("]","").toULongLong(0,16);
	else
		dwSelectedVA = tempSelectedString.split(" ")[1].replace("h","").toULongLong(0,16);

	if(dwSelectedVA != 0)
	{
		_OffsetWalkHistory.append(currentSelectedItems.value(0)->text().toULongLong(0,16));
		coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwSelectedVA);
	}
	return;
}

void qtDLGNanomite::OnDisAsReturn()
{
	if(_OffsetWalkHistory.isEmpty()) return;
	coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),_OffsetWalkHistory.takeLast());
}

bool qtDLGNanomite::eventFilter(QObject *pObject, QEvent *event)
{	
	if(event->type() == QEvent::Wheel && pObject == tblDisAs)
	{
		QWheelEvent *pWheel = (QWheelEvent*)event;
		
		OnDisAsScroll(pWheel->delta() * -1);
			return true;
	}
	return false;
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
		wstring* filePath = new wstring(QString(pEvent->mimeData()->data("FileName")).toStdWString());
		coreDebugger->SetTarget(*filePath);
		action_DebugStart();
		pEvent->acceptProposedAction();
    }
}

void qtDLGNanomite::closeEvent(QCloseEvent* closeEvent)
{
	clsAppSettings::SharedInstance()->SaveWindowState(this);

	closeEvent->accept();
}
