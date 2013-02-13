#ifndef QTDLGNANOMITE_H
#define QTDLGNANOMITE_H

#include "qtDLGDetailInfo.h"
#include "qtDLGDebugStrings.h"
#include "qtDLGBreakPointManager.h"

#include "clsDisassembler.h"
#include "clsDebugger/clsDebugger.h"
#include "clsPEManager.h"

#include "ui_qtDLGNanomite.h"

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
	qtDLGDetailInfo *dlgDetInfo;
	qtDLGDebugStrings *dlgDbgStr;
	qtDLGBreakPointManager *dlgBPManager;
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

	void OnF2BreakPointPlace();
	void OnDisplayDisassembly(quint64 dwEIP);
	void OnDisAsScroll(int iValue);
	void OnStackScroll(int iValue);
	void OnRegViewChangeRequest(QTableWidgetItem *pItem);
	void OnDebuggerBreak();
	void OnDebuggerTerminated();
	void OnCustomRegViewContextMenu(QPoint qPoint);
	void OnCustomDisassemblerContextMenu(QPoint qPoint);
	void OnPrintMemoryLeaks();

	void LoadRegView();
	void GenerateMenuCallback(QAction *qAction);
	void MenuCallback(QAction*);
	
private:
	int _iMenuPID;
	int _iSelectedRow;

	static qtDLGNanomite *qtDLGMyWindow;
	
	void resizeEvent(QResizeEvent *event);
	void InitListSizes();
	void CleanGUI();
	void GenerateMenu();
	void UpdateStateBar(DWORD dwAction);
	void LoadStackView(quint64 dwESP, DWORD dwStackSize);	
};

#endif // QTDLGNANOMITE_H
