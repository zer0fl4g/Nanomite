#ifndef QTDLGNANOMITE_H
#define QTDLGNANOMITE_H

#include "qtDLGDetailInfo.h"
#include "qtDLGDebugStrings.h"
#include "qtDLGBreakPointManager.h"

#include "clsDisassambler.h"
#include "clsDebugger/clsDebugger.h"

#include "ui_qtDLGNanomite.h"

Q_DECLARE_METATYPE (DWORD)
Q_DECLARE_METATYPE (DWORD64)
Q_DECLARE_METATYPE (std::wstring)

class qtDLGNanomite : public QMainWindow, public Ui_qtDLGNanomiteClass
{
	Q_OBJECT

public:
	struct DisAsDataRow
	{
		QString Offset;
		QString OpCodes;
		QString ASM;
		QString Comment;
	};

	static qtDLGNanomite* GetInstance();

	long lExceptionCount;
	clsDebugger *coreDebugger;
	clsDisassambler *coreDisAs;
	qtDLGDetailInfo *dlgDetInfo;
	qtDLGDebugStrings *dlgDbgStr;
	qtDLGBreakPointManager *dlgBPManager;

	qtDLGNanomite(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGNanomite();

private slots:
	void action_FileOpenNewFile();
	void action_FileAttachTo();
	void action_FileDetach();
	void action_FileTerminateGUI();
	void action_DebugStart();
	void action_DebugAttachStart(int iPID);
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

	void OnDisplayDisassambly(DWORD64 dwEIP);
	void OnDisAsScroll(int iValue);
	void OnStackScroll(int iValue);
	void OnDebuggerBreak();
	void OnDebuggerTerminated();

	void GenerateMenuCallback(QAction *qAction);

private:
	DWORD _iMenuPID;

	static qtDLGNanomite *qtDLGMyWindow;
	
	void InitListSizes();
	void CleanGUI();
	void GenerateMenu();
	void UpdateStateBar(DWORD dwAction);
	void LoadRegView();
	void LoadStackView(DWORD64 dwESP, DWORD dwStackSize);	
};

#endif // QTDLGNANOMITE_H
