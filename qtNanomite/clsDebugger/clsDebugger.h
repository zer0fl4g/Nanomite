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
#ifndef CLSDEBUGGER
#define CLSDEBUGGER

#include <string>
#include <vector>
#include <Windows.h>
#include <time.h>
#include <QtCore>

#include "../clsBreakpointManager.h"

#define LOGBUFFER (512 * sizeof(TCHAR))
#define LOGBUFFERCHAR (512)
#define THREAD_GETSET_CONTEXT	(0x0018) 

struct clsDebuggerSettings
{
	DWORD dwSuspendType;
	DWORD dwDefaultExceptionMode;
	bool bBreakOnNewDLL;
	bool bBreakOnNewPID;
	bool bBreakOnNewTID;
	bool bBreakOnExDLL;
	bool bBreakOnExPID;
	bool bBreakOnExTID;
	bool bBreakOnModuleEP;
	bool bBreakOnSystemEP;
	bool bBreakOnTLS;
	bool bDebugChilds;
	bool bAutoLoadSymbols;
	bool bUseExceptionAssist;
};

struct DLLStruct
{
	PTCHAR sPath;
	DWORD dwPID;
	quint64 dwBaseAdr;
	bool bLoaded;
};

struct ThreadStruct
{
	DWORD dwTID;
	DWORD dwPID;
	DWORD dwExitCode;
	quint64 dwEP;
	bool bSuspended;
};

struct PIDStruct
{
	DWORD dwPID;
	DWORD dwExitCode;
	DWORD dwBPRestoreFlag;
	HANDLE hProc;
	PTCHAR sFileName;
	quint64 dwEP;
	bool bKernelBP;
	bool bWOW64KernelBP;
	bool bRunning;
	bool bSymLoad;
	bool bTrapFlag;
	bool bTraceFlag;
};

struct customException
{
	DWORD dwAction;
	DWORD dwExceptionType;
	quint64 dwHandler;
};

enum BREAKPOINT
{
	SOFTWARE_BP = 0,
	MEMORY_BP	= 1,
	HARDWARE_BP = 2
};

enum BREAKPOINT_TYPE
{
	BP_DONOTKEEP	= 0,
	BP_KEEP			= 1,
	BP_STEPOVER		= 2,
	BP_OFFSETUPDATE = 3,
	BP_TRACETO		= 4
};

enum BP_BREAKON
{
	BP_EXEC		= 0x00,
	BP_WRITE	= 0x01,
	BP_ACCESS	= 0x10,
	BP_READ		= 0x11	
};

enum RESTORE_BP
{
	RESTORE_NON			= 0,
	RESTORE_BP_SOFTWARE	= 1,
	RESTORE_BP_MEMORY	= 2,
	RESTORE_BP_HARDWARE	= 3
};

class clsDebugger : public QThread
{
	Q_OBJECT

public:
	std::vector<DLLStruct> DLLs;
	std::vector<ThreadStruct> TIDs;
	std::vector<PIDStruct> PIDs;
	std::vector<customException> ExceptionHandler;

	CONTEXT ProcessContext;
	WOW64_CONTEXT wowProcessContext;

	clsDebuggerSettings dbgSettings;

	clsDebugger(clsBreakpointManager *pBPManager);
	~clsDebugger();

	static bool IsOffsetAnBP(quint64 Offset);
	static bool IsOffsetEIP(quint64 Offset);

	static void SetNewThreadContext(bool isWow64, CONTEXT newProcessContext, WOW64_CONTEXT newWowProcessContext);

	static HANDLE GetProcessHandleByPID(DWORD PID);

	bool StopDebuggingAll();
	bool StopDebugging(DWORD dwPID);
	bool SuspendDebuggingAll();
	bool SuspendDebugging(DWORD dwPID);
	bool ResumeDebugging();
	bool GetDebuggingState();
	bool StepOver(quint64 dwNewOffset);
	bool StepIn();
	bool DetachFromProcess();
	bool AttachToProcess(DWORD dwPID);
	bool IsTargetSet();
	bool SetTraceFlagForPID(DWORD dwPID, bool bIsEnabled);

	static DWORD GetCurrentPID();
	static DWORD GetCurrentTID();
		
	void ClearTarget();
	void ClearCommandLine();
	void SetTarget(std::wstring sTarget);
	void SetCommandLine(std::wstring sCommandLine);
	void CustomExceptionAdd(DWORD dwExceptionType,DWORD dwAction,quint64 dwHandler);
	void CustomExceptionRemove(DWORD dwExceptionType);
	void CustomExceptionRemoveAll();

	HANDLE GetCurrentProcessHandle();

	std::wstring GetTarget();
	std::wstring GetCMDLine();

public slots:
	void HandleForException(int handleException);

signals:
	void OnDebuggerBreak();
	void OnDebuggerTerminated();
	void AskForException(DWORD exceptionCode);
	void OnThread(DWORD dwPID,DWORD dwTID,quint64 dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
	void OnPID(DWORD dwPID,std::wstring sFile,DWORD dwExitCode,quint64 dwEP,bool bFound);
	void OnException(std::wstring sFuncName,std::wstring sModName,quint64 dwOffset,quint64 dwExceptionCode,DWORD dwPID,DWORD dwTID);
	void OnDbgString(std::wstring sMessage,DWORD dwPID);
	void OnLog(std::wstring sLog);
	void OnDll(std::wstring sDLLPath,DWORD dwPID,quint64 dwEP,bool bLoaded);
	void OnNewPID(std::wstring,int);
	void DeletePEManagerObject(std::wstring,int);
	void CleanPEManager();
	void UpdateOffsetsPatches(HANDLE hProc, int PID);

private:
	static clsDebugger *pThis;

	clsBreakpointManager *m_pBreakpointManager;

	PTCHAR tcLogString;
	std::wstring _sTarget;
	std::wstring _sCommandLine;

	STARTUPINFO _si;
	PROCESS_INFORMATION _pi;
	PROCESS_INFORMATION m_dbgPI;
	bool _isDebugging;
	bool _NormalDebugging;
	bool _bStopDebugging;
	bool _bSingleStepFlag;
	bool m_debuggerBreak;
	HANDLE _hDbgEvent;
	HANDLE _hCurProc;
	HANDLE m_waitForGUI;
	DWORD _dwPidToAttach;
	DWORD _dwCurPID;
	DWORD _dwCurTID;
	 
	int m_continueWithException;

	void DebuggingLoop();
	void AttachedDebugging(LPVOID pDebProc);
	void NormalDebugging(LPVOID pDebProc);
	void CleanWorkSpace();
	
	bool PBThreadInfo(DWORD dwPID,DWORD dwTID,quint64 dwEP,bool bSuspended,DWORD dwExitCode,BOOL bNew);
	bool PBProcInfo(DWORD dwPID,PTCHAR sFileName,quint64 dwEP,DWORD dwExitCode,HANDLE hProc);
	bool PBExceptionInfo(quint64 dwExceptionOffset,quint64 dwExceptionCode,DWORD dwPID,DWORD dwTID);
	bool PBDLLInfo(PTCHAR sDLLPath,DWORD dwPID,quint64 dwEP,bool bLoaded, DLLStruct *pFoundDLL = NULL);
	bool PBLogInfo();
	bool PBDbgString(PTCHAR sMessage,DWORD dwPID);
	bool CheckProcessState(DWORD dwPID);
	bool CheckIfExceptionIsBP(PIDStruct *pCurrentPID, quint64 dwExceptionOffset, quint64 dwExceptionType, bool bClearTrapFlag, bool isExceptionRelevant = true);
	bool SuspendProcess(DWORD dwPID,bool bSuspend);
	bool SetThreadContextHelper(bool bDecIP,bool bSetTrapFlag,DWORD dwThreadID, PIDStruct *pCurrentPID);
	bool IsDebuggerSuspended();

	HANDLE GetCurrentProcessHandle(DWORD dwPID);

	DWORD CallBreakDebugger(DEBUG_EVENT *debug_event,DWORD dwHandle);
	DWORD GetMainProcessID();
	DWORD GetMainThreadID();

	PTCHAR GetFileNameFromModuleBase(HANDLE processHandle, LPVOID imageBase);

	PIDStruct* GetCurrentPIDDataPointer(DWORD processID);

	typedef DWORD (__stdcall *CustomHandler)(DEBUG_EVENT *debug_event);

protected:
	void run();
};

#endif