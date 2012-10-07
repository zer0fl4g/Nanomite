#include "clsDebugger.h"

clsDebugger::clsDebugger()
{
	_NormalDebugging = true;
	_isDebugging = false;
	dwOnThread = NULL;dwOnPID = NULL;dwOnDll = NULL;dwOnException = NULL;dwOnLog = NULL;dwOnDbgString = NULL;
	hDebuggingHandle = CreateEvent(NULL,false,false,L"clsDebugger");
	clsDebuggerSettings tempSet = {false,false,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	dbgSettings = tempSet;
}

clsDebugger::clsDebugger(wstring sTarget)
{
	_sTarget = sTarget;
	_NormalDebugging = true;
	_isDebugging = false;
	dwOnThread = NULL;dwOnPID = NULL;dwOnDll = NULL;dwOnException = NULL;dwOnLog = NULL;dwOnDbgString = NULL;
	hDebuggingHandle = CreateEvent(NULL,false,false,L"clsDebugger");
	clsDebuggerSettings tempSet = {false,false,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	dbgSettings = tempSet;
}

clsDebugger::~clsDebugger(void)
{
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
	DLLs.clear();
	Exceptions.clear();
	Threads.clear();
	PIDs.clear();
	Log.clear();
	DbgStrings.clear();
}

bool clsDebugger::SuspendDebuggingAll()
{
	for(size_t i = 0;i < PIDs.size();i++)
	{
		SuspendDebugging(PIDs[i].dwPID);
	}
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
				wstringstream ss; ss << L"[!] 0x" << hex << dwPID << L" Debugging suspended!";
				PBLogInfo(ss.str());
				return true;
			}
		}
		else// if(dbgSettings.dwSuspendType == 0x1)
		{
			if(SuspendProcess(dwPID,true))
			{
				wstringstream ss; ss << L"[!] 0x" << hex << dwPID << L" Debugging suspended!";
				PBLogInfo(ss.str());
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
	{
		if(CheckProcessState(PIDs[i].dwPID))
		{
			StopDebugging(PIDs[i].dwPID);
		}
	}	
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
	//PulseEvent(_hDbgEvent);
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
	{
		dwOnThread(dwPID,dwTID,dwEP,bSuspended,dwExitCode,bFound);
	}

	wstringstream sTempLog;

	if(bFound)
	{
		sTempLog << "[-] Exit Thread(" << hex << dwTID << ") in Process(" << hex << dwPID << ") with Exitcode: " << dwExitCode;
	}
	else
	{
		sTempLog << "[+] New Thread(" << hex << dwTID << ") in Process(" << hex << dwPID << ") with Entrypoint: " << dwEP;
	}
	PBLogInfo(sTempLog.str());

	return true;
}

bool clsDebugger::PBProcInfo(DWORD dwPID,wstring sFileName,DWORD dwEP,DWORD dwExitCode,HANDLE hSymInfo)
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

		PIDs.push_back(newPID);
	}

	if(dwOnPID != NULL)
	{
		dwOnPID(dwPID,sFileName,dwExitCode,dwEP,bFound);
	}

	wstringstream sTempLog;
	if(bFound)
	{
		sTempLog << "[-] Exit Process(" << hex << dwPID << ") with Exitcode: " << dwExitCode;
	}
	else
	{
		sTempLog << "[+] New Process(" << hex << dwPID << "): " << sFileName << " with Entrypoint: " << dwEP;
	}

	PBLogInfo(sTempLog.str());

	return true;
}

bool clsDebugger::PBExceptionInfo(DWORD dwExceptionOffset,DWORD dwExceptionCode,DWORD dwPID,DWORD dwTID)
{
	ExceptionStruct newExcept;
	newExcept.dwExceptionCode = dwExceptionCode;
	newExcept.dwExceptionOffset = dwExceptionOffset;
	newExcept.dwPID = dwPID;
	newExcept.dwTID = dwTID;

	Exceptions.push_back(newExcept);

	wstring sModName,sFuncName;

	_dwCurPID = dwPID;
	LoadSymbolForAddr(sFuncName,sModName,dwExceptionOffset);
	_dwCurPID = 0;

	if(dwOnException != NULL)
	{
		dwOnException(sFuncName,sModName,dwExceptionOffset,dwExceptionCode,dwPID,dwTID);
	}

	wstringstream sTempLog;
	sTempLog << "[!] " << sFuncName << "@" << sModName << " ExceptionCode: " << hex << dwExceptionCode << " ExceptionOffset: " << hex << dwExceptionOffset << " PID: " << hex << dwPID << " TID: " << hex << dwTID;
	PBLogInfo(sTempLog.str());

	return true;
}

bool clsDebugger::PBDLLInfo(wstring sDLLPath,DWORD dwPID,DWORD dwEP,bool bLoaded)
{
	bool bFound = false;
	for(size_t i = 0;i < DLLs.size(); i++)
	{
		if(DLLs[i].sPath == sDLLPath && DLLs[i].dwPID)
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
	{
		dwOnDll(sDLLPath,dwPID,dwEP,bLoaded);
	}

	wstringstream sTempLog;
	if(bLoaded)
	{
		sTempLog << "[+] PID(" << hex << dwPID << ") - Loaded DLL: " << sDLLPath << " EntryPoint: " << hex << dwEP;
	}
	else
	{
		sTempLog << "[+] PID(" << hex << dwPID << ") - Unloaded DLL: " << sDLLPath;
	}

	PBLogInfo(sTempLog.str());

	return true;
}

bool clsDebugger::PBDbgString(wstring sMessage,DWORD dwPID)
{
	DbgStrings.push_back(sMessage);

	if(dwOnDbgString != NULL)
	{
		dwOnDbgString(sMessage,dwPID);
	}
	wstringstream sTempLog;
	sTempLog << "[+] PID(" << hex << dwPID << ") DbgString: " << sMessage;

	PBLogInfo(sTempLog.str());
	return true;
}

bool clsDebugger::PBLogInfo(wstring sLog)
{
	time_t tTime;
	tm* timeInfo;
	time(&tTime);
	timeInfo = localtime(&tTime);
	wstringstream ss; ss << "[" << timeInfo->tm_hour << ":" << timeInfo->tm_min << ":" << timeInfo->tm_sec << "] " << sLog;

	if(dwOnLog != NULL)
	{
		dwOnLog(ss.str());
	}
	Log.push_back(ss.str());

	return true;
}

wstring clsDebugger::GetFileNameFromHandle(HANDLE hFile) 
{
	BOOL bSuccess = false;
	TCHAR pszFilename[MAX_PATH+1];
	HANDLE hFileMap;

	wstringstream strFilename;

	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 

	if( dwFileSizeLo == 0 && dwFileSizeHi == 0 )
	{     
		return false;
	}

	hFileMap = CreateFileMapping(hFile,NULL,PAGE_READONLY,0,1,NULL);

	if (hFileMap) 
	{
		LPVOID pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem) 
		{
			if (GetMappedFileName(GetCurrentProcess(),pMem,pszFilename,MAX_PATH)) 
			{
				TCHAR szTemp[255];
				szTemp[0] = '\0';

				if (GetLogicalDriveStrings(255-1, szTemp)) 
				{
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = false;
					PTCHAR p = szTemp;

					do 
					{
						*szDrive = *p;

						if (QueryDosDevice(szDrive, szName, MAX_PATH))
						{
							size_t uNameLen = wcslen(szName);

							if (uNameLen < MAX_PATH) 
							{
								bFound = _wcsnicmp(pszFilename, szName,	uNameLen) == 0;

								if (bFound) 
								{
									strFilename << szDrive << (pszFilename+uNameLen);
								}
							}
						}
						while (*p++);
					} while (!bFound && *p);
				}
			}
			bSuccess = true;
			UnmapViewOfFile(pMem);
		} 

		CloseHandle(hFileMap);
	}

	return(strFilename.str());
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

		PBLogInfo(L"[+] Attached to Process");
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

		switch(debug_event.dwDebugEventCode)
		{
		case CREATE_PROCESS_DEBUG_EVENT:
			{
				HANDLE hProc = debug_event.u.CreateProcessInfo.hProcess;
				PBProcInfo(debug_event.dwProcessId,GetFileNameFromHandle(debug_event.u.CreateProcessInfo.hFile),(DWORD)debug_event.u.CreateProcessInfo.lpStartAddress,-1,hProc);

				int iPid = 0;
				for(size_t i = 0;i < PIDs.size();i++)
				{
					if(PIDs[i].dwPID == debug_event.dwProcessId)
						iPid = i;
				}

				PIDs[iPid].bSymLoad = SymInitialize(hProc,NULL,false);

				if(PIDs[iPid].bSymLoad)
				{
					DWORD64 dwBase = SymLoadModuleExW(hProc,debug_event.u.CreateProcessInfo.hFile,GetFileNameFromHandle(debug_event.u.CreateProcessInfo.hFile).c_str(),NULL,(DWORD64)debug_event.u.CreateProcessInfo.lpBaseOfImage,0,0,0);

					IMAGEHLP_MODULE64 module_info;
					module_info.SizeOfStruct = sizeof(module_info);
					BOOL bSuccess = SymGetModuleInfo64(hProc,dwBase, &module_info);
				}
				else
				{
					wstringstream sTempLog;
					sTempLog << "[!] Could not load symbols for Process(" << hex << debug_event.dwProcessId << ")";
					PBLogInfo(sTempLog.str());
				}

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
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			PBThreadInfo(debug_event.dwProcessId,debug_event.dwThreadId,(DWORD)debug_event.u.CreateThread.lpStartAddress,false,debug_event.u.ExitThread.dwExitCode,false);
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			{
				PBProcInfo(debug_event.dwProcessId,L"",(DWORD)debug_event.u.CreateProcessInfo.lpStartAddress,debug_event.u.ExitProcess.dwExitCode,NULL);

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
				wstring sDLLFileName = GetFileNameFromHandle(debug_event.u.LoadDll.hFile); 
				PBDLLInfo(sDLLFileName,debug_event.dwProcessId,(DWORD)debug_event.u.LoadDll.lpBaseOfDll,true);

				HANDLE hProc = 0;
				int iPid = 0;
				
				for(size_t i = 0;i < PIDs.size();i++)
				{
					if(PIDs[i].dwPID == debug_event.dwProcessId)
						hProc = PIDs[i].hSymInfo;iPid = i;
				}

				if(PIDs[iPid].bSymLoad && dbgSettings.bAutoLoadSymbols == true)
				{
					DWORD64 dwBase = SymLoadModuleExW(hProc,NULL,sDLLFileName.c_str(),0,(DWORD)debug_event.u.LoadDll.lpBaseOfDll,0,0,0);

					IMAGEHLP_MODULE64 module_info;
					module_info.SizeOfStruct = sizeof(module_info);
					BOOL bSuccess = SymGetModuleInfo64(hProc,dwBase, &module_info);
				}
				else
				{
					wstringstream sTempLog;
					sTempLog << "[!] Could not load symbols for DLL: " << sDLLFileName << ")";
					PBLogInfo(sTempLog.str());
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
				ZeroMemory(wMsg,debug_event.u.DebugString.nDebugStringLength);

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
					ZeroMemory(Msg,debug_event.u.DebugString.nDebugStringLength);

					ReadProcessMemory(hProcess,debug_event.u.DebugString.lpDebugStringData,Msg,debug_event.u.DebugString.nDebugStringLength,NULL);	

					mbstowcs(wMsg,Msg,debug_event.u.DebugString.nDebugStringLength);
					free(Msg);
				}
				PBDbgString((LPWSTR)wMsg,debug_event.dwProcessId);
				//free(wMsg);
				break;
			}
		case EXCEPTION_DEBUG_EVENT:
			EXCEPTION_DEBUG_INFO& exInfo = debug_event.u.Exception;

			if(!CheckIfExceptionIsBP(exInfo))
				PBExceptionInfo((DWORD)exInfo.ExceptionRecord.ExceptionAddress,exInfo.ExceptionRecord.ExceptionCode,debug_event.dwProcessId,debug_event.dwThreadId);

			switch (exInfo.ExceptionRecord.ExceptionCode)
			{
			case EXCEPTION_ACCESS_VIOLATION:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_ACCESS_VIOLATION);
				break;
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_ARRAY_BOUNDS_EXCEEDED);
				break;
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
					{
						dwContinueStatus = CallBreakDebugger(debug_event,0);
					}

					if(PIDs[iPid].bKernelBP)
					{
						if((DWORD)exInfo.ExceptionRecord.ExceptionAddress == PIDs[iPid].dwEP)
						{
							bIsEP = true;
							InitBP();
						}
						
						for(size_t i = 0;i < SoftwareBPs.size(); i++)
						{
							if((DWORD)exInfo.ExceptionRecord.ExceptionAddress == SoftwareBPs[i].dwOffset)
								bIsBP = true;
						}

						if(bIsBP)
						{
							HANDLE hProc = PIDs[iPid].hSymInfo;
							HANDLE hThread = OpenThread(THREAD_ALL_ACCESS,false,debug_event.dwThreadId);

							for(size_t i = 0;i < SoftwareBPs.size(); i++)
							{
								WriteProcessMemory(hProc,(LPVOID)SoftwareBPs[i].dwOffset,(LPVOID)&SoftwareBPs[i].bOrgByte,SoftwareBPs[i].dwSize,NULL);
								FlushInstructionCache(hProc,(LPVOID)SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwSize);
							}

							CONTEXT cTT;
							cTT.ContextFlags = CONTEXT_ALL;
							GetThreadContext(hThread,&cTT);
							cTT.Eip--;
							cTT.EFlags |= 0x100;
							
							SetThreadContext(hThread,&cTT);

							if(bIsEP && dbgSettings.dwBreakOnEPMode == 3)
								dwContinueStatus = CallBreakDebugger(debug_event,2);
							else
								dwContinueStatus = CallBreakDebugger(debug_event,0);
						}
						else
						{
							dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_BREAKPOINT);
						}
					}

					PIDs[iPid].bKernelBP = true;
					break;
				}
			case EXCEPTION_DATATYPE_MISALIGNMENT:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_DATATYPE_MISALIGNMENT);
				break;
			case EXCEPTION_FLT_DENORMAL_OPERAND:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_FLT_DENORMAL_OPERAND);
				break;
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_FLT_DIVIDE_BY_ZERO);
				break;
			case EXCEPTION_FLT_INEXACT_RESULT:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_FLT_INEXACT_RESULT);
				break;
			case EXCEPTION_FLT_INVALID_OPERATION:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_FLT_INVALID_OPERATION);
				break;
			case EXCEPTION_FLT_OVERFLOW:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_FLT_OVERFLOW);
				break;
			case EXCEPTION_FLT_STACK_CHECK:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_FLT_STACK_CHECK);
				break;
			case EXCEPTION_FLT_UNDERFLOW:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_FLT_UNDERFLOW);
				break;
			case EXCEPTION_ILLEGAL_INSTRUCTION:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_ILLEGAL_INSTRUCTION);
				break;
			case EXCEPTION_IN_PAGE_ERROR:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_IN_PAGE_ERROR);
				break;
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_INT_DIVIDE_BY_ZERO);
				break;
			case EXCEPTION_INT_OVERFLOW:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_INT_OVERFLOW);
				break;
			case EXCEPTION_INVALID_DISPOSITION:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_INVALID_DISPOSITION);
				break;
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_NONCONTINUABLE_EXCEPTION);
				break;
			case EXCEPTION_PRIV_INSTRUCTION:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_PRIV_INSTRUCTION);
				break;
			case EXCEPTION_SINGLE_STEP:
				{
					bool bIsBP = false;

					for(size_t i = 0;i < SoftwareBPs.size(); i++)
					{
						if(SoftwareBPs[i].dwHandle == 0x1)
							wSoftwareBP(SoftwareBPs[i].dwPID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwHandle,SoftwareBPs[i].dwSize,SoftwareBPs[i].bOrgByte);

						if(SoftwareBPs[i].dwOffset <= ((DWORD)debug_event.u.Exception.ExceptionRecord.ExceptionAddress + 0x05) &&
							SoftwareBPs[i].dwOffset >= ((DWORD)debug_event.u.Exception.ExceptionRecord.ExceptionAddress - 0x05))
						{
							bIsBP = true;
						}
					}

					for(size_t i = 0;i < MemoryBPs.size(); i++)
					{
						wMemoryBP(MemoryBPs[i].dwPID,MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].dwHandle);
					}

					bool bT = false;

					for(size_t i = 0;i < HardwareBPs.size(); i++)
					{
						if(HardwareBPs[i].dwOffset == (DWORD)debug_event.u.Exception.ExceptionRecord.ExceptionAddress && RemoveHWBP(debug_event.dwProcessId))
						{
							dwContinueStatus = CallBreakDebugger(debug_event,0);
							bT = true;
						}
					}

					if(!bT)
					{
						RemoveHWBP(debug_event.dwProcessId);
						for(size_t i = 0;i < HardwareBPs.size(); i++)
						{
							if(HardwareBPs[i].dwHandle == 0x1)
								wHardwareBP(HardwareBPs[i].dwPID,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSize,HardwareBPs[i].dwHandle);
						}
						//if(!bIsBP)
						//	dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_SINGLE_STEP);
					}
					else
					{
						CONTEXT cTT;
						cTT.ContextFlags = CONTEXT_ALL;
						GetThreadContext(OpenThread(THREAD_GET_CONTEXT,false,debug_event.dwThreadId),&cTT);
						//cTT.Eip--;
						cTT.EFlags |= 0x100;
						SetThreadContext(OpenThread(THREAD_SET_CONTEXT,false,debug_event.dwThreadId),&cTT);
					}
					break;
				}
			case EXCEPTION_STACK_OVERFLOW:
				dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_STACK_OVERFLOW);
				break;

			case EXCEPTION_GUARD_PAGE:
				{
					DWORD dwHandleIt = 1;

					for(size_t i = 0;i < MemoryBPs.size(); i++)
					{
						if((DWORD)exInfo.ExceptionRecord.ExceptionAddress == MemoryBPs[i].dwOffset && MemoryBPs[i].dwHandle == 0x1)
						{
							dwHandleIt = 0;
							//wMemoryBP(MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].bKeep);
						}
					}

					HANDLE hThread = OpenThread(THREAD_ALL_ACCESS,false,debug_event.dwThreadId);
					CONTEXT cTT;
					cTT.ContextFlags = CONTEXT_ALL;
					GetThreadContext(hThread,&cTT);
					//cTT.Eip--;
					cTT.EFlags |= 0x100;

					SetThreadContext(hThread,&cTT);

					if(dwHandleIt == 0)
						dwContinueStatus = CallBreakDebugger(debug_event,dwHandleIt);
					else
						dwContinueStatus = CallBreakDebugger(debug_event,dbgSettings.dwEXCEPTION_GUARD_PAGE);


					break;
				}
			default:
				{
					//if(debug_event.u.Exception.dwFirstChance == 1)
					//{
					//	dwContinueStatus = CallBreakDebugger(debug_event,0);
					//}

					bool bCustomHandler = false;

					for (int i = 0; i < exCustom.size();i++)
					{
						if(debug_event.u.Exception.ExceptionRecord.ExceptionCode == exCustom[i].dwExceptionType)
						{
							dwContinueStatus = CallBreakDebugger(debug_event,exCustom[i].dwAction);
							bCustomHandler = true;
						}
					}
					if(!bCustomHandler)
						dwContinueStatus = CallBreakDebugger(debug_event,1); // NOT_HANDLED

					break;
				}
			}
			break;
		}
		ContinueDebugEvent(debug_event.dwProcessId,debug_event.dwThreadId,dwContinueStatus);
		dwContinueStatus = DBG_CONTINUE;
	}

	PBLogInfo(L"[-] Debugging finished!");
	_isDebugging = false;
	PulseEvent(hDebuggingHandle);
}

DWORD clsDebugger::CallBreakDebugger(DEBUG_EVENT debug_event,DWORD dwHandle)
{
	switch (dwHandle)
	{
	case 0:
		_dwCurPID = debug_event.dwProcessId;
		_dwCurTID = debug_event.dwThreadId;

		ProcessContext.ContextFlags = CONTEXT_ALL;
		GetThreadContext(OpenThread(THREAD_GET_CONTEXT,false,debug_event.dwThreadId),&ProcessContext);

		PulseEvent(hDebuggingHandle);
		WaitForSingleObject(_hDbgEvent,INFINITE);
		GetThreadContext(OpenThread(THREAD_SET_CONTEXT,false,debug_event.dwThreadId),&ProcessContext);

		_dwCurPID = 0;_dwCurTID = 0;
		return DBG_EXCEPTION_HANDLED;
	case 1:
		return DBG_EXCEPTION_NOT_HANDLED;
	case 2:
		return DBG_CONTINUE;
	case 3:
		ProcessContext.ContextFlags = CONTEXT_ALL;
		GetThreadContext(OpenThread(THREAD_GET_CONTEXT,false,debug_event.dwThreadId),&ProcessContext);
		ProcessContext.Eip = GetReturnAdressFromStackFrame(ProcessContext.Ebp + 0x04,debug_event);
		SetThreadContext(OpenThread(THREAD_SET_CONTEXT,false,debug_event.dwThreadId),&ProcessContext);
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
	DWORD dwOldProtection,dwPageOffset;

	HANDLE hPID;
	hPID = _pi.hProcess;
	
	for(size_t i = 0;i < PIDs.size(); i++)
	{
		if(PIDs[i].dwPID == dwPID)
			hPID = PIDs[i].hSymInfo;
	}

	GetSystemInfo(&sysInfo);
	VirtualQueryEx(hPID,(LPVOID)dwOffset,&MBI,sizeof(MBI));

	if(!VirtualProtectEx(hPID,(LPVOID)dwOffset,dwSize,MBI.Protect | PAGE_GUARD,&dwOldProtection))
		return false;

	//dwPageOffset = (DWORD)MBI.BaseAddress;

	//while(dwPageOffset <= (dwOffset + dwSize))
	//{
	//	if(!VirtualProtectEx(hPID,MBI.BaseAddress,dwSize,MBI.Protect | PAGE_GUARD,&dwOldProtection))
	//		return false;
	//	dwPageOffset += sysInfo.dwPageSize;
	//}
	return true;
}

bool clsDebugger::wHardwareBP(DWORD dwPID,DWORD dwOffset,DWORD dwSize,DWORD dwKeep)
{
	bool bSlot1 = false,bSlot2 = false,bSlot3 = false,bSlot4 = false;

	if(dwPID == -1)
		dwPID = _pi.dwProcessId;

	if(dwSize == 1 || dwSize == 2 || dwSize == 4)
	{
		for(size_t i = 0;i < HardwareBPs.size();i++)
		{
			switch(HardwareBPs[i].dwSlot)
			{
			case 1:
				bSlot1 = true;
				break;
			case 2:
				bSlot2 = true;
				break;
			case 3:
				bSlot3 = true;
				break;
			case 4:
				bSlot4 = true;
				break;
			default:
				HardwareBPs[i].dwSlot = 1;
			}
		}

		DWORD dwSlot;

		if(!bSlot4) dwSlot = 4;
		if(!bSlot3) dwSlot = 3;
		if(!bSlot2) dwSlot = 2;
		if(!bSlot1) dwSlot = 1;

		HANDLE hProcessSnap;
		THREADENTRY32 threadEntry32;

		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,dwPID);

		if(hProcessSnap == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		threadEntry32.dwSize = sizeof(THREADENTRY32);

		if(!Thread32First(hProcessSnap,&threadEntry32))
		{
			CloseHandle(hProcessSnap);
			return false;
		}

		DWORD dwThreadCounter = 0;

		do{
			CONTEXT cTT;
			cTT.ContextFlags = CONTEXT_ALL;
			HANDLE hThread = INVALID_HANDLE_VALUE;

			if(dwPID == threadEntry32.th32OwnerProcessID)
				hThread = OpenThread(THREAD_ALL_ACCESS,false,threadEntry32.th32ThreadID);

			if(hThread != INVALID_HANDLE_VALUE)
			{
				dwThreadCounter++;
				GetThreadContext(hThread,&cTT);

				switch(dwSlot)
				{
				case 1:
					cTT.Dr0 = dwOffset;
					break;
				case 2:
					cTT.Dr1 = dwOffset;
					break;
				case 3:
					cTT.Dr2 = dwOffset;
					break;
				case 4:
					cTT.Dr3 = dwOffset;
					break;
				}

				//cTT.Dr6 = 0;
				//cTT.Dr7 |= 1 << (dwSlot * 2);
				//cTT.Dr7 |= DR_EXECUTE << ((dwSlot * 4) + 16 );
				//cTT.Dr7 |= dwSize << ((dwSlot * 4) + 18 );

				SetDebugBitMask(cTT.Dr7,16 + ((dwSlot - 1)*4),2,DR_EXECUTE); 
				SetDebugBitMask(cTT.Dr7,18 + ((dwSlot - 1)*4),2,dwSize); 
				SetDebugBitMask(cTT.Dr7,(dwSlot - 1)*2,1,1);		

				SetThreadContext(hThread,&cTT);
			}
		}while(Thread32Next(hProcessSnap,&threadEntry32));

		wstringstream sTemp;
		sTemp << "[+] New HardwareBP in " << dwThreadCounter << " Threads placed at " << hex << dwOffset << " in Slot Nr.: " << dwSlot;
		PBLogInfo(sTemp.str());

		CloseHandle(hProcessSnap);
		return true;
	}

	return false;
}

bool clsDebugger::StepOver(DWORD dwNewOffset)
{
	for (vector<BPStruct>::iterator it = SoftwareBPs.begin(); it!= SoftwareBPs.end(); ++it) {
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

	HANDLE hProc,hThread = OpenThread(THREAD_ALL_ACCESS,false,_dwCurTID);

	int iPid = 0;
	for(size_t i = 0;i < PIDs.size(); i++)
	{
		if(PIDs[i].dwPID == _dwCurPID)
			hProc = PIDs[i].hSymInfo;iPid = i;
	}

	if(!PIDs[iPid].bSymLoad)
		PIDs[iPid].bSymLoad = SymInitialize(hProc,NULL,true);
	else
		SymRefreshModuleList(hProc);

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
		bSuccess = StackWalk64(IMAGE_FILE_MACHINE_I386,hProc,hThread, &stackFr,&context,NULL,SymFunctionTableAccess64,SymGetModuleBase64, 0);

		if(!bSuccess)        
			break;

		IMAGEHLP_MODULEW64 imgMod = {0};
		imgMod.SizeOfStruct = sizeof(imgMod);
		bSuccess = SymGetModuleInfoW64(hProc,(DWORD)stackFr.AddrPC.Offset, &imgMod);

		SYMBOL_INFOW *pSymbol;
		DWORD64 dwDisplacement;
		pSymbol = (SYMBOL_INFOW*)new BYTE[sizeof(SYMBOL_INFOW)+MAX_SYM_NAME];
		memset(pSymbol, 0, sizeof(PSYMBOL_INFOW) + MAX_SYM_NAME);
		pSymbol->SizeOfStruct = sizeof(PSYMBOL_INFOW);
		pSymbol->MaxNameLen = MAX_SYM_NAME;

		DWORD dwStackAddr = stackFr.AddrStack.Offset;

		DWORD dwEIP = stackFr.AddrPC.Offset;
		bSuccess = SymFromAddrW(hProc,dwEIP,&dwDisplacement,pSymbol);
		wstring sFuncName = pSymbol->Name;
		bSuccess = SymGetModuleInfoW64(hProc, (DWORD)dwEIP, &imgMod);
		wstring sFuncMod = imgMod.ModuleName;

		DWORD dwReturnTo = stackFr.AddrReturn.Offset;
		bSuccess = SymFromAddrW(hProc,dwReturnTo,&dwDisplacement,pSymbol);
		wstring sReturnToFunc = pSymbol->Name;
		bSuccess = SymGetModuleInfoW64(hProc, (DWORD)dwReturnTo, &imgMod);
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
		
	}while ( stackFr.AddrReturn.Offset != 0 );

	return false;
}

void clsDebugger::AddBreakpointToList(DWORD dwBPType,DWORD dwPID,DWORD dwOffset,DWORD dwKeep)
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

				HardwareBPs.push_back(newBP);
				break;
			}
		}
	}
}

bool clsDebugger::InitBP()
{
	for(size_t i = 0;i < SoftwareBPs.size(); i++)
	{
		wSoftwareBP(SoftwareBPs[i].dwPID,SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwHandle,SoftwareBPs[i].dwSize,SoftwareBPs[i].bOrgByte);
	}

	for(size_t i = 0;i < MemoryBPs.size(); i++)
	{
		wMemoryBP(MemoryBPs[i].dwPID,MemoryBPs[i].dwOffset,MemoryBPs[i].dwSize,MemoryBPs[i].dwHandle);
	}

	for(size_t i = 0;i < HardwareBPs.size(); i++)
	{
		wHardwareBP(HardwareBPs[i].dwPID,HardwareBPs[i].dwOffset,HardwareBPs[i].dwSize,HardwareBPs[i].dwHandle);
	}

	return true;
}

bool clsDebugger::DetachFromProcess(bool bAll,DWORD dwPID)
{
	DWORD dw = NULL;

	if(bAll)
	{
		for(size_t i = 0;i < PIDs.size(); i++)
		{
			if(CheckProcessState(PIDs[i].dwPID) && PIDs[i].bRunning)
			{
				_NormalDebugging = true;
				if(!DebugActiveProcessStop(PIDs[i].dwPID))
				{
					dw = GetLastError();
				}
			}
		}
	}
	else
	{
		if(dwPID != NULL && CheckProcessState(dwPID))
		{
			_NormalDebugging = true;
			return DebugActiveProcessStop(dwPID);
		}
	}
	return false;
}

bool clsDebugger::AttachToProcess(DWORD dwPID)
{
	_NormalDebugging = false;_dwPidToAttach = dwPID;
	return true;//StartDebugging();
}

bool clsDebugger::LoadSymbolForAddr(wstring& sFuncName,wstring& sModName,DWORD dwOffset)
{
	HANDLE hProcess = INVALID_HANDLE_VALUE;
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

	SymRefreshModuleList(hProcess);

	IMAGEHLP_MODULEW imgMod = {0};
	imgMod.SizeOfStruct = sizeof(imgMod);
	BOOL bSymMod = SymGetModuleInfoW(hProcess,dwOffset,&imgMod);

	SYMBOL_INFOW *pSymbol;
	DWORD64 dwDisplacement;
	pSymbol = (SYMBOL_INFOW*)new BYTE[sizeof(SYMBOL_INFOW)+MAX_SYM_NAME];
	memset(pSymbol, 0, sizeof(SYMBOL_INFOW) + MAX_SYM_NAME);
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
	pSymbol->MaxNameLen = MAX_SYM_NAME;

	bool bSuc = SymFromAddrW(hProcess,dwOffset,&dwDisplacement,pSymbol);

	sFuncName = pSymbol->Name;
	sModName = imgMod.ModuleName;

	return bSuc;
}

bool clsDebugger::RemoveBPFromList(DWORD dwOffset,DWORD dwType,DWORD dwPID)
{ 
	switch(dwType)
	{
	case 0:
		for (vector<BPStruct>::iterator it = SoftwareBPs.begin(); it != SoftwareBPs.end(); ++it) {
			if(it->dwOffset == dwOffset && it->dwPID == dwPID)
			{
				SoftwareBPs.erase(it);
				it = SoftwareBPs.begin();
			}
		}
		break;

	case 1:
		for (vector<BPStruct>::iterator it = MemoryBPs.begin(); it != MemoryBPs.end(); ++it) {
			if(it->dwOffset == dwOffset && it->dwPID == dwPID)
			{
				MemoryBPs.erase(it);
				it = MemoryBPs.begin();
			}
		}
		break;

	case 2:
		for (vector<BPStruct>::iterator it = HardwareBPs.begin(); it != HardwareBPs.end(); ++it) {
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

bool clsDebugger::RemoveHWBP(DWORD dwPID)
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
			hThread = OpenThread(THREAD_ALL_ACCESS,false,threadEntry32.th32ThreadID);

		if(hThread != INVALID_HANDLE_VALUE)
		{
			GetThreadContext(hThread,&cTT);

			cTT.Dr7 = 0;
			cTT.Dr0 = 0x00000000;
			cTT.Dr1 = 0x00000000;
			cTT.Dr2 = 0x00000000;
			cTT.Dr3 = 0x00000000;

			SetThreadContext(hThread,&cTT);
		}
	}while(Thread32Next(hProcessSnap,&threadEntry32));
	CloseHandle(hProcessSnap);

	for(size_t i = 0;i < HardwareBPs.size();i++)
		HardwareBPs[i].dwSlot = 0;

	return true;
}

void clsDebugger::SetDebugBitMask(DWORD &dwOld,DWORD dwMask,DWORD dwBits,DWORD dwNewValue)
{
	int mask = (1 << dwBits) - 1; 
	dwOld = (dwOld & ~(mask << dwMask)) | (dwNewValue << dwMask); 
}

DWORD clsDebugger::GetReturnAdressFromStackFrame(DWORD dwEbp,DEBUG_EVENT debug_event)
{
	DWORD dwRetAdr = NULL,
		dwBytesRead;
	HANDLE hProcess;

	int iPid = 0;
	for(size_t i = 0;i < PIDs.size(); i++)
	{
		if(PIDs[i].dwPID == debug_event.dwProcessId)
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

bool clsDebugger::CheckIfExceptionIsBP(EXCEPTION_DEBUG_INFO exInfo)
{
	bool bIsException = false;

	if(exInfo.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP)
		bIsException = true;

	for(size_t i = 0;i < SoftwareBPs.size();i++)
	{
		if((DWORD)exInfo.ExceptionRecord.ExceptionAddress == SoftwareBPs[i].dwOffset)
			bIsException = true;
	}
	for(size_t i = 0;i < MemoryBPs.size();i++)
	{
		if((DWORD)exInfo.ExceptionRecord.ExceptionAddress == MemoryBPs[i].dwOffset)
			bIsException = true;
	}
	for(size_t i = 0;i < HardwareBPs.size();i++)
	{
		if((DWORD)exInfo.ExceptionRecord.ExceptionAddress == HardwareBPs[i].dwOffset)
			bIsException = true;
	}
	return bIsException;
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