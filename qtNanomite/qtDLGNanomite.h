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
#ifndef QTDLGNANOMITE_H
#define QTDLGNANOMITE_H

#include "qtDLGDetailInfo.h"
#include "qtDLGDebugStrings.h"
#include "qtDLGBreakPointManager.h"
#include "qtDLGSourceViewer.h"
#include "qtDLGTrace.h"
#include "qtDLGPatchManager.h"

// Dock Widget Classes
#include "qtDLGRegisters.h"
#include "qtDLGCallstack.h"
#include "qtDLGStack.h"
#include "qtDLGLogView.h"

#include "clsDisassembler.h"
#include "clsDebugger/clsDebugger.h"
#include "clsPEManager.h"
#include "clsDBManager.h"

#include "ui_qtDLGNanomite.h"

#include <QDockwidget>
#include <QMainwindow>
#include <QWheelEvent>

Q_DECLARE_METATYPE (DWORD)
Q_DECLARE_METATYPE (quint64)
Q_DECLARE_METATYPE (std::wstring)
Q_DECLARE_METATYPE (BPStruct)
Q_DECLARE_METATYPE (HANDLE)

struct qtNanomiteDisAsColorSettings
{ 
	QString colorBP;
	QString colorCall;
	QString colorMove;
	QString colorJump;
	QString colorStack;
	QString colorMath;
};

class qtDLGNanomite : public QMainWindow, public Ui_qtDLGNanomiteClass
{
	Q_OBJECT

public:
	static qtDLGNanomite* GetInstance();

	long lExceptionCount;
	clsDebugger *coreDebugger;
	clsDisassembler *coreDisAs;
	clsPEManager *PEManager;
	clsDBManager *DBManager;

	qtDLGDetailInfo *dlgDetInfo;
	qtDLGDebugStrings *dlgDbgStr;
	qtDLGBreakPointManager *dlgBPManager;
	qtDLGSourceViewer *dlgSourceViewer;
	qtDLGTrace *dlgTraceWindow;
	qtDLGPatchManager *dlgPatchManager;

	// Docks
	qtDLGRegisters	*cpuRegView;
	qtDLGCallstack	*callstackView;
	qtDLGStack		*stackView;
	qtDLGLogView	*logView;	

	qtNanomiteDisAsColorSettings *qtNanomiteDisAsColor;

	qtDLGNanomite(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGNanomite();

public slots:
	void OnDisplayDisassembly(quint64 dwEIP);

private slots:
	void action_FileOpenNewFile();
	void action_FileAttachTo();
	void action_FileDetach();
	void action_FileTerminateGUI();
	void action_DebugStart();
	void action_DebugAttachStart(int iPID,QString FilePath);
	void action_DebugStop();
	void action_DebugRestart();
	void action_DebugSuspend();
	void action_DebugStepIn();
	void action_DebugStepOver();
	void action_DebugStepOut();
	void action_DebugRunToUserCode();
	void action_OptionsAbout();
	void action_OptionsOptions();
	void action_WindowDetailInformation();
	void action_WindowBreakpointManager();
	void action_WindowPatches();
	void action_WindowShowMemory();
	void action_WindowShowHeap();
	void action_WindowShowStrings();
	void action_WindowShowDebugOutput();
	void action_WindowShowHandles();
	void action_WindowShowWindows();
	void action_WindowShowPEEditor();
	void action_WindowShowFunctions();
	void action_DebugTraceStart();
	void action_DebugTraceStop();
	void action_DebugTraceShow();

	void OnF2BreakPointPlace();
	void OnDisAsScroll(int iValue);
	void OnDebuggerBreak();
	void OnDebuggerTerminated();
	void OnCustomDisassemblerContextMenu(QPoint qPoint);
	void OnDisAsReturnPressed();
	void OnDisAsReturn();
	void GenerateMenuCallback(QAction *qAction);
	void CustomDisassemblerMenuCallback(QAction*);

private:
	int _iMenuPID;
	int _iSelectedRow;

	bool m_IsRestart;

	QList<quint64> _OffsetWalkHistory;

	static qtDLGNanomite *qtDLGMyWindow;

	void resizeEvent(QResizeEvent *event);
	void CleanGUI(bool bKeepLogBox = false);
	void GenerateMenu(bool isAllEnabled = true);
	void UpdateStateBar(DWORD dwAction);
	void LoadWidgets();
	void ParseCommandLineArgs();

protected:
	bool eventFilter(QObject *pOpject,QEvent *event);
	void dragEnterEvent(QDragEnterEvent* pEvent);
	void dropEvent(QDropEvent* pEvent);
	void closeEvent(QCloseEvent* closeEvent);
};

#endif // QTDLGNANOMITE_H
