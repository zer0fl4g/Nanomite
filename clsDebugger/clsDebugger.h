#ifndef CLSDEBUGGER
#define CLSDEBUGGER

//#pragma pack(1)

#include <string>
#include <vector>

#include <Windows.h>
#include <process.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <time.h>

#pragma comment(lib,"psapi.lib")

#include "dbghelp.h"

#ifdef _AMD64_
#pragma comment(lib,"../clsDebugger/dbghelp_x64.lib")
#else
#pragma comment(lib,"../clsDebugger/dbghelp_x86.lib")
#endif

using namespace std;

// Taken from GDB
#define DR_EXECUTE	(0x00)		/* Break on instruction execution.  */
#define DR_WRITE	(0x01)		/* Break on data writes.			*/
#define DR_READ		(0x11)		/* Break on data reads or writes.	*/
//

#define LOGBUFFER (512 * sizeof(TCHAR))
#define LOGBUFFERCHAR (512)
#define THREAD_GETSET_CONTEXT	(0x0018) 

class clsDebugger
{
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
		DWORD64 dwBaseAdr;
	};

	struct ThreadStruct
	{
		DWORD dwTID;
		DWORD dwPID;
		DWORD dwExitCode;
		DWORD64 dwEP;
		bool bSuspended;
	};

	struct PIDStruct
	{
		DWORD dwPID;
		DWORD dwExitCode;
		DWORD dwBPRestoreFlag;
		DWORD64 dwEP;
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
		DWORD64 dwOffset;
		DWORD dwSize;
		DWORD dwSlot;
		DWORD dwPID;
		DWORD dwTypeFlag;
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
		DWORD64 dwHandler;
	};

public:

	vector<DLLStruct> DLLs;
	vector<ThreadStruct> TIDs;
	vector<PIDStruct> PIDs;
	vector<BPStruct> SoftwareBPs;
	vector<BPStruct> MemoryBPs;
	vector<BPStruct> HardwareBPs;
	vector<customException> ExceptionHandler;

	CONTEXT ProcessContext;
	WOW64_CONTEXT wowProcessContext;

	clsDebuggerSettings dbgSettings;

	HANDLE hDebuggingHandle;

	clsDebugger();
	clsDebugger(wstring sTarget);
	~clsDebugger();

	bool StopDebuggingAll();
	bool StopDebugging(DWORD dwPID);
	bool SuspendDebuggingAll();
	bool SuspendDebugging(DWORD dwPID);
	bool ResumeDebugging();
	bool RestartDebugging();
	bool StartDebugging();
	bool GetDebuggingState();
	bool StepOver(DWORD64 dwNewOffset);
	bool StepIn();
	bool ShowCallStack();
	bool DetachFromProcess();
	bool AttachToProcess(DWORD dwPID);
	bool IsTargetSet();
	bool RemoveBPFromList(DWORD64 dwOffset,DWORD dwType,DWORD dwPID);
	bool RemoveBPs();
	bool LoadSymbolForAddr(wstring& sFuncName,wstring& sModName,DWORD64 dwOffset);
	bool ReadMemoryFromDebugee(DWORD dwPID,DWORD64 dwAddress,DWORD dwSize,LPVOID lpBuffer);
	bool WriteMemoryFromDebugee(DWORD dwPID,DWORD64 dwAddress,DWORD dwSize,LPVOID lpBuffer);

	DWORD GetCurrentPID();
	DWORD GetCurrentTID();

	void AddBreakpointToList(DWORD dwBPType,DWORD dwTypeFlag,DWORD dwPID,DWORD64 dwOffset,DWORD dwSlot,DWORD dwKeep);
	void SetTarget(wstring sTarget);
	void CustomExceptionAdd(DWORD dwExceptionType,DWORD dwAction,DWORD64 dwHandler);
	void CustomExceptionRemove(DWORD dwExceptionType);
	void CustomExceptionRemoveAll();

	HANDLE GetCurrentProcessHandle();

	int (*dwOnThread)(DWORD dwPID,DWORD dwTID,DWORD64 dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
	int (*dwOnPID)(DWORD dwPID,wstring sFile,DWORD dwExitCode,DWORD64 dwEP,bool bFound);
	int (*dwOnException)(wstring sFuncName,wstring sModName,DWORD64 dwOffset,DWORD64 dwExceptionCode,DWORD dwPID,DWORD dwTID);
	int (*dwOnDbgString)(wstring sMessage,DWORD dwPID);
	int (*dwOnLog)(tm timeInfo,wstring sLog);
	int (*dwOnDll)(wstring sDLLPath,DWORD dwPID,DWORD64 dwEP,bool bLoaded);
	int (*dwOnCallStack)(DWORD64 dwStackAddr,
						 DWORD64 dwReturnTo,wstring sReturnToFunc,wstring sModuleName,
						 DWORD64 dwEIP,wstring sFuncName,wstring sFuncModule,
						 wstring sSourceFilePath,int iSourceLineNum);

	wstring GetTarget();

private:
	PTCHAR tcLogString;
	wstring _sTarget;
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
	
	static unsigned DebuggingEntry(LPVOID pThis);

	bool PBThreadInfo(DWORD dwPID,DWORD dwTID,DWORD64 dwEP,bool bSuspended,DWORD dwExitCode,BOOL bNew);
	bool PBProcInfo(DWORD dwPID,PTCHAR sFileName,DWORD64 dwEP,DWORD dwExitCode,HANDLE hProc);
	bool PBExceptionInfo(DWORD64 dwExceptionOffset,DWORD64 dwExceptionCode,DWORD dwPID,DWORD dwTID);
	bool PBDLLInfo(PTCHAR sDLLPath,DWORD dwPID,DWORD64 dwEP,bool bLoaded);
	bool PBLogInfo();
	bool PBDbgString(PTCHAR sMessage,DWORD dwPID);
	bool wSoftwareBP(DWORD dwPID,DWORD64 dwOffset,DWORD dwKeep,DWORD dwSize,BYTE& bOrgByte);
	bool dSoftwareBP(DWORD dwPID,DWORD64 dwOffset,DWORD dwSize,BYTE btOrgByte);
	bool wMemoryBP(DWORD dwPID,DWORD64 dwOffset,DWORD dwSize,DWORD dwKeep);
	bool dMemoryBP(DWORD dwPID,DWORD64 dwOffset,DWORD dwSize);
	bool wHardwareBP(DWORD dwPID,DWORD64 dwOffset,DWORD dwSize,DWORD dwSlot,DWORD dwTypeFlag);
	bool dHardwareBP(DWORD dwPID,DWORD64 dwOffset,DWORD dwSlot);
	bool InitBP();
	bool IsValidFile();
	bool CheckProcessState(DWORD dwPID);
	bool CheckIfExceptionIsBP(DWORD64 dwExceptionOffset,DWORD dwPID,bool bClearTrapFlag);
	bool SuspendProcess(DWORD dwPID,bool bSuspend);
	bool EnableDebugFlag();
	bool SetThreadContextHelper(bool bDecIP,bool bSetTrapFlag,DWORD dwThreadID, DWORD dwPID);

	HANDLE GetCurrentProcessHandle(DWORD dwPID);

	DWORD CallBreakDebugger(DEBUG_EVENT *debug_event,DWORD dwHandle);
	DWORD GetReturnAdressFromStackFrame(DWORD dwEbp,DEBUG_EVENT *debug_event);

	PTCHAR GetFileNameFromHandle(HANDLE hFile);

	typedef DWORD (__stdcall *CustomHandler)(DEBUG_EVENT *debug_event);
};

#endif