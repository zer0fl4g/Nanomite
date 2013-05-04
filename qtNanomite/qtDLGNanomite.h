#ifndef QTDLGNANOMITE_H
#define QTDLGNANOMITE_H

#include "qtDLGDetailInfo.h"
#include "qtDLGDebugStrings.h"
#include "qtDLGBreakPointManager.h"
#include "qtDLGSourceViewer.h"
#include "qtDLGTrace.h"
#include <qdockwidget.h>
#include <qmainwindow.h>

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

#include <QWheelEvent>

Q_DECLARE_METATYPE (DWORD)
Q_DECLARE_METATYPE (quint64)
Q_DECLARE_METATYPE (std::wstring)
Q_DECLARE_METATYPE (BPStruct)

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

	// Docks
	qtDLGRegisters	*cpuRegView;
	qtDLGCallstack	*callstackView;
	qtDLGStack		*stackView;
	qtDLGLogView	*logView;	

	QTableWidget	*tblLogBox;
	QTableWidget	*tblCallstack;
	QTableWidget	*tblStack;
	QScrollBar		*scrollStackView;
	QTableWidget	*tblRegView;

	qtNanomiteDisAsColorSettings *qtNanomiteDisAsColor;

	qtDLGNanomite(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGNanomite();

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
	void OnDisplayDisassembly(quint64 dwEIP);
	void OnDisAsScroll(int iValue);
	void OnStackScroll(int iValue);
	void OnRegViewChangeRequest(QTableWidgetItem *pItem);
	void OnDebuggerBreak();
	void OnDebuggerTerminated();
	void OnCustomRegViewContextMenu(QPoint qPoint);
	void OnCustomCallstackContextMenu(QPoint qPoint);
	void OnCustomDisassemblerContextMenu(QPoint qPoint);
	void OnCustomLogContextMenu(QPoint qPoint);
	void OnDisAsReturnPressed();
	void OnDisAsReturn();
	void OnCallstackDisplaySource(QTableWidgetItem *pItem);
	void LoadRegView();
	void GenerateMenuCallback(QAction *qAction);
	void MenuCallback(QAction*);

signals:
	void OnDisplaySource(QString,int);

private:
	int _iMenuPID;
	int _iSelectedRow;
	int _iSelectedAction;
	/*
	0 = from Disassembler
	1 = from RegView
	2 = from Callstack
	3 = from StackView
	4 = from LogBox
	*/

	QList<quint64> _OffsetWalkHistory;

	static qtDLGNanomite *qtDLGMyWindow;
	


	void resizeEvent(QResizeEvent *event);
	void InitListSizes();
	void CleanGUI(bool bKeepLogBox = false);
	void GenerateMenu(bool isAllEnabled = true);
	void UpdateStateBar(DWORD dwAction);
	void LoadStackView(quint64 dwESP, DWORD dwStackSize);
	void LoadWidgets();

protected:
	bool eventFilter(QObject *pOpject,QEvent *event);
	void dragEnterEvent(QDragEnterEvent* pEvent);
	void dropEvent(QDropEvent* pEvent);
	void closeEvent(QCloseEvent* closeEvent);
};

#endif // QTDLGNANOMITE_H
