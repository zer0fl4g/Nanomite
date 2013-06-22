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

// Taken from GDB
#define DR_EXECUTE	(0x00)		/* Break on instruction execution.  */
#define DR_WRITE	(0x01)		/* Break on data writes.			*/
#define DR_READ		(0x11)		/* Break on data reads or writes.	*/
//

#define LOGBUFFER (512 * sizeof(TCHAR))
#define LOGBUFFERCHAR (512)
#define THREAD_GETSET_CONTEXT	(0x0018) 

struct clsDebuggerSettings
{
	DWORD dwSuspendType;
	DWORD dwBreakOnEPMode;
	DWORD dwDefaultExceptionMode;
	bool bBreakOnNewDLL;
	bool bBreakOnNewPID;
	bool bBreakOnNewTID;
	bool bDebugChilds;
	bool bAutoLoadSymbols;
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

struct BPStruct
{
	DWORD dwSize;
	DWORD dwSlot;
	DWORD dwPID;
	DWORD dwTypeFlag;
	/*
	|	DR_EXECUTE
	|	DR_WRITE
	|	DR_READ
	*/
	DWORD dwHandle;
	/*
	|	0x0 - don´t keep
	|   0x1 - keep
	|   0x2 - step , remove it
	|   0x3 - offset changed
	*/
	quint64 dwOffset;
	quint64 dwBaseOffset;
	quint64 dwOldOffset;
	BYTE bOrgByte;
	bool bRestoreBP;
	PTCHAR moduleName;
};

struct customException
{
	DWORD dwExceptionType;
	DWORD dwAction;
	quint64 dwHandler;
};

class clsDebugger : public QThread
{
	Q_OBJECT

public:
	std::vector<DLLStruct> DLLs;
	std::vector<ThreadStruct> TIDs;
	std::vector<PIDStruct> PIDs;
	std::vector<BPStruct> SoftwareBPs;
	std::vector<BPStruct> MemoryBPs;
	std::vector<BPStruct> HardwareBPs;
	std::vector<customException> ExceptionHandler;

	CONTEXT ProcessContext;
	WOW64_CONTEXT wowProcessContext;

	clsDebuggerSettings dbgSettings;

	clsDebugger();
	clsDebugger(std::wstring sTarget);
	~clsDebugger();

	static bool IsOffsetAnBP(quint64 Offset);
	static bool IsOffsetEIP(quint64 Offset);

	bool StopDebuggingAll();
	bool StopDebugging(DWORD dwPID);
	bool SuspendDebuggingAll();
	bool SuspendDebugging(DWORD dwPID);
	bool ResumeDebugging();
	bool RestartDebugging();
	bool StartDebugging();
	bool GetDebuggingState();
	bool StepOver(quint64 dwNewOffset);
	bool StepIn();
	bool ShowCallStack();
	bool DetachFromProcess();
	bool AttachToProcess(DWORD dwPID);
	bool IsTargetSet();
	bool RemoveBPFromList(quint64 dwOffset,DWORD dwType); //,DWORD dwPID);
	bool RemoveBPs();
	bool AddBreakpointToList(DWORD dwBPType,DWORD dwTypeFlag,DWORD dwPID,quint64 dwOffset,DWORD dwSlot,DWORD dwKeep);
	bool SetTraceFlagForPID(DWORD dwPID, bool bIsEnabled);

	DWORD GetCurrentPID();
	DWORD GetCurrentTID();

	void ClearTarget();
	void ClearCommandLine();
	void SetTarget(std::wstring sTarget);
	void SetCommandLine(std::wstring sCommandLine);

	void CustomExceptionAdd(DWORD dwExceptionType,DWORD dwAction,quint64 dwHandler);
	void CustomExceptionRemove(DWORD dwExceptionType);
	void CustomExceptionRemoveAll();

	HANDLE GetCurrentProcessHandle();
	static HANDLE GetProcessHandleByPID(DWORD PID);

	std::wstring GetTarget();
	std::wstring GetCMDLine();

signals:
	void OnDebuggerBreak();
	void OnDebuggerTerminated();

	void OnThread(DWORD dwPID,DWORD dwTID,quint64 dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
	void OnPID(DWORD dwPID,std::wstring sFile,DWORD dwExitCode,quint64 dwEP,bool bFound);
	void OnException(std::wstring sFuncName,std::wstring sModName,quint64 dwOffset,quint64 dwExceptionCode,DWORD dwPID,DWORD dwTID);
	void OnDbgString(std::wstring sMessage,DWORD dwPID);
	void OnLog(std::wstring sLog);
	void OnDll(std::wstring sDLLPath,DWORD dwPID,quint64 dwEP,bool bLoaded);
	void OnCallStack(quint64 dwStackAddr,
		quint64 dwReturnTo,std::wstring sReturnToFunc,std::wstring sModuleName,
		quint64 dwEIP,std::wstring sFuncName,std::wstring sFuncModule,
		std::wstring sSourceFilePath,int iSourceLineNum);
	void OnNewBreakpointAdded(BPStruct newBP,int iType);
	void OnBreakpointDeleted(quint64 bpOffset);
	void OnNewPID(std::wstring,int);
	void DeletePEManagerObject(std::wstring,int);
	void CleanPEManager();
	void UpdateOffsetsPatches(HANDLE hProc, int PID);

private:
	static clsDebugger *pThis;

	PTCHAR tcLogString;
	std::wstring _sTarget;
	std::wstring _sCommandLine;

	STARTUPINFO _si;
	PROCESS_INFORMATION _pi;
	bool _isDebugging;
	bool _NormalDebugging;
	bool _bStopDebugging;
	bool _bSingleStepFlag;
	HANDLE _hDbgEvent;
	HANDLE _hCurProc;
	DWORD _dwPidToAttach;
	DWORD _dwCurPID;
	DWORD _dwCurTID;

	void DebuggingLoop();
	void AttachedDebugging(LPVOID pDebProc);
	void NormalDebugging(LPVOID pDebProc);
	void CleanWorkSpace();
	void UpdateOffsetsBPs();

	static unsigned __stdcall DebuggingEntry(LPVOID pThis);

	bool PBThreadInfo(DWORD dwPID,DWORD dwTID,quint64 dwEP,bool bSuspended,DWORD dwExitCode,BOOL bNew);
	bool PBProcInfo(DWORD dwPID,PTCHAR sFileName,quint64 dwEP,DWORD dwExitCode,HANDLE hProc);
	bool PBExceptionInfo(quint64 dwExceptionOffset,quint64 dwExceptionCode,DWORD dwPID,DWORD dwTID);
	bool PBDLLInfo(PTCHAR sDLLPath,DWORD dwPID,quint64 dwEP,bool bLoaded, int foundDLL = -1);
	bool PBLogInfo();
	bool PBDbgString(PTCHAR sMessage,DWORD dwPID);
	bool wSoftwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwKeep,DWORD dwSize,BYTE& bOrgByte);
	bool dSoftwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize,BYTE btOrgByte);
	bool wMemoryBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize,DWORD dwKeep);
	bool dMemoryBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize);
	bool wHardwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize,DWORD dwSlot,DWORD dwTypeFlag);
	bool dHardwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwSlot);
	bool InitBP();
	bool CheckProcessState(DWORD dwPID);
	bool CheckIfExceptionIsBP(quint64 dwExceptionOffset,quint64 dwExceptionType,DWORD dwPID,bool bClearTrapFlag);
	bool SuspendProcess(DWORD dwPID,bool bSuspend);
	bool SetThreadContextHelper(bool bDecIP,bool bSetTrapFlag,DWORD dwThreadID, DWORD dwPID);

	HANDLE GetCurrentProcessHandle(DWORD dwPID);

	DWORD CallBreakDebugger(DEBUG_EVENT *debug_event,DWORD dwHandle);

	PTCHAR GetFileNameFromHandle(HANDLE hFile);

	typedef DWORD (__stdcall *CustomHandler)(DEBUG_EVENT *debug_event);

protected:
	void run();
};

#endif