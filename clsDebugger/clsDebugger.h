#ifndef CLSDEBUGGER
#define CLSDEBUGGER

#pragma pack(1)

#include <string>
#include <vector>

#include <Windows.h>
#include <process.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <time.h>

#include "dbghelp.h"

#pragma comment(lib,"psapi.lib")
#pragma comment(lib,"dbghelp.lib")

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
	};

	struct DLLStruct
	{
		PTCHAR sPath;
		bool bLoaded;
		DWORD dwBaseAdr;
		DWORD dwPID;
	};

	struct ThreadStruct
	{
		DWORD dwTID;
		DWORD dwEP;
		DWORD dwPID;
		bool bSuspended;
		DWORD dwExitCode;
	};

	struct PIDStruct
	{
		DWORD dwPID;
		DWORD dwExitCode;
		DWORD dwEP;
		DWORD dwBPRestoreFlag;
		BOOL bKernelBP;
		BOOL bRunning;
		BOOL bSymLoad;
		BOOL bTrapFlag;
		HANDLE hProc;
		PTCHAR sFileName;
	};

	struct BPStruct
	{
		DWORD dwOffset;
		DWORD dwSize;
		DWORD dwSlot;
		DWORD dwPID;
		bool bRestoreBP;
		BYTE bOrgByte;
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
		DWORD dwHandler;
		DWORD dwAction;
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
	bool StepOver(DWORD dwNewOffset);
	bool StepIn();
	bool ShowCallStack();
	bool DetachFromProcess();
	bool AttachToProcess(DWORD dwPID);
	bool IsTargetSet();
	bool RemoveBPFromList(DWORD dwOffset,DWORD dwType,DWORD dwPID);
	bool RemoveBPs();
	bool LoadSymbolForAddr(wstring& sFuncName,wstring& sModName,DWORD dwOffset);
	bool ReadMemoryFromDebugee(DWORD dwPID,DWORD dwAddress,DWORD dwSize,LPVOID lpBuffer);
	bool WriteMemoryFromDebugee(DWORD dwPID,DWORD dwAddress,DWORD dwSize,LPVOID lpBuffer);

	DWORD GetCurrentPID();
	DWORD GetCurrentTID();

	void AddBreakpointToList(DWORD dwBPType,DWORD dwPID,DWORD dwOffset,DWORD dwSlot,DWORD dwKeep);
	void SetTarget(wstring sTarget);
	void CustomExceptionAdd(DWORD dwExceptionType,DWORD dwAction,DWORD dwHandler);
	void CustomExceptionRemove(DWORD dwExceptionType);
	void CustomExceptionRemoveAll();

	int (*dwOnThread)(DWORD dwPID,DWORD dwTID,DWORD dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
	int (*dwOnPID)(DWORD dwPID,wstring sFile,DWORD dwExitCode,DWORD dwEP,bool bFound);
	int (*dwOnException)(wstring sFuncName,wstring sModName,DWORD dwOffset,DWORD dwExceptionCode,DWORD dwPID,DWORD dwTID);
	int (*dwOnDbgString)(wstring sMessage,DWORD dwPID);
	int (*dwOnLog)(tm timeInfo,wstring sLog);
	int (*dwOnDll)(wstring sDLLPath,DWORD dwPID,DWORD dwEP,bool bLoaded);
	int (*dwOnCallStack)(DWORD dwStackAddr,
						 DWORD dwReturnTo,wstring sReturnToFunc,wstring sModuleName,
						 DWORD dwEIP,wstring sFuncName,wstring sFuncModule,
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
	DWORD _dwPidToAttach;
	DWORD _dwCurPID;
	DWORD _dwCurTID;

	void DebuggingLoop();
	void AttachedDebugging(LPVOID pDebProc);
	void NormalDebugging(LPVOID pDebProc);
	void CleanWorkSpace();
	
	static unsigned DebuggingEntry(LPVOID pThis);

	bool PBThreadInfo(DWORD dwPID,DWORD dwTID,DWORD dwEP,bool bSuspended,DWORD dwExitCode,BOOL bNew);
	bool PBProcInfo(DWORD dwPID,PTCHAR sFileName,DWORD dwEP,DWORD dwExitCode,HANDLE hProc);
	bool PBExceptionInfo(DWORD dwExceptionOffset,DWORD dwExceptionCode,DWORD dwPID,DWORD dwTID);
	bool PBDLLInfo(PTCHAR sDLLPath,DWORD dwPID,DWORD dwEP,bool bLoaded);
	bool PBLogInfo();
	bool PBDbgString(PTCHAR sMessage,DWORD dwPID);
	bool wSoftwareBP(DWORD dwPID,DWORD dwOffset,DWORD dwKeep,DWORD dwSize,BYTE& bOrgByte);
	bool wMemoryBP(DWORD dwPID,DWORD dwOffset,DWORD dwSize,DWORD dwKeep);
	bool wHardwareBP(DWORD dwPID,DWORD dwOffset,DWORD dwSize,DWORD dwSlot);
	bool dHardwareBP(DWORD dwPID,DWORD dwOffset,DWORD dwSlot);
	bool InitBP();
	bool IsValidFile();
	bool CheckProcessState(DWORD dwPID);
	bool CheckIfExceptionIsBP(DWORD dwExceptionOffset,DWORD dwPID,bool bClearTrapFlag);
	bool SuspendProcess(DWORD dwPID,bool bSuspend);
	bool EnableDebugFlag();

	DWORD CallBreakDebugger(DEBUG_EVENT *debug_event,DWORD dwHandle);
	DWORD GetReturnAdressFromStackFrame(DWORD dwEbp,DEBUG_EVENT *debug_event);

	PTCHAR GetFileNameFromHandle(HANDLE hFile);

	typedef DWORD (__stdcall *CustomHandler)(DEBUG_EVENT *debug_event);
};

#endif