#include "clsDebugger.h"
#include "dbghelp.h"
#include "clsAPIImport.h"
#include "clsHelperClass.h"
#include "clsPEManager.h"

#include <process.h>
#include <Psapi.h>
#include <TlHelp32.h>

#pragma comment(lib,"psapi.lib")

#ifdef _AMD64_
#pragma comment(lib,"clsDebugger/dbghelp_x64.lib")
#else
#pragma comment(lib,"clsDebugger/dbghelp_x86.lib")
#endif

// defuq?
#define SYNCHRONIZE                      (0x00100000L)

using namespace std;

clsDebugger* clsDebugger::pThis = NULL;

clsDebugger::clsDebugger()
{
	_NormalDebugging = true;
	_isDebugging = false;
	tcLogString = (PTCHAR)malloc(LOGBUFFER);
	_sCommandLine = L"";
	clsDebuggerSettings tempSet = {false,false,0,0};
	dbgSettings = tempSet;
	pThis = this;
	EnableDebugFlag();
}

clsDebugger::clsDebugger(wstring sTarget)
{
	_sTarget = sTarget;
	_NormalDebugging = true;
	_isDebugging = false;
	tcLogString = (PTCHAR)malloc(LOGBUFFER);
	_sCommandLine = L"";
	clsDebuggerSettings tempSet = {false,false,0,0};
	dbgSettings = tempSet;
	pThis = this;
	EnableDebugFlag();
}

clsDebugger::~clsDebugger(void)
{
	CleanWorkSpace();
	free(tcLogString);
}

void clsDebugger::SetTarget(wstring sTarget)
{
	_sTarget = sTarget;
	_NormalDebugging = true;
}

wstring clsDebugger::GetTarget()
{
	return _sTarget;
}

void clsDebugger::CleanWorkSpace()
{
	for(size_t i = 0;i < PIDs.size();i++)
		SymCleanup(PIDs[i].hProc);

	PIDs.clear();
	DLLs.clear();
	TIDs.clear();
}

bool clsDebugger::SuspendDebuggingAll()
{
	for(size_t i = 0;i < PIDs.size();i++)
		SuspendDebugging(PIDs[i].dwPID);
	CleanWorkSpace();
	return true;
}

bool clsDebugger::SuspendDebugging(DWORD dwPID)
{
	if(CheckProcessState(dwPID))
	{
		if(dbgSettings.dwSuspendType == 0x0)
		{
			HANDLE hProcess = NULL;
			for(size_t i = 0;i < PIDs.size();i++)
			{
				if(PIDs[i].bRunning && PIDs[i].dwPID == dwPID)
					hProcess = PIDs[i].hProc;
			}

			if(DebugBreakProcess(hProcess))
			{
				memset(tcLogString,0x00,LOGBUFFER);
				swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] %X Debugging suspended!",dwPID);
				PBLogInfo();
				return true;
			}
		}
		else// if(dbgSettings.dwSuspendType == 0x1)
		{
			if(SuspendProcess(dwPID,true))
			{
				memset(tcLogString,0x00,LOGBUFFER);
				swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] %X Debugging suspended!",dwPID);
				PBLogInfo();
				return true;
			}
		}
	}
	return false;
}

bool clsDebugger::StopDebuggingAll()
{
	_isDebugging = false;
	for(size_t i = 0;i < PIDs.size();i++)
		StopDebugging(PIDs[i].dwPID);
	return PulseEvent(_hDbgEvent);
}

bool clsDebugger::StopDebugging(DWORD dwPID)
{
	HANDLE hProcess = NULL;
	int iPid = 0;
	for(size_t i = 0;i < PIDs.size();i++)
	{
		if(PIDs[i].dwPID == dwPID)
			hProcess = PIDs[i].hProc;iPid = i;
	}
	if(CheckProcessState(dwPID))
	{
		if(TerminateProcess(hProcess,0))
		{
			PBProcInfo(PIDs[iPid].dwPID,PIDs[iPid].sFileName,PIDs[iPid].dwEP,0x0,hProcess);
			if(PIDs.size() == 1)
			{
				_isDebugging = false;
				//PulseEvent(_hDbgEvent);
			}
			return true;
		}
	}
	return false;
}

bool clsDebugger::ResumeDebugging()
{
	for(size_t i = 0;i < PIDs.size(); i++)
		SuspendProcess(PIDs[i].dwPID,false);
	return PulseEvent(_hDbgEvent);
}

bool clsDebugger::RestartDebugging()
{
	StopDebuggingAll();
	Sleep(2000);
	StartDebugging();
	return true;
}

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
		newTID.dwExitCode = dwExitCode;

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
		newPID.bKernelBP = false;
		newPID.bWOW64KernelBP = false;
		newPID.bSymLoad = false;
		newPID.bRunning = true;
		newPID.bTrapFlag = false;
		newPID.dwBPRestoreFlag = NULL;
		PIDs.push_back(newPID);
	}

	emit OnPID(dwPID,sFileName,dwExitCode,dwEP,bFound);

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

	_dwCurPID = dwPID;
	clsHelperClass::LoadSymbolForAddr(sFuncName,sModName,dwExceptionOffset,GetCurrentProcessHandle(dwPID));
	_dwCurPID = 0;

	emit OnException(sFuncName,sModName,dwExceptionOffset,dwExceptionCode,dwPID,dwTID);

	memset(tcLogString,0x00,LOGBUFFER);
#ifdef _AMD64_
	swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] %s@%s ExceptionCode: %016I64X ExceptionOffset: %016I64X PID: %X TID: %X",
		sFuncName.c_str(),
		sModName.c_str(),
		dwExceptionCode,
		dwExceptionOffset,
		dwPID,
		dwTID);
#else
	swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] %s@%s ExceptionCode: %08X ExceptionOffset: %08X PID: %X TID: %X",
		sFuncName.c_str(),
		sModName.c_str(),
		(DWORD)dwExceptionCode,
		(DWORD)dwExceptionOffset,
		dwPID,
		dwTID);
#endif
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

	free(sMessage);
	return true;
}

bool clsDebugger::PBLogInfo()
{
	emit OnLog(tcLogString);
	return true;
}

PTCHAR clsDebugger::GetFileNameFromHandle(HANDLE hFile) 
{
	HANDLE hFileMap;
	DWORD dwFileSizeHi = 0,
		dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 

	if(dwFileSizeLo == 0 && dwFileSizeHi == 0)
		return false;

	hFileMap = CreateFileMapping(hFile,NULL,PAGE_READONLY,0,1,NULL);
	if (hFileMap == INVALID_HANDLE_VALUE) 
		return NULL;

	LPVOID pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);
	if (pMem == NULL) 
	{
		CloseHandle(hFileMap);
		return NULL;
	}

	PTCHAR tcFilename = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
	if (!GetMappedFileName(GetCurrentProcess(),pMem,tcFilename,MAX_PATH)) 
	{
		UnmapViewOfFile(pMem);
		CloseHandle(hFileMap);
		free(tcFilename);
		return NULL;
	}

	PTCHAR tcTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
	tcTemp[0] = '\0';

	if (!GetLogicalDriveStrings(255-1, tcTemp)) 
	{
		UnmapViewOfFile(pMem);
		CloseHandle(hFileMap);
		free(tcFilename);
		free(tcTemp);
		return NULL;
	}

	PTCHAR tcName = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
	PTCHAR tcFile = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
	TCHAR tcDrive[3] = TEXT(" :");
	BOOL bFound = false;
	PTCHAR p = tcTemp;

	do 
	{
		*tcDrive = *p;

		if (QueryDosDevice(tcDrive,tcName,MAX_PATH))
		{
			size_t uNameLen = wcslen(tcName);
			if (uNameLen < MAX_PATH) 
			{
				bFound = _wcsnicmp(tcFilename,tcName,uNameLen) == 0;
				if (bFound)
					swprintf_s(tcFile,255,L"%s%s",tcDrive,(tcFilename + uNameLen));
			}
		}
		while (*p++);
	} while (!bFound && *p);

	free(tcName);
	free(tcTemp);
	free(tcFilename);
	UnmapViewOfFile(pMem);
	CloseHandle(hFileMap);

	return tcFile;
}

bool clsDebugger::GetDebuggingState()
{
	if(_isDebugging == true)
		return true;
	else
		return false;
}

void clsDebugger::run()
{
	StartDebugging();
}

bool clsDebugger::StartDebugging()
{
	if(_dwPidToAttach != 0 && !_NormalDebugging)
	{
		CleanWorkSpace();
		_isDebugging = true;
		_bSingleStepFlag = false;
		_beginthreadex(NULL,NULL,clsDebugger::DebuggingEntry,this,NULL,NULL);
	}	
	else
	{
		if(_sTarget.length() <= 0 || _isDebugging)
			return false;

		CleanWorkSpace();
		_isDebugging = true;
		_bSingleStepFlag = false;
		_beginthreadex(NULL,NULL,clsDebugger::DebuggingEntry,this,NULL,NULL);
	}
	return true;
}

unsigned __stdcall clsDebugger::DebuggingEntry(LPVOID pThis)
{
	clsDebugger* pThat = (clsDebugger*)pThis;

	if(pThat->_NormalDebugging)
		pThat->NormalDebugging(pThis);
	else
		pThat->AttachedDebugging(pThis);
	return 0;
}

void clsDebugger::AttachedDebugging(LPVOID pDebProc)
{
	if(CheckProcessState(_dwPidToAttach) && DebugActiveProcess(_dwPidToAttach))
	{
		_hDbgEvent = CreateEvent(NULL,false,false,L"hDebugEvent");

		memset(tcLogString,0x00,LOGBUFFER);
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] Attached to Process");
		PBLogInfo();
		DebuggingLoop();
		_NormalDebugging = true;
		return;
	}
}

void clsDebugger::NormalDebugging(LPVOID pDebProc)
{
	clsDebugger *pThis = (clsDebugger*)pDebProc;

	ZeroMemory(&_si, sizeof(_si));
	_si.cb = sizeof(_si);
	ZeroMemory(&_pi, sizeof(_pi));

	DWORD dwCreationFlag = 0x2;
	_hDbgEvent = CreateEvent(NULL,false,false,L"hDebugEvent");

	if(dbgSettings.bDebugChilds == true)
		dwCreationFlag = 0x1;

	if(CreateProcess(_sTarget.c_str(),(LPWSTR)_sCommandLine.c_str(),NULL,NULL,false,dwCreationFlag,NULL,NULL,&_si,&_pi))
		DebuggingLoop();
}

void clsDebugger::DebuggingLoop()
{
	DEBUG_EVENT debug_event = {0};
	bool bContinueDebugging = true;
	DWORD dwContinueStatus = DBG_CONTINUE;

	DebugSetProcessKillOnExit(false);

	while(bContinueDebugging && _isDebugging)
	{ 
		if (!WaitForDebugEvent(&debug_event, INFINITE))
			bContinueDebugging = false;

		if(_bStopDebugging && !_isDebugging)
		{
			_bStopDebugging = false;
			DebugActiveProcessStop(debug_event.dwProcessId);
			ContinueDebugEvent(debug_event.dwProcessId,debug_event.dwThreadId,DBG_CONTINUE);
			break;
		}

		switch(debug_event.dwDebugEventCode)
		{
		case CREATE_PROCESS_DEBUG_EVENT:
			{
				HANDLE hProc = debug_event.u.CreateProcessInfo.hProcess;
				PTCHAR tcDllFilepath = GetFileNameFromHandle(debug_event.u.CreateProcessInfo.hFile);
				PBProcInfo(debug_event.dwProcessId,tcDllFilepath,(quint64)debug_event.u.CreateProcessInfo.lpStartAddress,-1,hProc);

				BOOL Is64Bit = false;
				clsAPIImport::pIsWow64Process(hProc,&Is64Bit);

				emit OnNewPID((wstring)tcDllFilepath,debug_event.dwProcessId,Is64Bit);

				//clsDBInterface *pDB = new clsDBInterface((wstring)tcDllFilepath,debug_event.dwProcessId);
				//fileDBs.push_back(pDB);

				int iPid = 0;
				for(size_t i = 0;i < PIDs.size();i++)
				{
					if(PIDs[i].dwPID == debug_event.dwProcessId)
						iPid = i;
				}

				PIDs[iPid].bSymLoad = SymInitialize(hProc,NULL,false);
				if(!PIDs[iPid].bSymLoad)
				{
					memset(tcLogString,0x00,LOGBUFFER);
					swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Could not load symbols for Process(%X)",debug_event.dwProcessId);
					PBLogInfo();
				}
				else
					SymLoadModuleExW(hProc,NULL,tcDllFilepath,0,(quint64)debug_event.u.CreateProcessInfo.lpBaseOfImage,0,0,0);

				AddBreakpointToList(NULL,2,-1,(quint64)debug_event.u.CreateProcessInfo.lpStartAddress,NULL,0x2);

				break;
			}
		case CREATE_THREAD_DEBUG_EVENT:
			PBThreadInfo(debug_event.dwProcessId,debug_event.dwThreadId,(quint64)debug_event.u.CreateThread.lpStartAddress,false,-1,true);
			// Init HW BPs in new Threads
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			PBThreadInfo(debug_event.dwProcessId,debug_event.dwThreadId,(quint64)debug_event.u.CreateThread.lpStartAddress,false,debug_event.u.ExitThread.dwExitCode,false);
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			{
				PBProcInfo(debug_event.dwProcessId,L"",(quint64)debug_event.u.CreateProcessInfo.lpStartAddress,debug_event.u.ExitProcess.dwExitCode,NULL);
				SymCleanup(debug_event.u.CreateProcessInfo.hProcess);

				emit DeletePEManagerObject(L"",debug_event.dwProcessId);

				bool bStillOneRunning = false;
				for(size_t i = 0;i < PIDs.size();i++)
				{
					if(PIDs[i].bRunning && debug_event.dwProcessId != PIDs[i].dwPID)
					{
						bStillOneRunning = true;
						break;
					}

					// delete db object
					//if(PIDs[i].dwPID == debug_event.dwProcessId)
					//{
					//	for(int a = 0; a < fileDBs.size(); a++)
					//	{
					//		if(fileDBs[i]->_PID == debug_event.dwProcessId)
					//		{
					//			delete fileDBs[i];
					//		}
					//	}
					//}
				}

				if(!bStillOneRunning)
					bContinueDebugging = false;
			}
			break;

		case LOAD_DLL_DEBUG_EVENT:
			{
				PTCHAR sDLLFileName = GetFileNameFromHandle(debug_event.u.LoadDll.hFile); 
				PBDLLInfo(sDLLFileName,debug_event.dwProcessId,(quint64)debug_event.u.LoadDll.lpBaseOfDll,true);

				HANDLE hProc = 0;
				int iPid = 0;

				for(size_t i = 0;i < PIDs.size();i++)
				{
					if(PIDs[i].dwPID == debug_event.dwProcessId)
						hProc = PIDs[i].hProc;iPid = i;
				}

				if(PIDs[iPid].bSymLoad && dbgSettings.bAutoLoadSymbols == true)
					SymLoadModuleExW(hProc,NULL,sDLLFileName,0,(quint64)debug_event.u.LoadDll.lpBaseOfDll,0,0,0);
				else
				{
					memset(tcLogString,0x00,LOGBUFFER);
					swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Could not load symbols for DLL: %s",sDLLFileName);
					PBLogInfo();
				}
			}
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			for(size_t i = 0;i < DLLs.size(); i++)
			{
				if(DLLs[i].dwBaseAdr == (quint64)debug_event.u.UnloadDll.lpBaseOfDll && DLLs[i].dwPID == debug_event.dwProcessId)
				{
					PBDLLInfo(DLLs[i].sPath,DLLs[i].dwPID,DLLs[i].dwBaseAdr,false);
					SymUnloadModule64(GetCurrentProcessHandle(DLLs[i].dwPID),DLLs[i].dwBaseAdr);
					break;
				}
			}
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			{
				PTCHAR wMsg = (PTCHAR)malloc(debug_event.u.DebugString.nDebugStringLength * sizeof(TCHAR));
				HANDLE hProcess = NULL;

				for(size_t i = 0;i < PIDs.size(); i++)
				{
					if(PIDs[i].dwPID == debug_event.dwProcessId)
						hProcess = PIDs[i].hProc;
				}
				if(debug_event.u.DebugString.fUnicode)
					ReadProcessMemory(hProcess,debug_event.u.DebugString.lpDebugStringData,wMsg,debug_event.u.DebugString.nDebugStringLength,NULL);
				else
				{
					PCHAR Msg = (PCHAR)malloc(debug_event.u.DebugString.nDebugStringLength * sizeof(CHAR));
					ReadProcessMemory(hProcess,debug_event.u.DebugString.lpDebugStringData,Msg,debug_event.u.DebugString.nDebugStringLength,NULL);	
					mbstowcs(wMsg,Msg,debug_event.u.DebugString.nDebugStringLength);
					free(Msg);
				}
				PBDbgString(wMsg,debug_event.dwProcessId);
				//free(wMsg);
				break;
			}
		case EXCEPTION_DEBUG_EVENT:
			{
				EXCEPTION_DEBUG_INFO exInfo = debug_event.u.Exception;
				bool bIsEP = false,bIsBP = false,bIsKernelBP = false;

				int iPid = 0;
				for(size_t i = 0;i < PIDs.size(); i++)
					if(PIDs[i].dwPID == debug_event.dwProcessId)
						iPid = i;

				if(!CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,debug_event.dwProcessId,false) &&
					!(debug_event.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP && _bSingleStepFlag))
					PBExceptionInfo((quint64)exInfo.ExceptionRecord.ExceptionAddress,exInfo.ExceptionRecord.ExceptionCode,debug_event.dwProcessId,debug_event.dwThreadId);

				switch (exInfo.ExceptionRecord.ExceptionCode)
				{
				case 0x4000001f: // Breakpoint in x86 Process which got executed in a x64 environment
					if(PIDs[iPid].bKernelBP && !PIDs[iPid].bWOW64KernelBP && dbgSettings.dwBreakOnEPMode == 1)
					{
						dwContinueStatus = CallBreakDebugger(&debug_event,0);
						PIDs[iPid].bWOW64KernelBP = true;
						bIsKernelBP = true;
					}
					else
					{
						PIDs[iPid].bWOW64KernelBP = true;
						bIsKernelBP = true;
					}
				case EXCEPTION_BREAKPOINT:
					{
						if(!PIDs[iPid].bKernelBP && dbgSettings.dwBreakOnEPMode == 1)
						{
							dwContinueStatus = CallBreakDebugger(&debug_event,0);
							//if(PIDs.size() > 0)
							PIDs[iPid].bKernelBP = true;
							bIsKernelBP = true;
						}
						else if(PIDs[iPid].bKernelBP)
						{
							if((quint64)exInfo.ExceptionRecord.ExceptionAddress == PIDs[iPid].dwEP)
							{
								bIsEP = true;
								InitBP();
							}

							bIsBP = CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,debug_event.dwProcessId,true);
							if(bIsBP)
							{
								HANDLE hProc = PIDs[iPid].hProc;

								for(size_t i = 0;i < SoftwareBPs.size(); i++)
								{
									if((quint64)exInfo.ExceptionRecord.ExceptionAddress == SoftwareBPs[i].dwOffset && 
										(SoftwareBPs[i].dwPID == debug_event.dwProcessId || SoftwareBPs[i].dwPID == -1))
									{
										WriteProcessMemory(hProc,(LPVOID)SoftwareBPs[i].dwOffset,(LPVOID)&SoftwareBPs[i].bOrgByte,SoftwareBPs[i].dwSize,NULL);
										FlushInstructionCache(hProc,(LPVOID)SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwSize);

										if(SoftwareBPs[i].dwHandle != 0x2)
											SoftwareBPs[i].bRestoreBP = true;
									}
								}

								SetThreadContextHelper(true,true,debug_event.dwThreadId,debug_event.dwProcessId);
								PIDs[iPid].bTrapFlag = true;
								PIDs[iPid].dwBPRestoreFlag = 0x2;

								if(bIsEP && dbgSettings.dwBreakOnEPMode == 3)
									dwContinueStatus = CallBreakDebugger(&debug_event,2);
								else if(bIsEP && dbgSettings.dwBreakOnEPMode == 1)
									dwContinueStatus = CallBreakDebugger(&debug_event,2);
								else
								{
									memset(tcLogString,0x00,LOGBUFFER);
									if(bIsEP)
#ifdef _AMD64_
										swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on EP at %016I64X",(quint64)exInfo.ExceptionRecord.ExceptionAddress);
									else
										swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on Software BP at %016I64X",(quint64)exInfo.ExceptionRecord.ExceptionAddress);
#else
										swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on EP at %08X",(DWORD)exInfo.ExceptionRecord.ExceptionAddress);
									else
										swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on Software BP at %08X",(DWORD)exInfo.ExceptionRecord.ExceptionAddress);
#endif
									PBLogInfo();
									dwContinueStatus = CallBreakDebugger(&debug_event,0);
								}
							}
							//else
							//	dwContinueStatus = CallBreakDebugger(&debug_event,0);
						}
						else
						{
							PIDs[iPid].bKernelBP = true;
							bIsKernelBP = true;
						}
						break;
					}
				case 0x4000001E: // Single Step in x86 Process which got executed in a x64 environment
				case EXCEPTION_SINGLE_STEP:
					{
						bIsBP = CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,debug_event.dwProcessId,true);

						if(bIsBP)
						{
							if(PIDs[iPid].dwBPRestoreFlag == 0x2) // Restore SoftwareBP
							{
								for(size_t i = 0;i < SoftwareBPs.size(); i++)
								{
									if(SoftwareBPs[i].dwHandle == 0x1 && 
										SoftwareBPs[i].bRestoreBP && 
										(SoftwareBPs[i].dwPID == debug_event.dwProcessId || SoftwareBPs[i].dwPID == -1))
									{
										wSoftwareBP(SoftwareBPs[i].dwPID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwHandle,SoftwareBPs[i].dwSize,SoftwareBPs[i].bOrgByte);
										SoftwareBPs[i].bRestoreBP = false;

										memset(tcLogString,0x00,LOGBUFFER);
#ifdef _AMD64_
										swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Restored BP at %016I64X",SoftwareBPs[i].dwOffset);
#else
										swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Restored BP at %08X",(DWORD)SoftwareBPs[i].dwOffset);
#endif
										PBLogInfo();
									}
								}
								PIDs[iPid].bTrapFlag = false;
								PIDs[iPid].dwBPRestoreFlag = NULL;
							}
							else if(PIDs[iPid].dwBPRestoreFlag == 0x4) // Restore MemBP
							{
								for(size_t i = 0;i < MemoryBPs.size(); i++)
								{
									if(MemoryBPs[i].bRestoreBP &&
										MemoryBPs[i].dwHandle == 0x1 &&
										(MemoryBPs[i].dwPID == debug_event.dwProcessId || MemoryBPs[i].dwPID == -1))
										wMemoryBP(MemoryBPs[i].dwPID,MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].dwHandle);
									MemoryBPs[i].bRestoreBP = false;
								}
								PIDs[iPid].bTrapFlag = false;
								PIDs[iPid].dwBPRestoreFlag = NULL;
							}
							else if(PIDs[iPid].dwBPRestoreFlag == 0x8) // Restore HwBp
							{
								for(size_t i = 0;i < HardwareBPs.size();i++)
								{
									if(HardwareBPs[i].bRestoreBP &&
										HardwareBPs[i].dwHandle == 0x1 &&
										(HardwareBPs[i].dwPID == debug_event.dwProcessId || HardwareBPs[i].dwPID == -1))
									{
										wHardwareBP(HardwareBPs[i].dwPID,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSize,HardwareBPs[i].dwSlot,HardwareBPs[i].dwTypeFlag);
										HardwareBPs[i].bRestoreBP = false;
									}
								}
								PIDs[iPid].bTrapFlag = false;
								PIDs[iPid].dwBPRestoreFlag = NULL;
							}
							else if(PIDs[iPid].dwBPRestoreFlag == NULL) // First time hit HwBP
							{
								for(size_t i = 0;i < HardwareBPs.size(); i++)
								{
									if(HardwareBPs[i].dwOffset == (quint64)debug_event.u.Exception.ExceptionRecord.ExceptionAddress &&
										(HardwareBPs[i].dwPID == debug_event.dwProcessId || HardwareBPs[i].dwPID == -1) &&
										dHardwareBP(debug_event.dwProcessId,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSlot))
									{
										memset(tcLogString,0x00,LOGBUFFER);
#ifdef _AMD64_
										swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on Hardware BP at %016I64X",HardwareBPs[i].dwOffset);
#else
										swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on Hardware BP at %08X",(DWORD)HardwareBPs[i].dwOffset);
#endif
										PBLogInfo();

										dwContinueStatus = CallBreakDebugger(&debug_event,0);
										HardwareBPs[i].bRestoreBP = true;
										PIDs[iPid].dwBPRestoreFlag = 0x8;
										PIDs[iPid].bTrapFlag = true;

										SetThreadContextHelper(false,true,debug_event.dwThreadId,debug_event.dwProcessId);
									}
								}
							}
							else
								dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

							if(_bSingleStepFlag)
							{
								_bSingleStepFlag = false;
								bIsBP = true;
								dwContinueStatus = CallBreakDebugger(&debug_event,0);
							}
						}
						else
						{
							if(_bSingleStepFlag)
							{
								_bSingleStepFlag = false;
								bIsBP = true;
								dwContinueStatus = CallBreakDebugger(&debug_event,0);
							}
						}
						break;
					}
				case EXCEPTION_GUARD_PAGE:
					{
						bIsBP = CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,debug_event.dwProcessId,true);
						if(bIsBP)
						{
							SetThreadContextHelper(false,true,debug_event.dwThreadId,debug_event.dwProcessId);
							PIDs[iPid].dwBPRestoreFlag = 0x4;
							PIDs[iPid].bTrapFlag = true;

							for(size_t i = 0;i < MemoryBPs.size();i++)
								if(MemoryBPs[i].dwOffset == (quint64)exInfo.ExceptionRecord.ExceptionAddress)
									MemoryBPs[i].bRestoreBP = true;

							dwContinueStatus = CallBreakDebugger(&debug_event,0);
						}
						break;
					}
				}

				bool bExceptionHandler = false;

				if(!bIsEP && !bIsKernelBP && !bIsBP)
				{
					for (size_t i = 0; i < ExceptionHandler.size();i++)
					{
						if(debug_event.u.Exception.ExceptionRecord.ExceptionCode == ExceptionHandler[i].dwExceptionType)
						{
							bExceptionHandler = true;
							if(ExceptionHandler[i].dwHandler != NULL)
							{
								CustomHandler pCustomHandler = (CustomHandler)ExceptionHandler[i].dwHandler;
								dwContinueStatus = pCustomHandler(&debug_event);
								if(ExceptionHandler[i].dwAction == 0)
									CallBreakDebugger(&debug_event,ExceptionHandler[i].dwAction);
							}
							else
								dwContinueStatus = CallBreakDebugger(&debug_event,ExceptionHandler[i].dwAction);
						}
					}
				}
				if(!bExceptionHandler && !bIsBP && !bIsEP && !bIsKernelBP)
					dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwDefaultExceptionMode);
			}			
			break;
		}
		ContinueDebugEvent(debug_event.dwProcessId,debug_event.dwThreadId,dwContinueStatus);
		dwContinueStatus = DBG_CONTINUE;
	}

	memset(tcLogString,0x00,LOGBUFFER);
	swprintf_s(tcLogString,LOGBUFFERCHAR,L"[-] Debugging finished!");
	PBLogInfo();
	CleanWorkSpace();

	_isDebugging = false;

	emit CleanPEManager();
	emit OnDebuggerTerminated();
}

DWORD clsDebugger::CallBreakDebugger(DEBUG_EVENT *debug_event,DWORD dwHandle)
{
	switch (dwHandle)
	{
	case 0:
		{
			HANDLE hThread = OpenThread(THREAD_GETSET_CONTEXT,false,debug_event->dwThreadId);
			_dwCurPID = debug_event->dwProcessId;
			_dwCurTID = debug_event->dwThreadId;
			_hCurProc = GetCurrentProcessHandle(debug_event->dwProcessId);

#ifdef _AMD64_
			BOOL bIsWOW64 = false;

			if(clsAPIImport::pIsWow64Process)
				clsAPIImport::pIsWow64Process(_hCurProc,&bIsWOW64);
			if(bIsWOW64)
			{
				wowProcessContext.ContextFlags = WOW64_CONTEXT_ALL;
				clsAPIImport::pWow64GetThreadContext(hThread,&wowProcessContext);

				emit OnDebuggerBreak();
				WaitForSingleObject(_hDbgEvent,INFINITE);
				clsAPIImport::pWow64SetThreadContext(hThread,&wowProcessContext);
			}
			else
			{
				ProcessContext.ContextFlags = CONTEXT_ALL;
				GetThreadContext(hThread,&ProcessContext);

				emit OnDebuggerBreak();
				WaitForSingleObject(_hDbgEvent,INFINITE);
				SetThreadContext(hThread,&ProcessContext);
			}

#else
			ProcessContext.ContextFlags = CONTEXT_ALL;
			GetThreadContext(hThread,&ProcessContext);

			emit OnDebuggerBreak();
			WaitForSingleObject(_hDbgEvent,INFINITE);
			SetThreadContext(hThread,&ProcessContext);
#endif
			_dwCurPID = NULL;_dwCurTID = NULL;_hCurProc = NULL;

			CloseHandle(hThread);
			return DBG_EXCEPTION_HANDLED;
		}
	case 1:
		return DBG_EXCEPTION_NOT_HANDLED;
	case 2:
		return DBG_CONTINUE;
	default:
		return DBG_EXCEPTION_NOT_HANDLED;
	}
}

bool clsDebugger::wSoftwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwKeep,DWORD dwSize,BYTE &bOrgByte)
{
	if(dwOffset == 0)
		return false;

	SIZE_T dwBytesWritten,dwBytesRead;
	BYTE bNew = 0xCC,bOld;

	HANDLE hPID = _pi.hProcess;

	if(dwPID != -1)
	{
		for(size_t i = 0;i < PIDs.size();i++)
		{
			if(dwPID == PIDs[i].dwPID)
				hPID = PIDs[i].hProc;
		}
	}

	if(ReadProcessMemory(hPID,(LPVOID)dwOffset,(LPVOID)&bOld,dwSize,&dwBytesRead))
	{
		if(bOld != 0xCC)
		{
			bOrgByte = bOld;
			if(WriteProcessMemory(hPID,(LPVOID)dwOffset,(LPVOID)&bNew,dwSize,&dwBytesWritten))
				return true;
		}
		else
		{
			//EP Entrypoint
			if(WriteProcessMemory(hPID,(LPVOID)dwOffset,(LPVOID)&bOrgByte,dwSize,&dwBytesWritten))
				return true;
		}
	}
	return false;
}

bool clsDebugger::wMemoryBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize,DWORD dwKeep)
{
	MEMORY_BASIC_INFORMATION MBI;
	SYSTEM_INFO sysInfo;
	DWORD dwOldProtection;

	HANDLE hPID = _pi.hProcess;

	for(size_t i = 0;i < PIDs.size(); i++)
		if(PIDs[i].dwPID == dwPID)
			hPID = PIDs[i].hProc;

	GetSystemInfo(&sysInfo);
	VirtualQueryEx(hPID,(LPVOID)dwOffset,&MBI,sizeof(MBI));

	if(!VirtualProtectEx(hPID,(LPVOID)dwOffset,dwSize,MBI.Protect | PAGE_GUARD,&dwOldProtection))
		return false;
	return true;
}

bool clsDebugger::wHardwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize,DWORD dwSlot,DWORD dwTypeFlag)
{
	int iBP = NULL;
	DWORD dwThreadCounter = 0;
	THREADENTRY32 threadEntry32;
	threadEntry32.dwSize = sizeof(THREADENTRY32);
	CONTEXT cTT;
	cTT.ContextFlags = CONTEXT_ALL;
	HANDLE hThread = INVALID_HANDLE_VALUE;

	if(dwPID == -1 || dwPID == 0)
		dwPID = _pi.dwProcessId;

	if(!(dwSize == 1 || dwSize == 2 || dwSize == 4))
		return false; 

	if(!(dwTypeFlag == DR_EXECUTE || dwTypeFlag == DR_READ || dwTypeFlag == DR_WRITE))
		return false;

	for(size_t i = 0;i < HardwareBPs.size();i++)
		if(HardwareBPs[i].dwOffset == dwOffset)
			iBP = i;

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,dwPID);
	if(hProcessSnap == INVALID_HANDLE_VALUE)
		return false;

	if(!Thread32First(hProcessSnap,&threadEntry32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	do{
		if(dwPID == threadEntry32.th32OwnerProcessID)
			hThread = OpenThread(THREAD_ALL_ACCESS,false,threadEntry32.th32ThreadID);

		if(hThread != INVALID_HANDLE_VALUE)
		{
			dwThreadCounter++;
			GetThreadContext(hThread,&cTT);

			switch(HardwareBPs[iBP].dwSlot)
			{
			case 0:
				cTT.Dr0 = HardwareBPs[iBP].dwOffset;
				break;
			case 1:
				cTT.Dr1 = HardwareBPs[iBP].dwOffset;
				break;
			case 2:
				cTT.Dr2 = HardwareBPs[iBP].dwOffset;
				break;
			case 3:
				cTT.Dr3 = HardwareBPs[iBP].dwOffset;
				break;
			}

			cTT.Dr7 |= 1 << (HardwareBPs[iBP].dwSlot * 2);
			cTT.Dr7 |= dwTypeFlag << ((HardwareBPs[iBP].dwSlot * 4) + 16);
			cTT.Dr7 |= (dwSize - 1) << ((HardwareBPs[iBP].dwSlot * 4) + 18);

			SetThreadContext(hThread,&cTT);
		}
	}while(Thread32Next(hProcessSnap,&threadEntry32));

	memset(tcLogString,0x00,LOGBUFFER);
	swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] New HardwareBP in %i Threads placed at %X in Slot No. %i",dwThreadCounter,HardwareBPs[iBP].dwOffset,HardwareBPs[iBP].dwSlot);
	PBLogInfo();

	CloseHandle(hProcessSnap);
	return true;
}

bool clsDebugger::StepOver(quint64 dwNewOffset)
{
	for (vector<BPStruct>::iterator it = SoftwareBPs.begin(); it != SoftwareBPs.end();++it) {
		if(it->dwHandle == 0x2)
		{
			dSoftwareBP(it->dwPID,it->dwOffset,it->dwSize,it->bOrgByte);
			SoftwareBPs.erase(it);
			it = SoftwareBPs.begin();
		}
		if(SoftwareBPs.size() <= 0)
			break;
	}

	BPStruct newBP;
	newBP.dwOffset = dwNewOffset;
	newBP.dwHandle = 0x2;
	newBP.dwSize = 0x1;
	newBP.bOrgByte = NULL;
	newBP.dwPID = _dwCurPID;

	wSoftwareBP(newBP.dwPID,newBP.dwOffset,newBP.dwHandle,newBP.dwSize,newBP.bOrgByte);

	if(newBP.bOrgByte != 0xCC)
	{
		SoftwareBPs.push_back(newBP);
		PulseEvent(_hDbgEvent);
		return true;
	}
	return false;
}

bool clsDebugger::StepIn()
{
	_bSingleStepFlag = true;
	ProcessContext.EFlags |= 0x100;
	return PulseEvent(_hDbgEvent);
}

bool clsDebugger::CheckProcessState(DWORD dwPID)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 procEntry32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

	if(hProcessSnap == INVALID_HANDLE_VALUE)
		return false;
	procEntry32.dwSize = sizeof(PROCESSENTRY32);

	if(!Process32First(hProcessSnap,&procEntry32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	do{
		if(procEntry32.th32ProcessID == dwPID)
			return true;
	}while(Process32Next(hProcessSnap,&procEntry32));

	CloseHandle(hProcessSnap);
	return false;
}

bool clsDebugger::ShowCallStack()
{

	if(_dwCurTID == 0 || _dwCurPID == 0)
		return false;

	HANDLE hProc,hThread = OpenThread(THREAD_GETSET_CONTEXT,false,_dwCurTID);
	PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)malloc(sizeof(SYMBOL_INFOW) + MAX_SYM_NAME);
	DWORD dwMaschineMode = NULL;
	quint64 dwDisplacement;
	LPVOID pContext;
	IMAGEHLP_MODULEW64 imgMod = {0};
	STACKFRAME64 stackFr = {0};

	int iPid = 0;
	for(size_t i = 0;i < PIDs.size(); i++)
	{
		if(PIDs[i].dwPID == _dwCurPID)
			hProc = PIDs[i].hProc;iPid = i;
	}

	if(!PIDs[iPid].bSymLoad)
		PIDs[iPid].bSymLoad = SymInitialize(hProc,NULL,false);

#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	HANDLE hProcess = NULL;

	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(_hCurProc,&bIsWOW64);

	if(bIsWOW64)
	{
		dwMaschineMode = IMAGE_FILE_MACHINE_I386;
		WOW64_CONTEXT wowContext;
		pContext = &wowContext;
		wowContext.ContextFlags = WOW64_CONTEXT_ALL;
		clsAPIImport::pWow64GetThreadContext(hThread,&wowContext);

		stackFr.AddrPC.Mode = AddrModeFlat;
		stackFr.AddrFrame.Mode = AddrModeFlat;
		stackFr.AddrStack.Mode = AddrModeFlat;
		stackFr.AddrPC.Offset = wowContext.Eip;
		stackFr.AddrFrame.Offset = wowContext.Ebp;
		stackFr.AddrStack.Offset = wowContext.Esp;
	}
	else
	{
		dwMaschineMode = IMAGE_FILE_MACHINE_AMD64;
		CONTEXT context;
		pContext = &context;
		context.ContextFlags = CONTEXT_ALL;
		GetThreadContext(hThread, &context);

		stackFr.AddrPC.Mode = AddrModeFlat;
		stackFr.AddrFrame.Mode = AddrModeFlat;
		stackFr.AddrStack.Mode = AddrModeFlat;
		stackFr.AddrPC.Offset = context.Rip;
		stackFr.AddrFrame.Offset = context.Rbp;
		stackFr.AddrStack.Offset = context.Rsp;	
	}
#else
	dwMaschineMode = IMAGE_FILE_MACHINE_I386;
	CONTEXT context;
	pContext = &context;
	context.ContextFlags = CONTEXT_ALL;
	GetThreadContext(hThread, &context);

	stackFr.AddrPC.Mode = AddrModeFlat;
	stackFr.AddrFrame.Mode = AddrModeFlat;
	stackFr.AddrStack.Mode = AddrModeFlat;
	stackFr.AddrPC.Offset = context.Eip;
	stackFr.AddrFrame.Offset = context.Ebp;
	stackFr.AddrStack.Offset = context.Esp;
#endif

	BOOL bSuccess;
	do
	{
		bSuccess = StackWalk64(dwMaschineMode,hProc,hThread,&stackFr,pContext,NULL,SymFunctionTableAccess64,SymGetModuleBase64,0);

		if(!bSuccess)        
			break;

		memset(&imgMod,0x00,sizeof(imgMod));
		imgMod.SizeOfStruct = sizeof(imgMod);
		bSuccess = SymGetModuleInfoW64(hProc,stackFr.AddrPC.Offset, &imgMod);

		memset(pSymbol,0,sizeof(SYMBOL_INFOW) + MAX_SYM_NAME);
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
		pSymbol->MaxNameLen = MAX_SYM_NAME;

		quint64 dwStackAddr = stackFr.AddrStack.Offset;

		quint64 dwEIP = stackFr.AddrPC.Offset;
		bSuccess = SymFromAddrW(hProc,dwEIP,&dwDisplacement,pSymbol);
		wstring sFuncName = pSymbol->Name;
		bSuccess = SymGetModuleInfoW64(hProc,dwEIP, &imgMod);
		wstring sFuncMod = imgMod.ModuleName;

		quint64 dwReturnTo = stackFr.AddrReturn.Offset;
		bSuccess = SymFromAddrW(hProc,dwReturnTo,&dwDisplacement,pSymbol);
		wstring sReturnToFunc = pSymbol->Name;
		bSuccess = SymGetModuleInfoW64(hProc,dwReturnTo,&imgMod);

		wstring sReturnToMod = imgMod.ModuleName;

		IMAGEHLP_LINEW64 imgSource;
		imgSource.SizeOfStruct = sizeof(imgSource);
		bSuccess = SymGetLineFromAddrW64(hProc,stackFr.AddrPC.Offset,(PDWORD)&dwDisplacement,&imgSource);

		if(bSuccess)
			emit OnCallStack(dwStackAddr,
			dwReturnTo,sReturnToFunc,sReturnToMod,
			dwEIP,sFuncName,sFuncMod,
			imgSource.FileName,imgSource.LineNumber);
		else
			emit OnCallStack(dwStackAddr,
			dwReturnTo,sReturnToFunc,sReturnToMod,
			dwEIP,sFuncName,sFuncMod,
			L"",0);

	}while(stackFr.AddrReturn.Offset != 0);

	free(pSymbol);
	CloseHandle(hThread);
	return false;
}

bool clsDebugger::AddBreakpointToList(DWORD dwBPType,DWORD dwTypeFlag,DWORD dwPID,quint64 dwOffset,DWORD dwSlot,DWORD dwKeep)
{
	bool	bExists = false,
		bRetValue = false;

	for(size_t i = 0;i < SoftwareBPs.size();i++)
	{
		if(SoftwareBPs[i].dwOffset == dwOffset/* && SoftwareBPs[i].dwPID == dwPID */)
		{
			bExists = true;
			break;
		}
	}
	for(size_t i = 0;i < MemoryBPs.size();i++)
	{
		if(MemoryBPs[i].dwOffset == dwOffset /* && MemoryBPs[i].dwPID == dwPID*/)
		{
			bExists = true;
			break;
		}
	}
	for(size_t i = 0;i < HardwareBPs.size();i++)
	{
		if(HardwareBPs[i].dwOffset == dwOffset /* && HardwareBPs[i].dwPID == dwPID */)
		{
			bExists = true;
			break;
		}
	}

	if(!bExists)
	{
		switch(dwBPType)
		{
		case 0:
			{
				//SoftwareBP
				BPStruct newBP;

				newBP.dwOffset = dwOffset;
				newBP.dwHandle = dwKeep;
				newBP.dwSize = 1;
				newBP.bOrgByte = NULL;
				newBP.dwPID = dwPID;
				newBP.bRestoreBP = false;
				newBP.dwTypeFlag = dwTypeFlag;

				SoftwareBPs.push_back(newBP);
				emit OnNewBreakpointAdded(newBP,0);
				bRetValue = true;

				break;
			}
		case 1:
			{
				//MemoryBP
				BPStruct newBP;

				newBP.dwOffset = dwOffset;
				newBP.dwHandle = dwKeep;
				newBP.dwSize = 1;
				newBP.dwPID = dwPID;
				newBP.bOrgByte = NULL;
				newBP.bRestoreBP = false;
				newBP.dwTypeFlag = dwTypeFlag;

				MemoryBPs.push_back(newBP);
				emit OnNewBreakpointAdded(newBP,1);
				bRetValue = true;

				break;
			}
		case 2:
			{
				//HardwareBP
				if(HardwareBPs.size() == 4)
					break;

				BPStruct newBP;
				newBP.dwOffset = dwOffset;
				newBP.dwHandle = dwKeep;
				newBP.dwSize = 1;
				newBP.dwPID = dwPID;
				newBP.bOrgByte = NULL;
				newBP.bRestoreBP = false;
				newBP.dwTypeFlag = dwTypeFlag;

				bool bSlot1 = false,bSlot2 = false,bSlot3 = false,bSlot4 = false;
				for(size_t i = 0;i < HardwareBPs.size();i++)
				{
					switch(HardwareBPs[i].dwSlot)
					{
					case 0:
						bSlot1 = true;
						break;
					case 1:
						bSlot2 = true;
						break;
					case 2:
						bSlot3 = true;
						break;
					case 3:
						bSlot4 = true;
						break;
					}
				}
				if(!bSlot4) newBP.dwSlot = 3;
				if(!bSlot3) newBP.dwSlot = 2;
				if(!bSlot2) newBP.dwSlot = 1;
				if(!bSlot1) newBP.dwSlot = 0;

				HardwareBPs.push_back(newBP);
				emit OnNewBreakpointAdded(newBP,2);
				bRetValue = true;

				break;
			}
		}
	}
	if(_isDebugging)
		InitBP();

	return bRetValue;
}

bool clsDebugger::InitBP()
{
	for(size_t i = 0;i < SoftwareBPs.size(); i++)
		wSoftwareBP(SoftwareBPs[i].dwPID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwHandle,SoftwareBPs[i].dwSize,SoftwareBPs[i].bOrgByte);

	for(size_t i = 0;i < MemoryBPs.size(); i++)
		wMemoryBP(MemoryBPs[i].dwPID,MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].dwHandle);

	for(size_t i = 0;i < HardwareBPs.size(); i++)
		wHardwareBP(HardwareBPs[i].dwPID,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSize,HardwareBPs[i].dwSlot,HardwareBPs[i].dwTypeFlag);
	return true;
}

bool clsDebugger::DetachFromProcess()
{
	_NormalDebugging = true;
	_isDebugging = false;
	_bStopDebugging = true;

	RemoveBPs();

	for(size_t d = 0;d < PIDs.size();d++)
	{
		if(!CheckProcessState(PIDs[d].dwPID))
			break;
		DebugBreakProcess(PIDs[d].hProc);
		DebugActiveProcessStop(PIDs[d].dwPID);
		PulseEvent(_hDbgEvent);
	}

	emit OnDebuggerTerminated();
	return true;
}

bool clsDebugger::AttachToProcess(DWORD dwPID)
{
	CleanWorkSpace();
	_NormalDebugging = false;_dwPidToAttach = dwPID;
	return true;
}

bool clsDebugger::RemoveBPFromList(quint64 dwOffset,DWORD dwType) //,DWORD dwPID)
{ 
	switch(dwType)
	{
	case 0:
		for (vector<BPStruct>::iterator it = SoftwareBPs.begin();it != SoftwareBPs.end(); ++it)
		{
			if(it->dwOffset == dwOffset /* && it->dwPID == dwPID */)
			{
				dSoftwareBP(it->dwPID,it->dwOffset,it->dwSize,it->bOrgByte);
				SoftwareBPs.erase(it);
				it = SoftwareBPs.begin();

				if(it == SoftwareBPs.end())
					break;
			}
		}
		break;

	case 1:
		for (vector<BPStruct>::iterator it = MemoryBPs.begin();it != MemoryBPs.end(); ++it)
		{
			if(it->dwOffset == dwOffset /* && it->dwPID == dwPID */)
			{
				dMemoryBP(it->dwPID,it->dwOffset,it->dwSize);
				MemoryBPs.erase(it);
				it = MemoryBPs.begin();

				if(it == MemoryBPs.end())
					break;
			}
		}
		break;

	case 2:
		for (vector<BPStruct>::iterator it = HardwareBPs.begin();it != HardwareBPs.end(); ++it)
		{
			if(it->dwOffset == dwOffset /* && it->dwPID == dwPID */)
			{
				dHardwareBP(it->dwPID,it->dwOffset,it->dwSlot);
				HardwareBPs.erase(it);
				it = HardwareBPs.begin();

				if(it == HardwareBPs.end())
					break;
			}
		}
		break;
	}
	return true;
}

bool clsDebugger::dHardwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwSlot)
{
	HANDLE hProcessSnap;
	THREADENTRY32 threadEntry32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,dwPID);

	if(hProcessSnap == INVALID_HANDLE_VALUE)
		return false;

	threadEntry32.dwSize = sizeof(THREADENTRY32);

	if(!Thread32First(hProcessSnap,&threadEntry32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	do{
		CONTEXT cTT;
		cTT.ContextFlags = CONTEXT_ALL;
		HANDLE hThread = INVALID_HANDLE_VALUE;

		if(dwPID == threadEntry32.th32OwnerProcessID)
			hThread = OpenThread(THREAD_GETSET_CONTEXT,false,threadEntry32.th32ThreadID);

		if(hThread != INVALID_HANDLE_VALUE)
		{
			GetThreadContext(hThread,&cTT);

			if(cTT.Dr0 == dwOffset) cTT.Dr0 = 0;
			if(cTT.Dr1 == dwOffset) cTT.Dr1 = 0;
			if(cTT.Dr2 == dwOffset) cTT.Dr2 = 0;
			if(cTT.Dr3 == dwOffset) cTT.Dr3 = 0;

			cTT.Dr7 &= ~(1 << (dwSlot * 2));
			cTT.Dr7 &= ~(3 << ((dwSlot * 4) + 16));
			cTT.Dr7 &= ~(3 << ((dwSlot * 4) + 18));

			SetThreadContext(hThread,&cTT);
		}
	}while(Thread32Next(hProcessSnap,&threadEntry32));
	CloseHandle(hProcessSnap);
	return true;
}

bool clsDebugger::dSoftwareBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize,BYTE btOrgByte)
{
	if(dwOffset == 0 && btOrgByte == 0x00)
		return false;

	SIZE_T dwBytesWritten,dwBytesRead;
	HANDLE hPID = _pi.hProcess;

	if(dwPID != -1)
	{
		for(size_t i = 0;i < PIDs.size();i++)
		{
			if(dwPID == PIDs[i].dwPID)
				hPID = PIDs[i].hProc;
		}
	}

	if(WriteProcessMemory(hPID,(LPVOID)dwOffset,(LPVOID)&btOrgByte,dwSize,&dwBytesWritten))
		return true;
	return false;
}

bool clsDebugger::dMemoryBP(DWORD dwPID,quint64 dwOffset,DWORD dwSize)
{
	MEMORY_BASIC_INFORMATION MBI;
	SYSTEM_INFO sysInfo;
	DWORD dwOldProtection;

	HANDLE hPID = _pi.hProcess;

	for(size_t i = 0;i < PIDs.size(); i++)
		if(PIDs[i].dwPID == dwPID)
			hPID = PIDs[i].hProc;

	GetSystemInfo(&sysInfo);
	VirtualQueryEx(hPID,(LPVOID)dwOffset,&MBI,sizeof(MBI));

	if(!VirtualProtectEx(hPID,(LPVOID)dwOffset,dwSize,MBI.Protect & PAGE_GUARD,&dwOldProtection))
		return false;
	return true;
}

bool clsDebugger::IsTargetSet()
{
	if(_sTarget.length() > 0)
		return true;
	return false;
}

void clsDebugger::ClearTarget()
{
	_sTarget.clear();
}

DWORD clsDebugger::GetCurrentPID()
{
	return _dwCurPID;
}

DWORD clsDebugger::GetCurrentTID()
{
	return _dwCurTID;
}

bool clsDebugger::RemoveBPs()
{
	for(size_t i = 0; i < SoftwareBPs.size(); i++)
		dSoftwareBP(SoftwareBPs[i].dwPID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwSize,SoftwareBPs[i].bOrgByte);

	for(size_t i = 0; i < MemoryBPs.size(); i++)
		dSoftwareBP(MemoryBPs[i].dwPID,MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].bOrgByte);

	for(size_t i = 0; i < HardwareBPs.size(); i++)	
		dSoftwareBP(HardwareBPs[i].dwPID,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSize,HardwareBPs[i].bOrgByte);

	SoftwareBPs.clear();
	HardwareBPs.clear();
	MemoryBPs.clear();
	return true;
}

bool clsDebugger::CheckIfExceptionIsBP(quint64 dwExceptionOffset,DWORD dwPID,bool bClearTrapFlag)
{
	int iPID = NULL;
	for(size_t i = 0;i < PIDs.size();i++)
		if(PIDs[i].dwPID == dwPID)
			iPID = i;

	if(PIDs[iPID].bTrapFlag)
	{
		if(bClearTrapFlag)
			PIDs[iPID].bTrapFlag = false;
		return true;
	}

	for(size_t i = 0;i < SoftwareBPs.size();i++)
		if(dwExceptionOffset == SoftwareBPs[i].dwOffset && (SoftwareBPs[i].dwPID == dwPID || SoftwareBPs[i].dwPID == -1))
			return true;
	for(size_t i = 0;i < MemoryBPs.size();i++)
		if(dwExceptionOffset == MemoryBPs[i].dwOffset && (MemoryBPs[i].dwPID == dwPID || MemoryBPs[i].dwPID == -1))
			return true;
	for(size_t i = 0;i < HardwareBPs.size();i++)
		if(dwExceptionOffset == HardwareBPs[i].dwOffset && (HardwareBPs[i].dwPID == dwPID || HardwareBPs[i].dwPID == -1))
			return true;
	return false;
}

bool clsDebugger::SuspendProcess(DWORD dwPID,bool bSuspend)
{
	HANDLE hProcessSnap;
	THREADENTRY32 threadEntry32;
	threadEntry32.dwSize = sizeof(THREADENTRY32);

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,dwPID);

	if(hProcessSnap == INVALID_HANDLE_VALUE)
		return false;

	if(!Thread32First(hProcessSnap,&threadEntry32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}
	do{
		HANDLE hThread = INVALID_HANDLE_VALUE;

		if(dwPID == threadEntry32.th32OwnerProcessID)
			hThread = OpenThread(THREAD_ALL_ACCESS,false,threadEntry32.th32ThreadID);

		if(hThread != INVALID_HANDLE_VALUE)
		{
			if(bSuspend)
				SuspendThread(hThread);
			else
				ResumeThread(hThread);
		}

	}while(Thread32Next(hProcessSnap,&threadEntry32));

	return true;
}

void clsDebugger::CustomExceptionAdd(DWORD dwExceptionType,DWORD dwAction,quint64 dwHandler)
{
	if(dwExceptionType != EXCEPTION_SINGLE_STEP && dwExceptionType != EXCEPTION_BREAKPOINT)
	{
		customException custEx;
		custEx.dwAction = dwAction;
		custEx.dwExceptionType = dwExceptionType;
		custEx.dwHandler = dwHandler;
		ExceptionHandler.push_back(custEx);
	}
}

void clsDebugger::CustomExceptionRemove(DWORD dwExceptionType)
{
	for (vector<customException>::iterator it = ExceptionHandler.begin(); it != ExceptionHandler.end(); ++it) {
		if(it->dwExceptionType == dwExceptionType)
		{
			ExceptionHandler.erase(it);
			it = ExceptionHandler.begin();
		}
	}
}

void clsDebugger::CustomExceptionRemoveAll()
{
	ExceptionHandler.clear();
}

bool clsDebugger::EnableDebugFlag()
{
	TOKEN_PRIVILEGES tkpNewPriv;
	LUID luid;
	HANDLE hToken = NULL;

	if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
		return false;

	if (!LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&luid))
		return false;

	tkpNewPriv.PrivilegeCount = 1;
	tkpNewPriv.Privileges[0].Luid = luid;
	tkpNewPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(hToken,0,&tkpNewPriv,0,0,0))
		return false;
	return true;
}

bool clsDebugger::ReadMemoryFromDebugee(DWORD dwPID,quint64 dwAddress,DWORD dwSize,LPVOID lpBuffer)
{
	for(size_t i = 0;i < PIDs.size();i++)
		if(PIDs[i].dwPID == dwPID)
			return ReadProcessMemory(PIDs[i].hProc,(LPVOID)dwAddress,lpBuffer,dwSize,NULL);
	return false;
}

bool clsDebugger::WriteMemoryFromDebugee(DWORD dwPID,quint64 dwAddress,DWORD dwSize,LPVOID lpBuffer)
{
	for(size_t i = 0;i < PIDs.size();i++)
		if(PIDs[i].dwPID == dwPID)
			return WriteProcessMemory(PIDs[i].hProc,(LPVOID)dwAddress,lpBuffer,dwSize,NULL);
	return false;
}

bool clsDebugger::SetThreadContextHelper(bool bDecIP,bool bSetTrapFlag, DWORD dwThreadID, DWORD dwPID)
{
	HANDLE hThread = OpenThread(THREAD_GETSET_CONTEXT,false,dwThreadID);
	if(hThread == INVALID_HANDLE_VALUE)
		return false;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	HANDLE hProcess = NULL;

	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(_hCurProc,&bIsWOW64);

	if(bIsWOW64)
	{
		WOW64_CONTEXT wowcTT;
		wowcTT.ContextFlags = WOW64_CONTEXT_ALL;
		clsAPIImport::pWow64GetThreadContext(hThread,&wowcTT);

		if(bDecIP)
			wowcTT.Eip--;

		if(bSetTrapFlag)
			wowcTT.EFlags |= 0x100;

		clsAPIImport::pWow64SetThreadContext(hThread,&wowcTT);
	}
	else
	{
		CONTEXT cTT;
		cTT.ContextFlags = CONTEXT_ALL;
		cTT.ContextFlags = CONTEXT_ALL;
		GetThreadContext(hThread,&cTT);

		if(bDecIP)
			cTT.Rip--;

		if(bSetTrapFlag)
			cTT.EFlags |= 0x100;

		SetThreadContext(hThread,&cTT);
	}

#else
	CONTEXT cTT;
	cTT.ContextFlags = CONTEXT_ALL;
	cTT.ContextFlags = CONTEXT_ALL;
	GetThreadContext(hThread,&cTT);

	if(bDecIP)
		cTT.Eip--;

	if(bSetTrapFlag)
		cTT.EFlags |= 0x100;

	SetThreadContext(hThread,&cTT);
#endif

	CloseHandle(hThread);
	return true;
}

HANDLE clsDebugger::GetCurrentProcessHandle(DWORD dwPID)
{
	for(size_t i = 0;i < PIDs.size();i++)
	{
		if(PIDs[i].dwPID == dwPID)
			return PIDs[i].hProc;
	}
	return NULL;
}

HANDLE clsDebugger::GetCurrentProcessHandle()
{
	return _hCurProc;
}

bool clsDebugger::IsOffsetAnBP(quint64 Offset)
{
	for(size_t i = 0; i < pThis->SoftwareBPs.size(); i++)
		if(pThis->SoftwareBPs[i].dwOffset == Offset && pThis->SoftwareBPs[i].dwHandle != 0x2)
			return true;

	for(size_t i = 0; i < pThis->MemoryBPs.size(); i++)
		if(pThis->MemoryBPs[i].dwOffset == Offset)
			return true;

	for(size_t i = 0; i < pThis->HardwareBPs.size(); i++)
		if(pThis->HardwareBPs[i].dwOffset == Offset)
			return true;
	return false;
}

bool clsDebugger::IsOffsetEIP(quint64 Offset)
{
#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(pThis->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64)
	{
		if(pThis->wowProcessContext.Eip == Offset)
			return true;
	}
	else
	{
		if(pThis->ProcessContext.Rip == Offset)
			return true;
	}
#else
	if(pThis->ProcessContext.Eip == Offset)
		return true;
#endif	
	return false;
}

void clsDebugger::SetCommandLine(std::wstring CommandLine)
{
	_sCommandLine = CommandLine;
}

std::wstring clsDebugger::GetCMDLine()
{
	return _sCommandLine;
}

void clsDebugger::ClearCommandLine()
{
	_sCommandLine.clear();
}