#pragma once
#include <string>
#include <vector>
#include <sstream>

#include <Windows.h>
#include <process.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <DbgHelp.h>
#include <time.h>

#pragma comment(lib,"psapi.lib")
#pragma comment(lib,"dbghelp.lib")

using namespace std;

// Taken from GDB
#define DR_EXECUTE	(0x00)		/* Break on instruction execution.  */
#define DR_WRITE	(0x01)		/* Break on data writes.			*/
#define DR_READ		(0x11)		/* Break on data reads or writes.	*/


class clsDebugger
{
	struct clsDebuggerSettings
	{
		bool bDebugChilds;
		bool bAutoLoadSymbols;
		DWORD dwSuspendType;
		DWORD dwBreakOnEPMode;
		DWORD dwEXCEPTION_ACCESS_VIOLATION;
		DWORD dwEXCEPTION_ARRAY_BOUNDS_EXCEEDED;
		DWORD dwEXCEPTION_BREAKPOINT;
		DWORD dwEXCEPTION_DATATYPE_MISALIGNMENT;
		DWORD dwEXCEPTION_FLT_DENORMAL_OPERAND;
		DWORD dwEXCEPTION_FLT_DIVIDE_BY_ZERO;
		DWORD dwEXCEPTION_FLT_INEXACT_RESULT;
		DWORD dwEXCEPTION_FLT_INVALID_OPERATION;
		DWORD dwEXCEPTION_FLT_OVERFLOW;
		DWORD dwEXCEPTION_FLT_STACK_CHECK;
		DWORD dwEXCEPTION_FLT_UNDERFLOW;
		DWORD dwEXCEPTION_ILLEGAL_INSTRUCTION;
		DWORD dwEXCEPTION_IN_PAGE_ERROR;
		DWORD dwEXCEPTION_INT_DIVIDE_BY_ZERO;
		DWORD dwEXCEPTION_INT_OVERFLOW;
		DWORD dwEXCEPTION_INVALID_DISPOSITION;
		DWORD dwEXCEPTION_NONCONTINUABLE_EXCEPTION;
		DWORD dwEXCEPTION_PRIV_INSTRUCTION;
		DWORD dwEXCEPTION_SINGLE_STEP;
		DWORD dwEXCEPTION_STACK_OVERFLOW;
		DWORD dwEXCEPTION_GUARD_PAGE;
	};

	struct DLLStruct
	{
		wstring sPath;
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

	struct ExceptionStruct
	{
		DWORD dwExceptionOffset;
		DWORD dwExceptionCode;
		DWORD dwPID;
		DWORD dwTID;
	};


	struct PIDStruct
	{
		DWORD dwPID;
		DWORD dwExitCode;
		DWORD dwEP;
		BOOL bKernelBP;
		BOOL bRunning;
		BOOL bSymLoad;
		HANDLE hSymInfo;
		wstring sFileName;
	};

	struct BPStruct
	{
		DWORD dwOffset;
		DWORD dwSize;
		DWORD dwSlot;
		DWORD dwPID;
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
		DWORD dwAction;
	};
public:

	vector<DLLStruct> DLLs;
	vector<ThreadStruct> Threads;
	vector<ExceptionStruct> Exceptions;
	vector<PIDStruct> PIDs;
	vector<wstring>DbgStrings;
	vector<wstring>Log;
	vector<BPStruct>SoftwareBPs;
	vector<BPStruct>MemoryBPs;
	vector<BPStruct>HardwareBPs;
	vector<customException>exCustom;

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
	bool ShowCallStack();
	bool DetachFromProcess(bool bAll,DWORD dwPID);
	bool AttachToProcess(DWORD dwPID);
	bool IsTargetSet();
	bool RemoveBPFromList(DWORD dwOffset,DWORD dwType,DWORD dwPID);
	bool RemoveBPs();
	bool LoadSymbolForAddr(wstring& sFuncName,wstring& sModName,DWORD dwOffset);


	DWORD GetCurrentPID();
	DWORD GetCurrentTID();

	void AddBreakpointToList(DWORD dwTypeOfBreakPoint,DWORD dwPID,DWORD dwOffset,DWORD dwKeep);
	void SetTarget(wstring sTarget);
	void CustomExceptionAdd(DWORD dwExceptionType,DWORD dwAction);
	void CustomExceptionRemove(DWORD dwExceptionType);

	int (*dwOnThread)(DWORD dwPID,DWORD dwTID,DWORD dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
	int (*dwOnPID)(DWORD dwPID,wstring sFile,DWORD dwExitCode,DWORD dwEP,bool bFound);
	int (*dwOnException)(wstring sFuncName,wstring sModName,DWORD dwOffset,DWORD dwExceptionCode,DWORD dwPID,DWORD dwTID);
	int (*dwOnDbgString)(wstring sMessage,DWORD dwPID);
	int (*dwOnLog)(wstring sLog);
	int (*dwOnDll)(wstring sDLLPath,DWORD dwPID,DWORD dwEP,bool bLoaded);
	int (*dwOnCallStack)(DWORD dwStackAddr,
						 DWORD dwReturnTo,wstring sReturnToFunc,wstring sModuleName,
						 DWORD dwEIP,wstring sFuncName,wstring sFuncModule,
						 wstring sSourceFilePath,int iSourceLineNum);

	wstring GetTarget();

private:
	wstring _sTarget;
	STARTUPINFO _si;
	PROCESS_INFORMATION _pi;
	bool _isDebugging;
	HANDLE _hDbgEvent;
	bool _NormalDebugging;
	DWORD _dwPidToAttach,_dwCurPID,_dwCurTID;

	void DebuggingLoop();
	void AttachedDebugging(LPVOID pDebProc);
	void NormalDebugging(LPVOID pDebProc);
	void SetDebugBitMask(DWORD &dwOld,DWORD dwMask,DWORD dwBits,DWORD dwNewValue);
	void CleanWorkSpace();

	static unsigned _stdcall DebuggingEntry(LPVOID pThis);

	bool PBThreadInfo(DWORD dwPID,DWORD dwTID,DWORD dwEP,bool bSuspended,DWORD dwExitCode,BOOL bNew);
	bool PBProcInfo(DWORD dwPID,wstring sFileName,DWORD dwEP,DWORD dwExitCode,HANDLE hSymInfo);
	bool PBExceptionInfo(DWORD dwExceptionOffset,DWORD dwExceptionCode,DWORD dwPID,DWORD dwTID);
	bool PBDLLInfo(wstring sDLLPath,DWORD dwPID,DWORD dwEP,bool bLoaded);
	bool PBLogInfo(wstring sLog);
	bool PBDbgString(wstring sMessage,DWORD dwPID);
	bool wSoftwareBP(DWORD dwPID,DWORD dwOffset,DWORD dwKeep,DWORD dwSize,BYTE& bOrgByte);
	bool wMemoryBP(DWORD dwPID,DWORD dwOffset,DWORD dwSize,DWORD dwKeep);
	bool wHardwareBP(DWORD dwPID,DWORD dwOffset,DWORD dwSize,DWORD dwKeep);
	bool InitBP();
	bool IsValidFile();
	bool RemoveHWBP(DWORD dwPID);
	bool CheckProcessState(DWORD dwPID);
	bool CheckIfExceptionIsBP(EXCEPTION_DEBUG_INFO exInfo);
	bool SuspendProcess(DWORD dwPID,bool bSuspend);

	DWORD CallBreakDebugger(DEBUG_EVENT debug_event,DWORD dwHandle);
	DWORD GetReturnAdressFromStackFrame(DWORD dwEbp,DEBUG_EVENT debug_event);

	wstring GetFileNameFromHandle(HANDLE hFile);
};

