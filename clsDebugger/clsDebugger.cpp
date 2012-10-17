#include "clsDebugger.h"

clsDebugger::clsDebugger()
{
	_NormalDebugging = true;
	_isDebugging = false;
	dwOnThread = NULL;dwOnPID = NULL;dwOnDll = NULL;dwOnException = NULL;dwOnLog = NULL;dwOnDbgString = NULL;dwOnCallStack = NULL;
	hDebuggingHandle = CreateEvent(NULL,false,false,L"clsDebugger");
	tcLogString = (PTCHAR)malloc(LOGBUFFER);
	clsDebuggerSettings tempSet = {false,false,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	dbgSettings = tempSet;
	EnableDebugFlag();
}

clsDebugger::clsDebugger(wstring sTarget)
{
	_sTarget = sTarget;
	_NormalDebugging = true;
	_isDebugging = false;
	dwOnThread = NULL;dwOnPID = NULL;dwOnDll = NULL;dwOnException = NULL;dwOnLog = NULL;dwOnDbgString = NULL;dwOnCallStack = NULL;
	hDebuggingHandle = CreateEvent(NULL,false,false,L"clsDebugger");
	tcLogString = (PTCHAR)malloc(LOGBUFFER);
	clsDebuggerSettings tempSet = {false,false,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	dbgSettings = tempSet;
	EnableDebugFlag();
}

clsDebugger::~clsDebugger(void)
{
	CleanWorkSpace();
	free(tcLogString);
}

bool clsDebugger::IsValidFile()
{
	return true;
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
		SymCleanup(PIDs[i].hSymInfo);
	for(size_t i = 0; i < DLLs.size(); i++)
		free(DLLs[i].sPath);
	PIDs.clear();
	DbgStrings.clear();
	DLLs.clear();
	Threads.clear();
}

bool clsDebugger::SuspendDebuggingAll()
{
	for(size_t i = 0;i < PIDs.size();i++)
		SuspendDebugging(PIDs[i].dwPID);
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
					hProcess = PIDs[i].hSymInfo;
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
			hProcess = PIDs[i].hSymInfo;iPid = i;
	}
	if(CheckProcessState(dwPID))
	{
		if(TerminateProcess(hProcess,0))
		{
			PBProcInfo(PIDs[iPid].dwPID,PIDs[iPid].sFileName,PIDs[iPid].dwEP,0x0,hProcess);
			if(PIDs.size() == 1)
			{
				_isDebugging = false;
				PulseEvent(_hDbgEvent);
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
	StopDebugging(true);
	StartDebugging();
	return true;
}

bool clsDebugger::PBThreadInfo(DWORD dwPID,DWORD dwTID,DWORD dwEP,bool bSuspended,DWORD dwExitCode,BOOL bNew)
{
	bool bFound = false;

	for(size_t i = 0;i < Threads.size();i++)
	{
		if(Threads[i].dwTID == dwTID && Threads[i].dwPID == dwPID)
		{
			Threads[i].dwExitCode = dwExitCode;
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

		Threads.push_back(newTID);
	}

	if(dwOnThread != NULL)
		dwOnThread(dwPID,dwTID,dwEP,bSuspended,dwExitCode,bFound);

	memset(tcLogString,0x00,LOGBUFFER);

	if(bFound)
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[-] Exit Thread(%X) in Process(%X) with Exitcode: %X",dwTID,dwPID,dwExitCode);
	else
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] New Thread(%X) in Process(%X) with Entrypoint: %X",dwTID,dwPID,dwEP);
	PBLogInfo();

	return true;
}

bool clsDebugger::PBProcInfo(DWORD dwPID,PTCHAR sFileName,DWORD dwEP,DWORD dwExitCode,HANDLE hSymInfo)
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
		newPID.hSymInfo = hSymInfo;
		newPID.bKernelBP = false;
		newPID.bSymLoad = false;
		newPID.bRunning = true;
		newPID.dwBPRestoreFlag = NULL;
		PIDs.push_back(newPID);
	}

	if(dwOnPID != NULL)
		dwOnPID(dwPID,sFileName,dwExitCode,dwEP,bFound);

	memset(tcLogString,0x00,LOGBUFFER);

	if(bFound)
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[-] Exit Process(%X) with Exitcode: 0x%08X",dwPID,dwExitCode);
	else
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] New Process(%X) with Entrypoint: 0x%08X",dwPID,dwEP);
	PBLogInfo();

	return true;
}

bool clsDebugger::PBExceptionInfo(DWORD dwExceptionOffset,DWORD dwExceptionCode,DWORD dwPID,DWORD dwTID)
{
	wstring sModName,sFuncName;

	_dwCurPID = dwPID;
	LoadSymbolForAddr(sFuncName,sModName,dwExceptionOffset);
	_dwCurPID = 0;

	if(dwOnException != NULL)
		dwOnException(sFuncName,sModName,dwExceptionOffset,dwExceptionCode,dwPID,dwTID);

	memset(tcLogString,0x00,LOGBUFFER);
	swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] %s@%s ExceptionCode: %08X ExceptionOffset: %08X PID: %X TID: %X",
		sFuncName.c_str(),
		sModName.c_str(),
		dwExceptionCode,
		dwExceptionOffset,
		dwPID,
		dwTID);

	PBLogInfo();
	return true;
}

bool clsDebugger::PBDLLInfo(PTCHAR sDLLPath,DWORD dwPID,DWORD dwEP,bool bLoaded)
{
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

	if(dwOnDll != NULL)
		dwOnDll(sDLLPath,dwPID,dwEP,bLoaded);

	memset(tcLogString,0x00,LOGBUFFER);
	if(bLoaded)
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] PID(%X) - Loaded DLL: %s Entrypoint: %08X",dwPID,sDLLPath,dwEP);
	else
	{
		HANDLE hProc = NULL;
		for(size_t i = 0;i < PIDs.size();i++)
			if(PIDs[i].dwPID == dwPID)
				hProc = PIDs[i].hSymInfo;

		SymUnloadModule64(hProc,dwEP);
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] PID(%X) - Unloaded DLL: %s",dwPID,sDLLPath);
	}
	PBLogInfo();

	return true;
}

bool clsDebugger::PBDbgString(PTCHAR sMessage,DWORD dwPID)
{
	DbgStrings.push_back(sMessage);

	if(dwOnDbgString != NULL)
		dwOnDbgString(sMessage,dwPID);
	
	memset(tcLogString,0x00,LOGBUFFER);
	swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] PID(%X) - DbgString: %s",dwPID,sMessage);
	PBLogInfo();
	free(sMessage);
	return true;
}

bool clsDebugger::PBLogInfo()
{
	time_t tTime;
	tm* timeInfo;
	PTCHAR tcTempLog = (PTCHAR)malloc(LOGBUFFER + 128);
	time(&tTime);
	timeInfo = localtime(&tTime);

	swprintf_s(tcTempLog,LOGBUFFERCHAR + 64,L"[%i:%i:%i] %s",timeInfo->tm_hour,timeInfo->tm_min,timeInfo->tm_sec,tcLogString);

	if(dwOnLog != NULL)
		dwOnLog(tcTempLog);
	free(tcTempLog);
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
	
	PTCHAR tcTemp = (PTCHAR)malloc(255 * sizeof(TCHAR));
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

bool clsDebugger::StartDebugging()
{
	if(_dwPidToAttach != 0 && !_NormalDebugging)
	{
		CleanWorkSpace();
		_isDebugging = true;
		_beginthreadex(NULL,NULL,clsDebugger::DebuggingEntry,this,NULL,NULL);
	}	
	else
	{
		if(_sTarget.length() <= 0 || _isDebugging || !IsValidFile())
			return false;

		CleanWorkSpace();
		_isDebugging = true;
		_beginthreadex(NULL,NULL,clsDebugger::DebuggingEntry,this,NULL,NULL);
	}
	return true;
}

unsigned _stdcall clsDebugger::DebuggingEntry(LPVOID pThis)
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

	if(CreateProcess(_sTarget.c_str(),NULL,NULL,NULL,false,dwCreationFlag,NULL,NULL,&_si,&_pi))
	{
		DebuggingLoop();
	}
}

void clsDebugger::DebuggingLoop()
{
	DEBUG_EVENT debug_event = {0};
	bool bContinueDebugging = true;
	DWORD dwContinueStatus = DBG_CONTINUE;

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
				PBProcInfo(debug_event.dwProcessId,tcDllFilepath,(DWORD)debug_event.u.CreateProcessInfo.lpStartAddress,-1,hProc);

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
					SymLoadModuleExW(hProc,NULL,tcDllFilepath,0,(DWORD)debug_event.u.CreateProcessInfo.lpBaseOfImage,0,0,0);

				BPStruct newBP;
				newBP.dwHandle = 0x1;
				newBP.dwSize = 0x1;
				newBP.dwPID = (DWORD)debug_event.dwProcessId;
				newBP.dwOffset = (DWORD)debug_event.u.CreateProcessInfo.lpStartAddress;

				wSoftwareBP(newBP.dwPID,newBP.dwOffset,newBP.dwHandle,newBP.dwSize,newBP.bOrgByte);
				SoftwareBPs.push_back(newBP);

				break;
			}
		case CREATE_THREAD_DEBUG_EVENT:
			PBThreadInfo(debug_event.dwProcessId,debug_event.dwThreadId,(DWORD)debug_event.u.CreateThread.lpStartAddress,false,-1,true);
			// Init HW BPs in new Threads
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			PBThreadInfo(debug_event.dwProcessId,debug_event.dwThreadId,(DWORD)debug_event.u.CreateThread.lpStartAddress,false,debug_event.u.ExitThread.dwExitCode,false);
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			{
				PBProcInfo(debug_event.dwProcessId,L"",(DWORD)debug_event.u.CreateProcessInfo.lpStartAddress,debug_event.u.ExitProcess.dwExitCode,NULL);
				SymCleanup(debug_event.u.CreateProcessInfo.hProcess);

				BOOL bStillOneRunning = false;

				for(size_t i = 0;i < PIDs.size();i++)
				{
					if(PIDs[i].bRunning && debug_event.dwProcessId != PIDs[i].dwPID)
					{
						bStillOneRunning = true;
						break;
					}
				}

				if(!bStillOneRunning)
					bContinueDebugging = false;
			}
			break;

		case LOAD_DLL_DEBUG_EVENT:
			{
				PTCHAR sDLLFileName = GetFileNameFromHandle(debug_event.u.LoadDll.hFile); 
				PBDLLInfo(sDLLFileName,debug_event.dwProcessId,(DWORD)debug_event.u.LoadDll.lpBaseOfDll,true);

				HANDLE hProc = 0;
				int iPid = 0;
				
				for(size_t i = 0;i < PIDs.size();i++)
				{
					if(PIDs[i].dwPID == debug_event.dwProcessId)
						hProc = PIDs[i].hSymInfo;iPid = i;
				}

				if(PIDs[iPid].bSymLoad && dbgSettings.bAutoLoadSymbols == true)
					SymLoadModuleExW(hProc,NULL,sDLLFileName,0,(DWORD)debug_event.u.LoadDll.lpBaseOfDll,0,0,0);
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
				if(DLLs[i].dwBaseAdr == (DWORD)debug_event.u.UnloadDll.lpBaseOfDll && DLLs[i].dwPID == debug_event.dwProcessId)
				{
					PBDLLInfo(DLLs[i].sPath,DLLs[i].dwPID,DLLs[i].dwBaseAdr,false);
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
						hProcess = PIDs[i].hSymInfo;
				}
				if(debug_event.u.DebugString.fUnicode)
					ReadProcessMemory(hProcess,debug_event.u.DebugString.lpDebugStringData,wMsg,debug_event.u.DebugString.nDebugStringLength,NULL);
				else
				{
					PCHAR Msg = (PCHAR)malloc(debug_event.u.DebugString.nDebugStringLength  * sizeof(CHAR));
					ReadProcessMemory(hProcess,debug_event.u.DebugString.lpDebugStringData,Msg,debug_event.u.DebugString.nDebugStringLength,NULL);	
					mbstowcs(wMsg,Msg,debug_event.u.DebugString.nDebugStringLength);
					free(Msg);
				}
				PBDbgString(wMsg,debug_event.dwProcessId);
				//free(wMsg);
				break;
			}
		case EXCEPTION_DEBUG_EVENT:
			EXCEPTION_DEBUG_INFO exInfo = debug_event.u.Exception;

			if(!CheckIfExceptionIsBP((DWORD)exInfo.ExceptionRecord.ExceptionAddress,debug_event.dwProcessId,false))
				PBExceptionInfo((DWORD)exInfo.ExceptionRecord.ExceptionAddress,exInfo.ExceptionRecord.ExceptionCode,debug_event.dwProcessId,debug_event.dwThreadId);

			switch (exInfo.ExceptionRecord.ExceptionCode)
			{
			case EXCEPTION_BREAKPOINT:
				{
					BOOL bIsEP = false,bIsBP = false;
					int iPid = 0;
					for(size_t i = 0;i < PIDs.size(); i++)
					{
						if(PIDs[i].dwPID == debug_event.dwProcessId)
							iPid = i;
					}

					if(PIDs[iPid].bKernelBP == false && dbgSettings.dwBreakOnEPMode == 1)
						dwContinueStatus = CallBreakDebugger(&debug_event,0);
					else if(PIDs[iPid].bKernelBP)
					{
						if((DWORD)exInfo.ExceptionRecord.ExceptionAddress == PIDs[iPid].dwEP)
						{
							bIsEP = true;
							InitBP();
						}
						
						bIsBP = CheckIfExceptionIsBP((DWORD)exInfo.ExceptionRecord.ExceptionAddress,debug_event.dwProcessId,true);
						if(bIsBP)
						{
							HANDLE hProc = PIDs[iPid].hSymInfo;
							HANDLE hThread = OpenThread(THREAD_GETSET_CONTEXT,false,debug_event.dwThreadId);

							for(size_t i = 0;i < SoftwareBPs.size(); i++)
							{
								if((DWORD)exInfo.ExceptionRecord.ExceptionAddress == SoftwareBPs[i].dwOffset && 
									(SoftwareBPs[i].dwPID == debug_event.dwProcessId || SoftwareBPs[i].dwPID == -1))
								{
									WriteProcessMemory(hProc,(LPVOID)SoftwareBPs[i].dwOffset,(LPVOID)&SoftwareBPs[i].bOrgByte,SoftwareBPs[i].dwSize,NULL);
									FlushInstructionCache(hProc,(LPVOID)SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwSize);
									SoftwareBPs[i].bRestoreBP = true;
								}
							}

							CONTEXT cTT;
							cTT.ContextFlags = CONTEXT_ALL;
							GetThreadContext(hThread,&cTT);
							cTT.Eip--;
							cTT.EFlags |= 0x100;
							PIDs[iPid].bTrapFlag = true;
							PIDs[iPid].dwBPRestoreFlag = 0x2;
							SetThreadContext(hThread,&cTT);

							if(bIsEP && dbgSettings.dwBreakOnEPMode == 3)
								dwContinueStatus = CallBreakDebugger(&debug_event,2);
							else
							{
								memset(tcLogString,0x00,LOGBUFFER);
								if(bIsEP)
									swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on EP at 0x%08X",(DWORD)exInfo.ExceptionRecord.ExceptionAddress);
								else
									swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on Software BP at 0x%08X",(DWORD)exInfo.ExceptionRecord.ExceptionAddress);
								PBLogInfo();
								dwContinueStatus = CallBreakDebugger(&debug_event,0);
							}
						}
						else
							dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_BREAKPOINT);
					}
					PIDs[iPid].bKernelBP = true;
					break;
				}
			case EXCEPTION_ACCESS_VIOLATION:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_ACCESS_VIOLATION);
				break;
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_ARRAY_BOUNDS_EXCEEDED);
				break;
			case EXCEPTION_DATATYPE_MISALIGNMENT:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_DATATYPE_MISALIGNMENT);
				break;
			case EXCEPTION_FLT_DENORMAL_OPERAND:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_FLT_DENORMAL_OPERAND);
				break;
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_FLT_DIVIDE_BY_ZERO);
				break;
			case EXCEPTION_FLT_INEXACT_RESULT:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_FLT_INEXACT_RESULT);
				break;
			case EXCEPTION_FLT_INVALID_OPERATION:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_FLT_INVALID_OPERATION);
				break;
			case EXCEPTION_FLT_OVERFLOW:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_FLT_OVERFLOW);
				break;
			case EXCEPTION_FLT_STACK_CHECK:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_FLT_STACK_CHECK);
				break;
			case EXCEPTION_FLT_UNDERFLOW:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_FLT_UNDERFLOW);
				break;
			case EXCEPTION_ILLEGAL_INSTRUCTION:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_ILLEGAL_INSTRUCTION);
				break;
			case EXCEPTION_IN_PAGE_ERROR:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_IN_PAGE_ERROR);
				break;
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_INT_DIVIDE_BY_ZERO);
				break;
			case EXCEPTION_INT_OVERFLOW:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_INT_OVERFLOW);
				break;
			case EXCEPTION_INVALID_DISPOSITION:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_INVALID_DISPOSITION);
				break;
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_NONCONTINUABLE_EXCEPTION);
				break;
			case EXCEPTION_PRIV_INSTRUCTION:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_PRIV_INSTRUCTION);
				break;
			case EXCEPTION_STACK_OVERFLOW:
				dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_STACK_OVERFLOW);
				break;
			case EXCEPTION_SINGLE_STEP:
				{
					bool bIsBP = CheckIfExceptionIsBP((DWORD)exInfo.ExceptionRecord.ExceptionAddress,debug_event.dwProcessId,true);
					int iPID = NULL;
					for(size_t i = 0;i < PIDs.size();i++)
						if(PIDs[i].dwPID == debug_event.dwProcessId)
							iPID = i;

					if(bIsBP)
					{
						if(PIDs[iPID].dwBPRestoreFlag == 0x2) // Restore SoftwareBP
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
									swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Restored BP at 0x%08X",SoftwareBPs[i].dwOffset);
									PBLogInfo();
								}
							}
							PIDs[iPID].dwBPRestoreFlag = NULL;
							dwContinueStatus = DBG_CONTINUE;
						}
						else if(PIDs[iPID].dwBPRestoreFlag == 0x4) // Restore MemBP
						{
							for(size_t i = 0;i < MemoryBPs.size(); i++)
							{
								if(MemoryBPs[i].bRestoreBP &&
									MemoryBPs[i].dwHandle == 0x1 &&
									(MemoryBPs[i].dwPID == debug_event.dwProcessId || MemoryBPs[i].dwPID == -1))
									wMemoryBP(MemoryBPs[i].dwPID,MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].dwHandle);
								MemoryBPs[i].bRestoreBP = false;
							}
							PIDs[iPID].dwBPRestoreFlag = NULL;
							dwContinueStatus = DBG_CONTINUE;
						}
						else if(PIDs[iPID].dwBPRestoreFlag == 0x8) // Restore HwBp
						{
							for(size_t i = 0;i < HardwareBPs.size();i++)
							{
								if(HardwareBPs[i].bRestoreBP &&
									HardwareBPs[i].dwHandle == 0x1 &&
									(HardwareBPs[i].dwPID == debug_event.dwProcessId || HardwareBPs[i].dwPID == -1))
								{
									wHardwareBP(HardwareBPs[i].dwPID,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSize,HardwareBPs[i].dwSlot);
									HardwareBPs[i].bRestoreBP = false;
								}
							}
							PIDs[iPID].bTrapFlag = false;
							PIDs[iPID].dwBPRestoreFlag = NULL;
							dwContinueStatus = DBG_CONTINUE;
						}
						else if(PIDs[iPID].dwBPRestoreFlag == NULL) // First time hit HwBP
						{
							for(size_t i = 0;i < HardwareBPs.size(); i++)
							{
								if(HardwareBPs[i].dwOffset == (DWORD)debug_event.u.Exception.ExceptionRecord.ExceptionAddress &&
									(HardwareBPs[i].dwPID == debug_event.dwProcessId || HardwareBPs[i].dwPID == -1) &&
									dHardwareBP(debug_event.dwProcessId,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSlot))
								{
									HANDLE hThread = OpenThread(THREAD_GETSET_CONTEXT,false,debug_event.dwThreadId);
									
									memset(tcLogString,0x00,LOGBUFFER);
									swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on Hardware BP at 0x%08X",HardwareBPs[i].dwOffset);
									PBLogInfo();

									dwContinueStatus = CallBreakDebugger(&debug_event,0);
									HardwareBPs[i].bRestoreBP = true;
									PIDs[iPID].dwBPRestoreFlag = 0x8;
									PIDs[iPID].bTrapFlag = true;
									
									CONTEXT cTT;cTT.ContextFlags = CONTEXT_ALL;
									GetThreadContext(hThread,&cTT);
									cTT.EFlags |= 0x100;
									SetThreadContext(hThread,&cTT);
								}
							}
						}
						else
							dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
					}
					else
						dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_SINGLE_STEP);
					break;
				}
			case EXCEPTION_GUARD_PAGE:
				{
					bool bIsBP = CheckIfExceptionIsBP((DWORD)exInfo.ExceptionRecord.ExceptionAddress,debug_event.dwProcessId,true);
					if(bIsBP)
					{
						int iPID = NULL;
						for(size_t i = 0;i < PIDs.size();i++)
							if(PIDs[i].dwPID == debug_event.dwProcessId)
								iPID = i;

						HANDLE hThread = OpenThread(THREAD_GETSET_CONTEXT,false,debug_event.dwThreadId);
						CONTEXT cTT;cTT.ContextFlags = CONTEXT_ALL;
						GetThreadContext(hThread,&cTT);
						cTT.EFlags |= 0x100;
						SetThreadContext(hThread,&cTT);
						PIDs[iPID].dwBPRestoreFlag = 0x4;
						PIDs[iPID].bTrapFlag = true;

						for(size_t i = 0;i < MemoryBPs.size();i++)
							if(MemoryBPs[i].dwOffset == (DWORD)exInfo.ExceptionRecord.ExceptionAddress)
								MemoryBPs[i].bRestoreBP = true;

						dwContinueStatus = CallBreakDebugger(&debug_event,0);
					}
					else
						dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwEXCEPTION_GUARD_PAGE);
					break;
				}
			default:
				{
					bool bCustomHandler = false;
					for (int i = 0; i < exCustom.size();i++)
					{
						if(debug_event.u.Exception.ExceptionRecord.ExceptionCode == exCustom[i].dwExceptionType)
						{
							dwContinueStatus = CallBreakDebugger(&debug_event,exCustom[i].dwAction);
							bCustomHandler = true;
						}
					}
					if(!bCustomHandler)
						dwContinueStatus = CallBreakDebugger(&debug_event,1); // NOT_HANDLED

					break;
				}
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
	PulseEvent(hDebuggingHandle);
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

			ProcessContext.ContextFlags = CONTEXT_ALL;
			GetThreadContext(hThread,&ProcessContext);

			PulseEvent(hDebuggingHandle);
			WaitForSingleObject(_hDbgEvent,INFINITE);
			SetThreadContext(hThread,&ProcessContext);
			_dwCurPID = 0;_dwCurTID = 0;
			return DBG_EXCEPTION_HANDLED;
		}
	case 1:
		return DBG_EXCEPTION_NOT_HANDLED;
	case 2:
		return DBG_CONTINUE;
	case 3:
		ProcessContext.ContextFlags = CONTEXT_ALL;
		GetThreadContext(OpenThread(THREAD_GET_CONTEXT,false,debug_event->dwThreadId),&ProcessContext);
		ProcessContext.Eip = GetReturnAdressFromStackFrame(ProcessContext.Ebp + 0x04,debug_event);
		SetThreadContext(OpenThread(THREAD_SET_CONTEXT,false,debug_event->dwThreadId),&ProcessContext);
		return DBG_EXCEPTION_HANDLED;
	default:
		return DBG_EXCEPTION_NOT_HANDLED;
	}
}

bool clsDebugger::wSoftwareBP(DWORD dwPID,DWORD dwOffset,DWORD dwKeep,DWORD dwSize,BYTE &bOrgByte)
{
	if(dwOffset == 0)
		return false;

	DWORD dwBytesWritten,dwBytesRead;
	BYTE bNew = 0xCC,bOld;
	
	HANDLE hPID = _pi.hProcess;

	if(dwPID != -1)
	{
		for(size_t i = 0;i < PIDs.size();i++)
		{
			if(dwPID == PIDs[i].dwPID)
				hPID = PIDs[i].hSymInfo;
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

bool clsDebugger::wMemoryBP(DWORD dwPID,DWORD dwOffset,DWORD dwSize,DWORD dwKeep)
{
	MEMORY_BASIC_INFORMATION MBI;
	SYSTEM_INFO sysInfo;
	DWORD dwOldProtection;

	HANDLE hPID = _pi.hProcess;
	
	for(size_t i = 0;i < PIDs.size(); i++)
		if(PIDs[i].dwPID == dwPID)
			hPID = PIDs[i].hSymInfo;

	GetSystemInfo(&sysInfo);
	VirtualQueryEx(hPID,(LPVOID)dwOffset,&MBI,sizeof(MBI));

	if(!VirtualProtectEx(hPID,(LPVOID)dwOffset,dwSize,MBI.Protect | PAGE_GUARD,&dwOldProtection))
		return false;
	return true;
}

bool clsDebugger::wHardwareBP(DWORD dwPID,DWORD dwOffset,DWORD dwSize,DWORD dwSlot)
{
	int iBP = NULL;
	DWORD dwThreadCounter = 0;
	THREADENTRY32 threadEntry32;
	threadEntry32.dwSize = sizeof(THREADENTRY32);
	CONTEXT cTT;
	cTT.ContextFlags = CONTEXT_ALL;
	HANDLE hThread = INVALID_HANDLE_VALUE;

	if(dwPID == -1)
		dwPID = _pi.dwProcessId;

	if(!(dwSize == 1 || dwSize == 2 || dwSize == 4))
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
			cTT.Dr7 |= DR_EXECUTE << ((HardwareBPs[iBP].dwSlot * 4) + 16);
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

bool clsDebugger::StepOver(DWORD dwNewOffset)
{
	for (vector<BPStruct>::iterator it = SoftwareBPs.begin(); it!= SoftwareBPs.end(); it++) {
		if(it->dwHandle == 0x2)
		{
			SoftwareBPs.erase(it);
			it = SoftwareBPs.begin();
		}
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
	}
	return true;
}

bool clsDebugger::StepIn()
{
	ProcessContext.EFlags |= 0x100;
	return PulseEvent(_hDbgEvent);
}

bool clsDebugger::CheckProcessState(DWORD dwPID)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 procEntry32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

	if(hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return false;
	}

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
	if(dwOnCallStack == NULL)
		return false;

	if(_dwCurTID == 0 || _dwCurPID == 0)
		return false; //SuspendDebugging();

	HANDLE hProc,hThread = OpenThread(THREAD_GETSET_CONTEXT,false,_dwCurTID);
	PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)malloc(sizeof(SYMBOL_INFOW) + MAX_SYM_NAME);
	DWORD64 dwDisplacement;
	IMAGEHLP_MODULEW64 imgMod = {0};

	int iPid = 0;
	for(size_t i = 0;i < PIDs.size(); i++)
	{
		if(PIDs[i].dwPID == _dwCurPID)
			hProc = PIDs[i].hSymInfo;iPid = i;
	}

	if(!PIDs[iPid].bSymLoad)
		PIDs[iPid].bSymLoad = SymInitialize(hProc,NULL,false);

	//SymRefreshModuleList(hProc);

	STACKFRAME64 stackFr = {0};

	CONTEXT context;
	context.ContextFlags = CONTEXT_ALL;
	GetThreadContext(hThread, &context);

	stackFr.AddrPC.Offset = context.Eip;
	stackFr.AddrPC.Mode = AddrModeFlat;
	stackFr.AddrFrame.Offset = context.Ebp;
	stackFr.AddrFrame.Mode = AddrModeFlat;
	stackFr.AddrStack.Offset = context.Esp;
	stackFr.AddrStack.Mode = AddrModeFlat;

	BOOL bSuccess;
	do
	{
		bSuccess = StackWalk64(IMAGE_FILE_MACHINE_I386,hProc,hThread,&stackFr,&context,NULL,SymFunctionTableAccess64,SymGetModuleBase64,0);

		if(!bSuccess)        
			break;

		memset(&imgMod,0x00,sizeof(imgMod));
		imgMod.SizeOfStruct = sizeof(imgMod);
		bSuccess = SymGetModuleInfoW64(hProc,(DWORD)stackFr.AddrPC.Offset, &imgMod);

		memset(pSymbol,0,sizeof(SYMBOL_INFOW) + MAX_SYM_NAME);
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
		pSymbol->MaxNameLen = MAX_SYM_NAME;

		DWORD dwStackAddr = stackFr.AddrStack.Offset;

		DWORD dwEIP = stackFr.AddrPC.Offset;
		bSuccess = SymFromAddrW(hProc,dwEIP,&dwDisplacement,pSymbol);
		wstring sFuncName = pSymbol->Name;
		bSuccess = SymGetModuleInfoW64(hProc,(DWORD)dwEIP, &imgMod);
		wstring sFuncMod = imgMod.ModuleName;

		DWORD dwReturnTo = stackFr.AddrReturn.Offset;
		bSuccess = SymFromAddrW(hProc,dwReturnTo,&dwDisplacement,pSymbol);
		wstring sReturnToFunc = pSymbol->Name;
		bSuccess = SymGetModuleInfoW64(hProc,(DWORD)dwReturnTo,&imgMod);
		wstring sReturnToMod = imgMod.ModuleName;

		IMAGEHLP_LINEW64 imgSource;
		imgSource.SizeOfStruct = sizeof(imgSource);
		bSuccess = SymGetLineFromAddrW64(hProc,(DWORD)stackFr.AddrPC.Offset,(DWORD*)&dwDisplacement,&imgSource);

		if(bSuccess)
			dwOnCallStack(dwStackAddr,
			dwReturnTo,sReturnToFunc,sReturnToMod,
			dwEIP,sFuncName,sFuncMod,
			imgSource.FileName,imgSource.LineNumber);
		else
			dwOnCallStack(dwStackAddr,
			dwReturnTo,sReturnToFunc,sReturnToMod,
			dwEIP,sFuncName,sFuncMod,
			L"",0);
		
	}while(stackFr.AddrReturn.Offset != 0);

	free(pSymbol);
	return false;
}

void clsDebugger::AddBreakpointToList(DWORD dwBPType,DWORD dwPID,DWORD dwOffset,DWORD dwSlot,DWORD dwKeep)
{
	bool bExists = false;

	for(size_t i = 0;i < SoftwareBPs.size();i++)
	{
		if(SoftwareBPs[i].dwOffset == dwOffset && SoftwareBPs[i].dwPID == dwPID)
		{
			bExists = true;
			break;
		}
	}
	for(size_t i = 0;i < MemoryBPs.size();i++)
	{
		if(MemoryBPs[i].dwOffset == dwOffset && MemoryBPs[i].dwPID == dwPID)
		{
			bExists = true;
			break;
		}
	}
	for(size_t i = 0;i < HardwareBPs.size();i++)
	{
		if(HardwareBPs[i].dwOffset == dwOffset && HardwareBPs[i].dwPID == dwPID)
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
				SoftwareBPs.push_back(newBP);

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
				MemoryBPs.push_back(newBP);

				break;
			}
		case 2:
			{
				//HardwareBP
				if(HardwareBPs.size() == 4)
					return;

				BPStruct newBP;
				newBP.dwOffset = dwOffset;
				newBP.dwHandle = dwKeep;
				newBP.dwSize = 1;
				newBP.dwPID = dwPID;
				newBP.bOrgByte = NULL;
				newBP.bRestoreBP = false;

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
				break;
			}
		}
	}
}

bool clsDebugger::InitBP()
{
	for(size_t i = 0;i < SoftwareBPs.size(); i++)
		wSoftwareBP(SoftwareBPs[i].dwPID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwHandle,SoftwareBPs[i].dwSize,SoftwareBPs[i].bOrgByte);

	for(size_t i = 0;i < MemoryBPs.size(); i++)
		wMemoryBP(MemoryBPs[i].dwPID,MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].dwHandle);

	for(size_t i = 0;i < HardwareBPs.size(); i++)
		wHardwareBP(HardwareBPs[i].dwPID,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSize,HardwareBPs[i].dwSlot);
	return true;
}

bool clsDebugger::DetachFromProcess()
{
	_NormalDebugging = true;
	_isDebugging = false;
	_bStopDebugging = true;
	DebugBreakProcess(_pi.hProcess);
	return true;
}

bool clsDebugger::AttachToProcess(DWORD dwPID)
{
	_NormalDebugging = false;_dwPidToAttach = dwPID;
	return true;//StartDebugging();
}

bool clsDebugger::LoadSymbolForAddr(wstring& sFuncName,wstring& sModName,DWORD dwOffset)
{
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	bool bTest = false;

	if(_dwCurPID == 0)
		_dwCurPID = _pi.dwProcessId;

	int iPid = 0;
	for(size_t i = 0;i < PIDs.size(); i++)
	{
		if(PIDs[i].dwPID == _dwCurPID)
			hProcess = PIDs[i].hSymInfo;iPid = i;
	}

	if(!PIDs[iPid].bSymLoad)
		PIDs[iPid].bSymLoad = SymInitialize(hProcess,NULL,false);

	//bTest = SymRefreshModuleList(hProcess);

	IMAGEHLP_MODULEW imgMod = {0};
	imgMod.SizeOfStruct = sizeof(imgMod);
	PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)malloc(sizeof(SYMBOL_INFOW) + MAX_SYM_NAME);
	memset(pSymbol, 0, sizeof(SYMBOL_INFOW) + MAX_SYM_NAME);
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
	pSymbol->MaxNameLen = MAX_SYM_NAME;
	DWORD64 dwDisplacement;

	bTest = SymGetModuleInfoW(hProcess,dwOffset,&imgMod);
	bTest = SymFromAddrW(hProcess,dwOffset,&dwDisplacement,pSymbol);

	sFuncName = pSymbol->Name;
	sModName = imgMod.ModuleName;

	free(pSymbol);
	return bTest;
}

bool clsDebugger::RemoveBPFromList(DWORD dwOffset,DWORD dwType,DWORD dwPID)
{ 
	switch(dwType)
	{
	case 0:
		for (vector<BPStruct>::iterator it = SoftwareBPs.begin();it != SoftwareBPs.end();++it)
		{
			if(it->dwOffset == dwOffset && it->dwPID == dwPID)
			{
				SoftwareBPs.erase(it);
				it = SoftwareBPs.begin();
			}
		}
		break;

	case 1:
		for (vector<BPStruct>::iterator it = MemoryBPs.begin();it != MemoryBPs.end(); ++it)
		{
			if(it->dwOffset == dwOffset && it->dwPID == dwPID)
			{
				MemoryBPs.erase(it);
				it = MemoryBPs.begin();
			}
		}
		break;

	case 2:
		for (vector<BPStruct>::iterator it = HardwareBPs.begin();it != HardwareBPs.end(); ++it)
		{
			if(it->dwOffset == dwOffset && it->dwPID == dwPID)
			{
				HardwareBPs.erase(it);
				it = HardwareBPs.begin();
			}
		}
		break;
	}
	return true;
}

bool clsDebugger::dHardwareBP(DWORD dwPID,DWORD dwOffset,DWORD dwSlot)
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

DWORD clsDebugger::GetReturnAdressFromStackFrame(DWORD dwEbp,DEBUG_EVENT *debug_event)
{
	DWORD dwRetAdr = NULL,
		dwBytesRead;
	HANDLE hProcess;

	int iPid = 0;
	for(size_t i = 0;i < PIDs.size(); i++)
	{
		if(PIDs[i].dwPID == debug_event->dwProcessId)
			hProcess = PIDs[i].hSymInfo;iPid = i;
	}

	BYTE* bBuffer[8];

	if(ReadProcessMemory(hProcess,(LPVOID)dwEbp,(LPVOID)bBuffer,8,&dwBytesRead))
		dwRetAdr = (DWORD)*bBuffer;

	return dwRetAdr;
}

bool clsDebugger::IsTargetSet()
{
	if(_sTarget.length() > 0)
		return true;
	return false;
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
	SoftwareBPs.clear();
	HardwareBPs.clear();
	MemoryBPs.clear();
	return true;
}

bool clsDebugger::CheckIfExceptionIsBP(DWORD dwExceptionOffset,DWORD dwPID,bool bClearTrapFlag)
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

void clsDebugger::CustomExceptionAdd(DWORD dwExceptionType,DWORD dwAction)
{
	customException custEx;
	custEx.dwAction = dwAction;
	custEx.dwExceptionType = dwExceptionType;
	exCustom.push_back(custEx);
}

void clsDebugger::CustomExceptionRemove(DWORD dwExceptionType)
{
	for (vector<customException>::iterator it = exCustom.begin(); it != exCustom.end(); ++it) {
		if(it->dwExceptionType == dwExceptionType)
		{
			exCustom.erase(it);
			it = exCustom.begin();
		}
	}
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