#include "clsDebugger.h"
#include "clsHelperClass.h"
#include "clsMemManager.h"
#include "dbghelp.h"

using namespace std;

bool clsDebugger::PBThreadInfo(DWORD dwPID,DWORD dwTID,quint64 dwEP,bool bSuspended,DWORD dwExitCode,BOOL bNew)
{
	bool bFound = false;

	for(size_t i = 0;i < TIDs.size();i++)
	{
		if(TIDs[i].dwTID == dwTID && TIDs[i].dwPID == dwPID)
		{
			TIDs[i].dwExitCode = dwExitCode;
			bFound = true;
			break;
		}
	}

	if(!bFound)
	{
		ThreadStruct newTID;
		newTID.bSuspended = bSuspended;
		newTID.dwEP = dwEP;
		newTID.dwTID = dwTID;
		newTID.dwPID = dwPID;
		newTID.dwExitCode = 0;

		TIDs.push_back(newTID);
	}

	emit OnThread(dwPID,dwTID,dwEP,bSuspended,dwExitCode,bFound);

	memset(tcLogString,0x00,LOGBUFFER);

	if(bFound)
#ifdef _AMD64_
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[-] Exit Thread(%X) in Process(%X) with Exitcode: %08X",dwTID,dwPID,dwExitCode);
	else
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] New Thread(%X) in Process(%X) with Entrypoint: %016I64X",dwTID,dwPID,dwEP);
#else
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[-] Exit Thread(%X) in Process(%X) with Exitcode: %08X",dwTID,dwPID,dwExitCode);
	else
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] New Thread(%X) in Process(%X) with Entrypoint: %08X",dwTID,dwPID,(DWORD)dwEP);
#endif
	PBLogInfo();

	return true;
}

bool clsDebugger::PBProcInfo(DWORD dwPID,PTCHAR sFileName,quint64 dwEP,DWORD dwExitCode,HANDLE hProc)
{
	bool bFound = false;

	for(size_t i = 0;i < PIDs.size();i++)
	{
		if(PIDs[i].dwPID == dwPID)
		{
			PIDs[i].dwExitCode = dwExitCode;
			PIDs[i].bRunning = false;
			bFound = true;
		}
	}

	if(!bFound)
	{
		PIDStruct newPID;
		newPID.dwPID = dwPID;
		newPID.dwEP = dwEP;
		newPID.sFileName = sFileName;
		newPID.dwExitCode = dwExitCode;
		newPID.hProc = hProc;
		if(!_NormalDebugging)
		{
			_NormalDebugging = true;
			newPID.bKernelBP = true;
		}
		else
			newPID.bKernelBP = false;

		newPID.bWOW64KernelBP = false;
		newPID.bSymLoad = false;
		newPID.bRunning = true;
		newPID.bTrapFlag = false;
		newPID.bTraceFlag = false;
		newPID.dwBPRestoreFlag = NULL;
		PIDs.push_back(newPID);
	}

	if(!bFound)
		emit OnPID(dwPID,sFileName,dwExitCode,dwEP,bFound);
	else
		emit OnPID(dwPID,L"",dwExitCode,NULL,bFound);

	memset(tcLogString,0x00,LOGBUFFER);

	if(bFound)
#ifdef _AMD64_
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[-] Exit Process(%X) with Exitcode: %016I64X",dwPID,dwExitCode);
	else
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] New Process(%X) with Entrypoint: %016I64X",dwPID,dwEP);
#else
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[-] Exit Process(%X) with Exitcode: %08X",dwPID,dwExitCode);
	else
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] New Process(%X) with Entrypoint: %08X",dwPID,(DWORD)dwEP);
#endif
	PBLogInfo();

	return true;
}

bool clsDebugger::PBExceptionInfo(quint64 dwExceptionOffset,quint64 dwExceptionCode,DWORD dwPID,DWORD dwTID)
{
	wstring sModName,sFuncName;
	clsHelperClass::LoadSymbolForAddr(sFuncName,sModName,dwExceptionOffset,GetCurrentProcessHandle(dwPID));

	emit OnException(sFuncName,sModName,dwExceptionOffset,dwExceptionCode,dwPID,dwTID);

	memset(tcLogString,0x00,LOGBUFFER);

	if(sFuncName.length() > 0 && sModName.length() > 0)
#ifdef _AMD64_
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] %s@%s ExceptionCode: %016I64X ExceptionOffset: %016I64X PID: %X TID: %X",
#else
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] %s@%s ExceptionCode: %08X ExceptionOffset: %08X PID: %X TID: %X",
#endif
			sFuncName.c_str(),
			sModName.c_str(),
			dwExceptionCode,
			dwExceptionOffset,
			dwPID,
			dwTID);
	else
#ifdef _AMD64_
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] ExceptionCode: %016I64X ExceptionOffset: %016I64X PID: %X TID: %X",
#else
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] ExceptionCode: %08X ExceptionOffset: %08X PID: %X TID: %X",
#endif
			dwExceptionCode,
			dwExceptionOffset,
			dwPID,
			dwTID);
	PBLogInfo();

	return true;
}

bool clsDebugger::PBDLLInfo(PTCHAR sDLLPath,DWORD dwPID,quint64 dwEP,bool bLoaded)
{
	if(sDLLPath == NULL) return false;
	bool bFound = false;
	for(size_t i = 0;i < DLLs.size(); i++)
	{
		if(wcscmp(DLLs[i].sPath,sDLLPath) == NULL && DLLs[i].dwPID == dwPID)
		{
			DLLs[i].bLoaded = bLoaded;
			bFound = true;
		}
	}

	if(!bFound)
	{
		DLLStruct newDLL;
		newDLL.bLoaded = bLoaded;
		newDLL.dwBaseAdr = dwEP;
		newDLL.sPath = sDLLPath;
		newDLL.dwPID = dwPID;

		DLLs.push_back(newDLL);
	}

	emit OnDll(sDLLPath,dwPID,dwEP,bLoaded);

	memset(tcLogString,0x00,LOGBUFFER);
	if(bLoaded)
#ifdef _AMD64_
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] PID(%X) - Loaded DLL: %s Entrypoint: %016I64X",dwPID,sDLLPath,dwEP);
#else
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] PID(%X) - Loaded DLL: %s Entrypoint: %08X",dwPID,sDLLPath,(DWORD)dwEP);
#endif
	else
	{
		HANDLE hProc = GetCurrentProcessHandle(dwPID);

		SymUnloadModule64(hProc,dwEP);
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] PID(%X) - Unloaded DLL: %s",dwPID,sDLLPath);
	}
	PBLogInfo();

	return true;
}

bool clsDebugger::PBDbgString(PTCHAR sMessage,DWORD dwPID)
{
	emit OnDbgString(sMessage,dwPID);

	clsMemManager::CFree(sMessage);
	return true;
}

bool clsDebugger::PBLogInfo()
{
	emit OnLog(tcLogString);
	return true;
}