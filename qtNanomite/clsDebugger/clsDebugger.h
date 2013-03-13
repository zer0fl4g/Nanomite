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
	bool bDebugChilds;
	bool bAutoLoadSymbols;
	DWORD dwSuspendType;
	DWORD dwBreakOnEPMode;
	DWORD dwDefaultExceptionMode;
};

struct DLLStruct
{
	PTCHAR sPath;
	bool bLoaded;
	DWORD dwPID;
	quint64 dwBaseAdr;
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
	quint64 dwEP;
	bool bKernelBP;
	bool bWOW64KernelBP;
	bool bRunning;
	bool bSymLoad;
	bool bTrapFlag;
	HANDLE hProc;
	PTCHAR sFileName;
};

struct BPStruct
{
	bool bRestoreBP;
	BYTE bOrgByte;
	quint64 dwOffset;
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
	*/
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
	bool ReadMemoryFromDebugee(DWORD dwPID,quint64 dwAddress,DWORD dwSize,LPVOID lpBuffer);
	bool WriteMemoryFromDebugee(DWORD dwPID,quint64 dwAddress,DWORD dwSize,LPVOID lpBuffer);
	bool AddBreakpointToList(DWORD dwBPType,DWORD dwTypeFlag,DWORD dwPID,quint64 dwOffset,DWORD dwSlot,DWORD dwKeep);

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

	static unsigned __stdcall DebuggingEntry(LPVOID pThis);

	bool PBThreadInfo(DWORD dwPID,DWORD dwTID,quint64 dwEP,bool bSuspended,DWORD dwExitCode,BOOL bNew);
	bool PBProcInfo(DWORD dwPID,PTCHAR sFileName,quint64 dwEP,DWORD dwExitCode,HANDLE hProc);
	bool PBExceptionInfo(quint64 dwExceptionOffset,quint64 dwExceptionCode,DWORD dwPID,DWORD dwTID);
	bool PBDLLInfo(PTCHAR sDLLPath,DWORD dwPID,quint64 dwEP,bool bLoaded);
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
	bool CheckIfExceptionIsBP(quint64 dwExceptionOffset,DWORD dwPID,bool bClearTrapFlag);
	bool SuspendProcess(DWORD dwPID,bool bSuspend);
	bool EnableDebugFlag();
	bool SetThreadContextHelper(bool bDecIP,bool bSetTrapFlag,DWORD dwThreadID, DWORD dwPID);

	HANDLE GetCurrentProcessHandle(DWORD dwPID);

	DWORD CallBreakDebugger(DEBUG_EVENT *debug_event,DWORD dwHandle);

	PTCHAR GetFileNameFromHandle(HANDLE hFile);

	typedef DWORD (__stdcall *CustomHandler)(DEBUG_EVENT *debug_event);

protected:
	void run();
};

#endif