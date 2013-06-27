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
#include "clsDebugger.h"
#include "dbghelp.h"
#include "clsAPIImport.h"
#include "clsHelperClass.h"
#include "clsPEManager.h"
#include "clsMemManager.h"
#include "clsDBManager.h"

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
	tcLogString = (PTCHAR)clsMemManager::CAlloc(LOGBUFFER);
	_sCommandLine = L"";
	clsDebuggerSettings tempSet = {0,0,false,false,false,false,false,false,false,false,false,false,false,false};
	dbgSettings = tempSet;
	pThis = this;
	m_waitForGUI = CreateEvent(NULL,false,false,L"hWaitForGUI");
	_hDbgEvent = CreateEvent(NULL,false,false,L"hDebugEvent");
}

clsDebugger::clsDebugger(wstring sTarget)
{
	_sTarget = sTarget;
	_NormalDebugging = true;
	_isDebugging = false;
	tcLogString = (PTCHAR)clsMemManager::CAlloc(LOGBUFFER);
	_sCommandLine = L"";
	clsDebuggerSettings tempSet = {0,0,false,false,false,false,false,false,false,false,false,false,false,false};
	dbgSettings = tempSet;
	pThis = this;
	m_waitForGUI = CreateEvent(NULL,false,false,L"hWaitForGUI");
	_hDbgEvent = CreateEvent(NULL,false,false,L"hDebugEvent");
}

clsDebugger::~clsDebugger()
{
	CleanWorkSpace();

	for(vector<BPStruct>::const_iterator i = SoftwareBPs.cbegin(); i != SoftwareBPs.cend(); ++i)
		clsMemManager::CFree(i->moduleName);

	for(vector<BPStruct>::const_iterator i = MemoryBPs.cbegin(); i != MemoryBPs.cend(); ++i)
		clsMemManager::CFree(i->moduleName);

	for(vector<BPStruct>::const_iterator i = HardwareBPs.cbegin(); i != HardwareBPs.cend(); ++i)
		clsMemManager::CFree(i->moduleName);

	SoftwareBPs.clear();
	MemoryBPs.clear();
	HardwareBPs.clear();

	clsMemManager::CFree(tcLogString);

	CloseHandle(m_waitForGUI);
	CloseHandle(_hDbgEvent);
}

void clsDebugger::CleanWorkSpace()
{
	for(vector<PIDStruct>::const_iterator i = PIDs.cbegin(); i != PIDs.cend(); ++i)
	{
		SymCleanup(i->hProc);
		clsMemManager::CFree(i->sFileName);
	}

	for(vector<DLLStruct>::const_iterator i = DLLs.cbegin(); i != DLLs.cend(); ++i)
	{
		clsMemManager::CFree(i->sPath);
	}
	
	for (vector<BPStruct>::iterator it = SoftwareBPs.begin(); it != SoftwareBPs.end();++it)
	{
		if(it->dwHandle == 0x2)
		{
			clsMemManager::CFree(it->moduleName);
			SoftwareBPs.erase(it);
			it = SoftwareBPs.begin();
		}
		if(SoftwareBPs.size() <= 0)
			break;
	}

	PIDs.clear();
	DLLs.clear();
	TIDs.clear();
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

	PTCHAR tcFilename = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	if (!GetMappedFileName(GetCurrentProcess(),pMem,tcFilename,MAX_PATH)) 
	{
		UnmapViewOfFile(pMem);
		CloseHandle(hFileMap);
		clsMemManager::CFree(tcFilename);
		return NULL;
	}

	PTCHAR tcTemp = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	tcTemp[0] = '\0';

	if (!GetLogicalDriveStrings(255-1, tcTemp)) 
	{
		UnmapViewOfFile(pMem);
		CloseHandle(hFileMap);
		clsMemManager::CFree(tcFilename);
		clsMemManager::CFree(tcTemp);
		return NULL;
	}

	PTCHAR tcName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	PTCHAR tcFile = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
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

	clsMemManager::CFree(tcName);
	clsMemManager::CFree(tcTemp);
	clsMemManager::CFree(tcFilename);
	UnmapViewOfFile(pMem);
	CloseHandle(hFileMap);

	return tcFile;
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
		memset(tcLogString,0x00,LOGBUFFER);
		swprintf_s(tcLogString,LOGBUFFERCHAR,L"[+] Attached to Process");
		PBLogInfo();
		DebuggingLoop();
		_NormalDebugging = true;
		return;
	}
	_isDebugging = false;
	emit OnDebuggerTerminated();
}

void clsDebugger::NormalDebugging(LPVOID pDebProc)
{
	clsDebugger *pThis = (clsDebugger*)pDebProc;

	ZeroMemory(&_si, sizeof(_si));
	_si.cb = sizeof(_si);
	ZeroMemory(&_pi, sizeof(_pi));

	DWORD dwCreationFlag = 0x2;

	if(dbgSettings.bDebugChilds == true)
		dwCreationFlag = 0x1;

	if(CreateProcess(_sTarget.c_str(),(LPWSTR)_sCommandLine.c_str(),NULL,NULL,false,dwCreationFlag,NULL,NULL,&_si,&_pi))
		DebuggingLoop();
	else
	{
		_isDebugging = false;
		emit OnDebuggerTerminated();
	}
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

				emit OnNewPID((wstring)tcDllFilepath,debug_event.dwProcessId);

				//clsDBManager::OpenNewFile(debug_event.dwProcessId,(wstring)tcDllFilepath);

				size_t iPid = 0;
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
				
				if(dbgSettings.bBreakOnTLS)
				{
					DWORD64 tlsCallback = clsPEManager::getTLSCallbackOffset((wstring)tcDllFilepath,debug_event.dwProcessId);
					if(tlsCallback > 0)
						AddBreakpointToList(NULL,2,-1,(quint64)debug_event.u.CreateProcessInfo.lpBaseOfImage + tlsCallback,NULL,0x2);
				}
				
				InitBP();

				// Insert Main Thread to List
				PBThreadInfo(debug_event.dwProcessId,clsHelperClass::GetMainThread(debug_event.dwProcessId),(quint64)debug_event.u.CreateThread.lpStartAddress,false,0,true);
				
				if(dbgSettings.bBreakOnNewPID)
					dwContinueStatus = CallBreakDebugger(&debug_event,0);

				break;
			}
		case CREATE_THREAD_DEBUG_EVENT:
			PBThreadInfo(debug_event.dwProcessId,debug_event.dwThreadId,(quint64)debug_event.u.CreateThread.lpStartAddress,false,0,true);
			InitBP();

			if(dbgSettings.bBreakOnNewTID)
				dwContinueStatus = CallBreakDebugger(&debug_event,0);

			break;

		case EXIT_THREAD_DEBUG_EVENT:
			PBThreadInfo(debug_event.dwProcessId,debug_event.dwThreadId,NULL,false,debug_event.u.ExitThread.dwExitCode,false);

			if(dbgSettings.bBreakOnExTID)
				dwContinueStatus = CallBreakDebugger(&debug_event,0);

			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			{
				PBProcInfo(debug_event.dwProcessId,L"",NULL,debug_event.u.ExitProcess.dwExitCode,NULL);
				SymCleanup(debug_event.u.CreateProcessInfo.hProcess);

				emit DeletePEManagerObject(L"",debug_event.dwProcessId);
				clsDBManager::CloseFile(debug_event.dwProcessId);

				bool bStillOneRunning = false;
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

				if(dbgSettings.bBreakOnExPID)
					dwContinueStatus = CallBreakDebugger(&debug_event,0);
			}
			break;

		case LOAD_DLL_DEBUG_EVENT:
			{
				PTCHAR sDLLFileName = GetFileNameFromHandle(debug_event.u.LoadDll.hFile); 
				PBDLLInfo(sDLLFileName,debug_event.dwProcessId,(quint64)debug_event.u.LoadDll.lpBaseOfDll,true);

				HANDLE hProc = 0;
				size_t iPid = 0;

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

				if(dbgSettings.bBreakOnNewDLL)
					dwContinueStatus = CallBreakDebugger(&debug_event,0);
			}
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			for(size_t i = 0;i < DLLs.size(); i++)
			{
				if(DLLs[i].dwBaseAdr == (quint64)debug_event.u.UnloadDll.lpBaseOfDll && DLLs[i].dwPID == debug_event.dwProcessId)
				{
					PBDLLInfo(DLLs[i].sPath,DLLs[i].dwPID,DLLs[i].dwBaseAdr,false, i);
					SymUnloadModule64(GetCurrentProcessHandle(DLLs[i].dwPID),DLLs[i].dwBaseAdr);
					break;
				}
			}

			if(dbgSettings.bBreakOnExDLL)
				dwContinueStatus = CallBreakDebugger(&debug_event,0);

			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			{
				PTCHAR wMsg = (PTCHAR)clsMemManager::CAlloc(debug_event.u.DebugString.nDebugStringLength * sizeof(TCHAR));
				HANDLE hProcess = GetCurrentProcessHandle(debug_event.dwProcessId);

				if(debug_event.u.DebugString.fUnicode)
					ReadProcessMemory(hProcess,debug_event.u.DebugString.lpDebugStringData,wMsg,debug_event.u.DebugString.nDebugStringLength,NULL);
				else
				{
					size_t countConverted = NULL;
					PCHAR Msg = (PCHAR)clsMemManager::CAlloc(debug_event.u.DebugString.nDebugStringLength * sizeof(CHAR));
					
					ReadProcessMemory(hProcess,debug_event.u.DebugString.lpDebugStringData,Msg,debug_event.u.DebugString.nDebugStringLength,NULL);	
					mbstowcs_s(&countConverted,wMsg,debug_event.u.DebugString.nDebugStringLength,Msg,debug_event.u.DebugString.nDebugStringLength);
					clsMemManager::CFree(Msg);
				}
				PBDbgString(wMsg,debug_event.dwProcessId);
				//clsMemManager::CFree(wMsg);
				break;
			}
		case EXCEPTION_DEBUG_EVENT:
			{
				EXCEPTION_DEBUG_INFO exInfo = debug_event.u.Exception;
				bool bIsEP = false,bIsBP = false,bIsKernelBP = false;

				size_t iPid = 0;
				for(size_t i = 0;i < PIDs.size(); i++)
					if(PIDs[i].dwPID == debug_event.dwProcessId)
						iPid = i;

				if(!CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,exInfo.ExceptionRecord.ExceptionCode,debug_event.dwProcessId,false))
					PBExceptionInfo((quint64)exInfo.ExceptionRecord.ExceptionAddress,exInfo.ExceptionRecord.ExceptionCode,debug_event.dwProcessId,debug_event.dwThreadId);

				switch (exInfo.ExceptionRecord.ExceptionCode)
				{
				case 0x4000001f: // Breakpoint in x86 Process which got executed in a x64 environment
					if(PIDs[iPid].bKernelBP && !PIDs[iPid].bWOW64KernelBP)
					{
						if(dbgSettings.bBreakOnSystemEP)
							dwContinueStatus = CallBreakDebugger(&debug_event,0);
						else
							dwContinueStatus = CallBreakDebugger(&debug_event,3);

						PIDs[iPid].bWOW64KernelBP = true;
						bIsKernelBP = true;
					}

				case EXCEPTION_BREAKPOINT:
					{
						bool bStepOver = false;

						if(!PIDs[iPid].bKernelBP)
						{
							if(dbgSettings.bBreakOnSystemEP)
								dwContinueStatus = CallBreakDebugger(&debug_event,0);
							else
								dwContinueStatus = CallBreakDebugger(&debug_event,3);

							PIDs[iPid].bKernelBP = true;
							bIsKernelBP = true;
						}
						else if(PIDs[iPid].bKernelBP)
						{
							if((quint64)exInfo.ExceptionRecord.ExceptionAddress == PIDs[iPid].dwEP)
							{
								bIsEP = true;
								
								UpdateOffsetsBPs();
								emit UpdateOffsetsPatches(PIDs[iPid].hProc,PIDs[iPid].dwPID);

								InitBP();
							}

							bIsBP = CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,exInfo.ExceptionRecord.ExceptionCode,debug_event.dwProcessId,true);
							if(bIsBP)
							{
								HANDLE hProc = PIDs[iPid].hProc;

								for(size_t i = 0;i < SoftwareBPs.size(); i++)
								{
									if((quint64)exInfo.ExceptionRecord.ExceptionAddress == SoftwareBPs[i].dwOffset && 
										(SoftwareBPs[i].dwPID == debug_event.dwProcessId || SoftwareBPs[i].dwPID == -1))
									{
										bool b = WriteProcessMemory(hProc,(LPVOID)SoftwareBPs[i].dwOffset,(LPVOID)&SoftwareBPs[i].bOrgByte,SoftwareBPs[i].dwSize,NULL);
										bool c = FlushInstructionCache(hProc,(LPVOID)SoftwareBPs[i].dwOffset,SoftwareBPs[i].dwSize);

										if(SoftwareBPs[i].dwHandle != 0x2)
											SoftwareBPs[i].bRestoreBP = true;
										else
											if(!bIsEP)
												bStepOver = true;
									}
								}

								SetThreadContextHelper(true,true,debug_event.dwThreadId,debug_event.dwProcessId);
								PIDs[iPid].bTrapFlag = true;
								PIDs[iPid].dwBPRestoreFlag = 0x2;

								if(bIsEP && !dbgSettings.bBreakOnModuleEP)
									dwContinueStatus = CallBreakDebugger(&debug_event,2);
								else
								{
									if(!bStepOver)
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
									}
									dwContinueStatus = CallBreakDebugger(&debug_event,0);
								}
							}
						}
						break;
					}
				case 0x4000001E: // Single Step in x86 Process which got executed in a x64 environment
				case EXCEPTION_SINGLE_STEP:
					{
						bIsBP = CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,exInfo.ExceptionRecord.ExceptionCode,debug_event.dwProcessId,true);

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

								if(PIDs[iPid].bTraceFlag)
								{
									_bSingleStepFlag = true;
									SetThreadContextHelper(false,true,debug_event.dwThreadId,debug_event.dwProcessId);
									qtDLGTrace::addTraceData((quint64)debug_event.u.Exception.ExceptionRecord.ExceptionAddress,debug_event.dwProcessId,debug_event.dwThreadId);
								}
								else
									dwContinueStatus = CallBreakDebugger(&debug_event,0);
							}
						}
						else
						{
							if(_bSingleStepFlag)
							{
								_bSingleStepFlag = false;
								bIsBP = true;

								if(PIDs[iPid].bTraceFlag)
								{
									_bSingleStepFlag = true;
									SetThreadContextHelper(false,true,debug_event.dwThreadId,debug_event.dwProcessId);
									qtDLGTrace::addTraceData((quint64)debug_event.u.Exception.ExceptionRecord.ExceptionAddress,debug_event.dwProcessId,debug_event.dwThreadId);		
								}
								else
									dwContinueStatus = CallBreakDebugger(&debug_event,0);
							}
						}
						break;
					}
				case EXCEPTION_GUARD_PAGE:
					{
						bIsBP = CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,exInfo.ExceptionRecord.ExceptionCode,debug_event.dwProcessId,true);
						if(bIsBP)
						{
							SetThreadContextHelper(false,true,debug_event.dwThreadId,debug_event.dwProcessId);
							PIDs[iPid].dwBPRestoreFlag = 0x4;
							PIDs[iPid].bTrapFlag = true;

							for(size_t i = 0;i < MemoryBPs.size();i++)
							{
								if(MemoryBPs[i].dwOffset == (quint64)exInfo.ExceptionRecord.ExceptionAddress)
								{
									MemoryBPs[i].bRestoreBP = true;

									memset(tcLogString,0x00,LOGBUFFER);
#ifdef _AMD64_
									swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on Memory BP at %016I64X",MemoryBPs[i].dwOffset);
#else
									swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on Memory BP at %08X",(DWORD)MemoryBPs[i].dwOffset);
#endif
									PBLogInfo();
								}
							}
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
				{
					if(dbgSettings.dwDefaultExceptionMode == 1)
						dwContinueStatus = CallBreakDebugger(&debug_event,dbgSettings.dwDefaultExceptionMode);
					else if(dbgSettings.bUseExceptionAssist)
					{
						emit AskForException((DWORD)debug_event.u.Exception.ExceptionRecord.ExceptionCode);
						WaitForSingleObject(m_waitForGUI,INFINITE);

						if(m_continueWithException >= 10)
						{
							m_continueWithException -= 10;
							CustomExceptionAdd((DWORD)debug_event.u.Exception.ExceptionRecord.ExceptionCode,m_continueWithException,NULL);
						}

						dwContinueStatus = CallBreakDebugger(&debug_event,m_continueWithException);
					}
					else
						dwContinueStatus = CallBreakDebugger(&debug_event,0);
				}
			}			
			break;
		}
		ContinueDebugEvent(debug_event.dwProcessId,debug_event.dwThreadId,dwContinueStatus);
		dwContinueStatus = DBG_CONTINUE;
	}
	_isDebugging = false;

	memset(tcLogString,0x00,LOGBUFFER);
	swprintf_s(tcLogString,LOGBUFFERCHAR,L"[-] Debugging finished!");
	PBLogInfo();
	CleanWorkSpace();
	
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
	case 3:
		return DBG_EXCEPTION_HANDLED;
	default:
		return DBG_EXCEPTION_NOT_HANDLED;
	}
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

bool clsDebugger::CheckIfExceptionIsBP(quint64 dwExceptionOffset,quint64 dwExceptionType,DWORD dwPID,bool bClearTrapFlag)
{
	size_t iPID = NULL;
	for(size_t i = 0;i < PIDs.size();i++)
		if(PIDs[i].dwPID == dwPID)
			iPID = i;

	if(PIDs[iPID].bTrapFlag)
	{
		if(bClearTrapFlag)
			PIDs[iPID].bTrapFlag = false;
		return true;
	}

	if((dwExceptionType == EXCEPTION_SINGLE_STEP || dwExceptionType == 0x4000001e) && _bSingleStepFlag)
		return true;

	if(dwExceptionType == EXCEPTION_BREAKPOINT ||
		dwExceptionType == 0x4000001f ||
		dwExceptionType == EXCEPTION_GUARD_PAGE)
	{
		for(size_t i = 0;i < SoftwareBPs.size();i++)
			if(dwExceptionOffset == SoftwareBPs[i].dwOffset && (SoftwareBPs[i].dwPID == dwPID || SoftwareBPs[i].dwPID == -1))
				return true;
		for(size_t i = 0;i < MemoryBPs.size();i++)
			if(dwExceptionOffset == MemoryBPs[i].dwOffset && (MemoryBPs[i].dwPID == dwPID || MemoryBPs[i].dwPID == -1))
				return true;
		for(size_t i = 0;i < HardwareBPs.size();i++)
			if(dwExceptionOffset == HardwareBPs[i].dwOffset && (HardwareBPs[i].dwPID == dwPID || HardwareBPs[i].dwPID == -1))
			return true;
	}
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

bool clsDebugger::SetThreadContextHelper(bool bDecIP,bool bSetTrapFlag, DWORD dwThreadID, DWORD dwPID)
{
	HANDLE hThread = OpenThread(THREAD_GETSET_CONTEXT,false,dwThreadID);
	if(hThread == INVALID_HANDLE_VALUE) 
		return false;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;

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
	return _pi.hProcess;
}

HANDLE clsDebugger::GetProcessHandleByPID(DWORD PID)
{
	if(pThis != NULL)
		return pThis->GetCurrentProcessHandle(PID);

	return NULL;
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

void clsDebugger::HandleForException(int handleException)
{
	m_continueWithException = handleException;
	PulseEvent(m_waitForGUI);
}