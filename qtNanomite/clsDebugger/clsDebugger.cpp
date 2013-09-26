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

clsDebugger::clsDebugger(clsBreakpointManager *pBPManager) :
	m_pBreakpointManager(pBPManager)
{
	ZeroMemory(&_si, sizeof(_si));
	_si.cb = sizeof(_si);
	ZeroMemory(&_pi, sizeof(_pi));

	ZeroMemory(&dbgSettings, sizeof(clsDebuggerSettings));

	_NormalDebugging = true;
	_isDebugging = false;
	m_debuggerBreak = false;
	
	pThis = this;

	tcLogString = (PTCHAR)clsMemManager::CAlloc(LOGBUFFER);
	_sCommandLine = L"";

	m_waitForGUI = CreateEvent(NULL,false,false,L"hWaitForGUI");
	_hDbgEvent = CreateEvent(NULL,false,false,L"hDebugEvent");

	//SymSetOptions(SYMOPT_DEFERRED_LOADS);
}

clsDebugger::~clsDebugger()
{
	CleanWorkSpace();

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

	m_pBreakpointManager->BreakpointCleanup();

	PIDs.clear();
	DLLs.clear();
	TIDs.clear();

	if(_pi.hProcess != NULL && _pi.hThread != NULL)
	{
		CloseHandle(_pi.hProcess);
		CloseHandle(_pi.hThread);
		ZeroMemory(&_si, sizeof(_si));
		_si.cb = sizeof(_si);
		ZeroMemory(&_pi, sizeof(_pi));
	}
	
	ZeroMemory(&m_dbgPI, sizeof(m_dbgPI));
}

PTCHAR clsDebugger::GetFileNameFromModuleBase(HANDLE processHandle, LPVOID imageBase) 
{
	PTCHAR tcFilename = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	if (!GetMappedFileName(processHandle, imageBase, tcFilename, MAX_PATH)) 
	{
		clsMemManager::CFree(tcFilename);
		return NULL;
	}

	PTCHAR tcTemp = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	if (!GetLogicalDriveStrings(MAX_PATH - 1, tcTemp)) 
	{
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

		if(QueryDosDevice(tcDrive, tcName, MAX_PATH))
		{
			size_t uNameLen = wcslen(tcName);
			if(uNameLen < MAX_PATH) 
			{
				bFound = _wcsnicmp(tcFilename, tcName, uNameLen) == 0;
				if(bFound)
					swprintf_s(tcFile, MAX_PATH, L"%s%s", tcDrive, (tcFilename + uNameLen));
			}
		}

		while (*p++);
	} while (!bFound && *p);

	clsMemManager::CFree(tcName);
	clsMemManager::CFree(tcTemp);
	clsMemManager::CFree(tcFilename);

	return tcFile;
}

void clsDebugger::run()
{
	_bStopDebugging = false;
	if(_dwPidToAttach != 0 && !_NormalDebugging)
	{
		CleanWorkSpace();
		_isDebugging = true;
		_bSingleStepFlag = false;

		AttachedDebugging();
	}	
	else
	{
		if(_sTarget.length() <= 0 || _isDebugging)
			return;

		CleanWorkSpace();
		_isDebugging = true;
		_bSingleStepFlag = false;
		
		NormalDebugging();
	}
}

void clsDebugger::AttachedDebugging()
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

void clsDebugger::NormalDebugging()
{
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
	ZeroMemory(&m_dbgPI, sizeof(m_dbgPI));

	DebugSetProcessKillOnExit(false);

	while(bContinueDebugging && _isDebugging)
	{ 
		if (!WaitForDebugEvent(&debug_event, INFINITE))
			bContinueDebugging = false;

		if(_bStopDebugging)
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

				if(m_dbgPI.hProcess == NULL)
				{
					m_dbgPI.hProcess = debug_event.u.CreateProcessInfo.hProcess;
					m_dbgPI.hThread = debug_event.u.CreateProcessInfo.hThread;
					m_dbgPI.dwProcessId = debug_event.dwProcessId;
					m_dbgPI.dwThreadId = debug_event.dwThreadId;
				}

				PTCHAR tcDllFilepath = GetFileNameFromModuleBase(hProc, debug_event.u.CreateProcessInfo.lpBaseOfImage);
				PBProcInfo(debug_event.dwProcessId,tcDllFilepath,(quint64)debug_event.u.CreateProcessInfo.lpStartAddress,-1,hProc);

				emit OnNewPID((wstring)tcDllFilepath,debug_event.dwProcessId);

				//clsDBManager::OpenNewFile(debug_event.dwProcessId,(wstring)tcDllFilepath);

				PIDStruct *pCurrentPID = GetCurrentPIDDataPointer(debug_event.dwProcessId);

				pCurrentPID->bSymLoad = SymInitialize(hProc,NULL,false);
				if(!pCurrentPID->bSymLoad)
				{
					memset(tcLogString,0x00,LOGBUFFER);
					swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Could not load symbols for Process(%X)",debug_event.dwProcessId);
					PBLogInfo();
				}
				else
					SymLoadModuleExW(hProc,NULL,tcDllFilepath,0,(quint64)debug_event.u.CreateProcessInfo.lpBaseOfImage,0,0,0);

				m_pBreakpointManager->BreakpointAdd(SOFTWARE_BP, NULL, debug_event.dwProcessId, (quint64)debug_event.u.CreateProcessInfo.lpStartAddress, 1, BP_DONOTKEEP);
				
				if(dbgSettings.bBreakOnTLS)
				{
					QList<quint64> tlsCallback = clsPEManager::getTLSCallbackOffset((wstring)tcDllFilepath,debug_event.dwProcessId);
					if(tlsCallback.length() > 0)
					{
						for(int i = 0; i < tlsCallback.count(); i++)
						{
							m_pBreakpointManager->BreakpointAdd(SOFTWARE_BP, NULL, debug_event.dwProcessId, (quint64)debug_event.u.CreateProcessInfo.lpBaseOfImage + tlsCallback.at(i), 1, BP_DONOTKEEP);
						}
					}						
				}
				
				m_pBreakpointManager->BreakpointInit(debug_event.dwProcessId);

				// Insert Main Thread to List
				PBThreadInfo(debug_event.dwProcessId,clsHelperClass::GetMainThread(debug_event.dwProcessId),(quint64)debug_event.u.CreateProcessInfo.lpStartAddress,false,0,true);
				
				if(dbgSettings.bBreakOnNewPID)
					dwContinueStatus = CallBreakDebugger(&debug_event,0);

				CloseHandle(debug_event.u.CreateProcessInfo.hFile);
				break;
			}
		case CREATE_THREAD_DEBUG_EVENT:
			PBThreadInfo(debug_event.dwProcessId,debug_event.dwThreadId,(quint64)debug_event.u.CreateThread.lpStartAddress,false,0,true);
			m_pBreakpointManager->BreakpointInit(debug_event.dwProcessId, true);

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
				SymCleanup(GetCurrentProcessHandle(debug_event.dwProcessId));

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
				PIDStruct *pCurrentPID = GetCurrentPIDDataPointer(debug_event.dwProcessId);
				PTCHAR sDLLFileName = GetFileNameFromModuleBase(pCurrentPID->hProc, debug_event.u.LoadDll.lpBaseOfDll); 
				PBDLLInfo(sDLLFileName,debug_event.dwProcessId,(quint64)debug_event.u.LoadDll.lpBaseOfDll,true);

				if(pCurrentPID->bSymLoad && dbgSettings.bAutoLoadSymbols)
					SymLoadModuleExW(pCurrentPID->hProc,NULL,sDLLFileName,0,(quint64)debug_event.u.LoadDll.lpBaseOfDll,0,0,0);
				else
				{
					memset(tcLogString,0x00,LOGBUFFER);
					swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Could not load symbols for DLL: %s",sDLLFileName);
					PBLogInfo();
				}

				if(dbgSettings.bBreakOnNewDLL)
					dwContinueStatus = CallBreakDebugger(&debug_event,0);

				CloseHandle(debug_event.u.LoadDll.hFile);
			}
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			{
				DLLStruct *pCurrent = NULL;
				size_t countDLL = DLLs.size();

				for(size_t i = 0;i < countDLL; i++)
				{
					pCurrent = &DLLs[i];

					if(pCurrent->dwBaseAdr == (quint64)debug_event.u.UnloadDll.lpBaseOfDll && pCurrent->dwPID == debug_event.dwProcessId)
					{
						PBDLLInfo(NULL, NULL, NULL, false, pCurrent);
						SymUnloadModule64(GetCurrentProcessHandle(debug_event.dwProcessId), pCurrent->dwBaseAdr);
						break;
					}
				}

				if(dbgSettings.bBreakOnExDLL)
					dwContinueStatus = CallBreakDebugger(&debug_event,0);

				break;
			}
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
				EXCEPTION_RECORD exInfo = debug_event.u.Exception.ExceptionRecord;
				bool	bIsEP			= false,
						bIsBP			= false,
						bIsKernelBP		= false;
				PIDStruct *pCurrentPID = GetCurrentPIDDataPointer(debug_event.dwProcessId);

				switch (exInfo.ExceptionCode)
				{
				case 0x4000001f: // Breakpoint in x86 Process which got executed in a x64 environment
					if(pCurrentPID->bKernelBP && !pCurrentPID->bWOW64KernelBP)
					{
						memset(tcLogString,0x00,LOGBUFFER);
#ifdef _AMD64_
						swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] WOW64 Kernel EP - PID %06X - %016I64X", debug_event.dwProcessId, (quint64)exInfo.ExceptionAddress);
#else
						swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] WOW64 Kernel EP - PID %06X - %08X", debug_event.dwProcessId, (DWORD)exInfo.ExceptionAddress);
#endif
						PBLogInfo();
							
						if(dbgSettings.bBreakOnSystemEP)
							dwContinueStatus = CallBreakDebugger(&debug_event,0);
						else
							dwContinueStatus = CallBreakDebugger(&debug_event,3);

						pCurrentPID->bWOW64KernelBP = true;
						bIsKernelBP = true;
					}

				case EXCEPTION_BREAKPOINT:
					{
						bool bStepOver = false;

						if(!pCurrentPID->bKernelBP)
						{
							memset(tcLogString,0x00,LOGBUFFER);
	#ifdef _AMD64_
							swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Kernel EP - PID %06X - %016I64X", debug_event.dwProcessId, (quint64)exInfo.ExceptionAddress);
	#else
							swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Kernel EP - PID %06X - %08X", debug_event.dwProcessId, (DWORD)exInfo.ExceptionAddress);
	#endif
							PBLogInfo();
								
							if(dbgSettings.bBreakOnSystemEP)
								dwContinueStatus = CallBreakDebugger(&debug_event,0);
							else
								dwContinueStatus = CallBreakDebugger(&debug_event,3);

							pCurrentPID->bKernelBP = true;
							bIsKernelBP = true;
						}
						else if(pCurrentPID->bKernelBP)
						{
							if((quint64)exInfo.ExceptionAddress == pCurrentPID->dwEP)
							{
								bIsEP = true;
								
								m_pBreakpointManager->BreakpointUpdateOffsets();
								emit UpdateOffsetsPatches(pCurrentPID->hProc,pCurrentPID->dwPID);

								m_pBreakpointManager->BreakpointInit(debug_event.dwProcessId);
							}

							bIsBP = CheckIfExceptionIsBP(pCurrentPID, (quint64)exInfo.ExceptionAddress, EXCEPTION_BREAKPOINT, true);
							if(bIsBP)
							{
								bool bpNeedsReplace = false;
								BPStruct *pCurrentBP;

								if(m_pBreakpointManager->BreakpointFind((DWORD64)exInfo.ExceptionAddress, SOFTWARE_BP, debug_event.dwProcessId, true, &pCurrentBP))
								{
									bool b = WriteProcessMemory(pCurrentPID->hProc, (LPVOID)pCurrentBP->dwOffset, (LPVOID)pCurrentBP->bOrgByte, pCurrentBP->dwSize,NULL);
									bool c = FlushInstructionCache(pCurrentPID->hProc, (LPVOID)pCurrentBP->dwOffset, pCurrentBP->dwSize);

									switch(pCurrentBP->dwHandle)
									{
									case BP_KEEP: // normal breakpoint
										pCurrentBP->bRestoreBP = true;
										bpNeedsReplace = true;

										break;
									case BP_STEPOVER: // StepOver BP
										if(!bIsEP)
											bStepOver = true;

										m_pBreakpointManager->BreakpointRemove(pCurrentBP->dwOffset,SOFTWARE_BP);
										break;
									case BP_TRACETO: // Trace End BP
										_bSingleStepFlag = false;

										m_pBreakpointManager->BreakpointRemove(pCurrentBP->dwOffset,SOFTWARE_BP);
										break;

									default:
										pCurrentBP->bRestoreBP = true;
										break;
									}
								}
								
								if(bpNeedsReplace || bStepOver)
								{
									SetThreadContextHelper(true, true, debug_event.dwThreadId, pCurrentPID);
									pCurrentPID->bTrapFlag = true;
									pCurrentPID->dwBPRestoreFlag = RESTORE_BP_SOFTWARE;
								}
								else
									SetThreadContextHelper(true, false, debug_event.dwThreadId, pCurrentPID);

								if(bIsEP && !dbgSettings.bBreakOnModuleEP)
									dwContinueStatus = CallBreakDebugger(&debug_event,2);
								else
								{
									if(!bStepOver)
									{
										memset(tcLogString,0x00,LOGBUFFER);
										if(bIsEP)
#ifdef _AMD64_
											swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Entrypoint - PID %06X - %016I64X", debug_event.dwProcessId, (quint64)exInfo.ExceptionAddress);
										else
											swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Software BP - PID %06X - %016I64X", debug_event.dwProcessId, (quint64)exInfo.ExceptionAddress);
#else
											swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Entrypoint - PID %06X - %08X", debug_event.dwProcessId, (DWORD)exInfo.ExceptionAddress);
										else
											swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Software BP - PID %06X - %08X", debug_event.dwProcessId, (DWORD)exInfo.ExceptionAddress);
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
						if(pCurrentPID->bTraceFlag && _bSingleStepFlag)
						{
							bIsBP = true;
							SetThreadContextHelper(false, true, debug_event.dwThreadId, pCurrentPID);
							qtDLGTrace::addTraceData((quint64)exInfo.ExceptionAddress, debug_event.dwProcessId, debug_event.dwThreadId);
							break;
						}
						else if(_bSingleStepFlag)
						{
							_bSingleStepFlag = false;
							bIsBP = true;

							dwContinueStatus = CallBreakDebugger(&debug_event,0);
							break;
						}

						bIsBP = CheckIfExceptionIsBP(pCurrentPID, (quint64)exInfo.ExceptionAddress, EXCEPTION_SINGLE_STEP, true, false);
						if(bIsBP)
						{
							if(pCurrentPID->dwBPRestoreFlag == RESTORE_BP_SOFTWARE) // Restore SoftwareBP
							{
								BPStruct *pCurrentBP = NULL;
								int countSoftwareBP = m_pBreakpointManager->SoftwareBPs.size();

								for(int i = 0; i < countSoftwareBP; i++)
								{
									pCurrentBP = &m_pBreakpointManager->SoftwareBPs[i];

									if(pCurrentBP->bRestoreBP &&
										pCurrentBP->dwHandle == BP_KEEP &&
										(pCurrentBP->dwPID == debug_event.dwProcessId || pCurrentBP->dwPID == -1))
									{
										clsBreakpointSoftware::wSoftwareBP(pCurrentBP->dwPID, pCurrentBP->dwOffset, pCurrentBP->dwSize, &pCurrentBP->bOrgByte);
										pCurrentBP->bRestoreBP = false;

										break;
									}									
								}

								pCurrentPID->bTrapFlag = false;
								pCurrentPID->dwBPRestoreFlag = RESTORE_NON;
							}
							else if(pCurrentPID->dwBPRestoreFlag == RESTORE_BP_MEMORY) // Restore MemBP
							{
								BPStruct *pCurrentBP = NULL;
								int countMemoryBP = m_pBreakpointManager->MemoryBPs.size();

								for(int i = 0; i < countMemoryBP; i++)
								{
									pCurrentBP = &m_pBreakpointManager->MemoryBPs[i];

									if(pCurrentBP->bRestoreBP &&
										pCurrentBP->dwHandle == BP_KEEP &&
										(pCurrentBP->dwPID == debug_event.dwProcessId || pCurrentBP->dwPID == -1))
									{
										clsBreakpointMemory::wMemoryBP(pCurrentBP->dwPID, pCurrentBP->dwOffset, pCurrentBP->dwSize, pCurrentBP->dwTypeFlag, &pCurrentBP->dwOldProtection);
										pCurrentBP->bRestoreBP = false;

										break;
									}									
								}

								pCurrentPID->bTrapFlag = false;
								pCurrentPID->dwBPRestoreFlag = RESTORE_NON;
							}
							else if(pCurrentPID->dwBPRestoreFlag == RESTORE_BP_HARDWARE) // Restore HwBp
							{
								BPStruct *pCurrentBP = NULL;
								int countHardwareBP = m_pBreakpointManager->HardwareBPs.size();

								for(int i = 0; i < countHardwareBP; i++)
								{
									pCurrentBP = &m_pBreakpointManager->HardwareBPs[i];

									if(pCurrentBP->bRestoreBP &&
										pCurrentBP->dwHandle == BP_KEEP &&
										(pCurrentBP->dwPID == debug_event.dwProcessId || pCurrentBP->dwPID == -1))
									{
										clsBreakpointHardware::wHardwareBP(debug_event.dwProcessId, pCurrentBP->dwOffset, pCurrentBP->dwSize, pCurrentBP->dwSlot, pCurrentBP->dwTypeFlag);
										pCurrentBP->bRestoreBP = false;

										break;
									}
								}

								pCurrentPID->bTrapFlag = false;
								pCurrentPID->dwBPRestoreFlag = RESTORE_NON;
							}
							else if(pCurrentPID->dwBPRestoreFlag == RESTORE_NON) // First time hit HwBP
							{
								BPStruct *pCurrentBP;
								if(m_pBreakpointManager->BreakpointFind((DWORD64)exInfo.ExceptionAddress, HARDWARE_BP, debug_event.dwProcessId, true, &pCurrentBP))
								{
									clsBreakpointHardware::dHardwareBP(debug_event.dwProcessId, pCurrentBP->dwOffset, pCurrentBP->dwSlot);

									memset(tcLogString,0x00,LOGBUFFER);
#ifdef _AMD64_
									swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Hardware BP - PID: %06X - %016I64X", debug_event.dwProcessId, pCurrentBP->dwOffset);
#else
									swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Hardware BP - PID: %06X - %08X", debug_event.dwProcessId, (DWORD)pCurrentBP->dwOffset);
#endif
									PBLogInfo();

									pCurrentBP->bRestoreBP = true;
									dwContinueStatus = CallBreakDebugger(&debug_event,0);

									pCurrentPID->dwBPRestoreFlag = RESTORE_BP_HARDWARE;
									pCurrentPID->bTrapFlag = true;

									SetThreadContextHelper(false, true, debug_event.dwThreadId, pCurrentPID);
								}
							}
							else
								bIsBP = false;
						}

						break;
					}
				case EXCEPTION_ACCESS_VIOLATION:
				case EXCEPTION_GUARD_PAGE:
					{
						bIsBP = CheckIfExceptionIsBP(pCurrentPID, (quint64)exInfo.ExceptionAddress, exInfo.ExceptionCode, true);
						if(bIsBP)
						{
							SetThreadContextHelper(false, true, debug_event.dwThreadId, pCurrentPID);
							pCurrentPID->dwBPRestoreFlag = RESTORE_BP_MEMORY;
							pCurrentPID->bTrapFlag = true;

							BPStruct *pCurrentBP;
							if(m_pBreakpointManager->BreakpointFind((DWORD64)exInfo.ExceptionAddress, MEMORY_BP, debug_event.dwProcessId, true, &pCurrentBP))
							{
								pCurrentBP->bRestoreBP = true;

								memset(tcLogString,0x00,LOGBUFFER);
#ifdef _AMD64_
								swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Memory BP - PID: %06X - %016I64X", debug_event.dwProcessId, pCurrentBP->dwOffset);
#else
								swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Memory BP - PID: %06X - %08X", debug_event.dwProcessId, (DWORD)pCurrentBP->dwOffset);
#endif
								PBLogInfo();

								if(exInfo.ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
								{
									DWORD currentProtection = NULL;
									VirtualProtectEx(pCurrentPID->hProc, (LPVOID)exInfo.ExceptionAddress, pCurrentBP->dwSize, pCurrentBP->dwOldProtection, &currentProtection);
								}

								dwContinueStatus = CallBreakDebugger(&debug_event,0);
								break;							
							}
							else
							{ // PAGE_GUARD on a page where we placed an BP
								MEMORY_BASIC_INFORMATION mbi;

								quint64 pageBase = NULL,
										pageSize = NULL;

								HANDLE processHandle = pCurrentPID->hProc;

								if(VirtualQueryEx(processHandle,(LPVOID)exInfo.ExceptionAddress,&mbi,sizeof(mbi)))
								{
									if((DWORD64)debug_event.u.Exception.ExceptionRecord.ExceptionAddress >= (DWORD64)mbi.BaseAddress && (DWORD64)exInfo.ExceptionAddress <= ((DWORD64)mbi.BaseAddress + mbi.RegionSize))
									{
										pageSize = mbi.RegionSize; 
										pageBase = (DWORD64)mbi.BaseAddress;
									}
								}

								for(int i = 0;i < m_pBreakpointManager->MemoryBPs.size(); i++)
								{
									if(m_pBreakpointManager->MemoryBPs[i].dwOffset <= (pageBase + pageSize) && m_pBreakpointManager->MemoryBPs[i].dwOffset >= pageBase)
									{
										m_pBreakpointManager->MemoryBPs[i].bRestoreBP = true;

										if(exInfo.ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
										{
											DWORD currentProtection = NULL;
											VirtualProtectEx(pCurrentPID->hProc, (LPVOID)exInfo.ExceptionAddress, m_pBreakpointManager->MemoryBPs[i].dwSize, m_pBreakpointManager->MemoryBPs[i].dwOldProtection, &currentProtection);
										}

										break;
									}
								}
							}
						}
						break;
					}
				}

				if(!bIsEP && !bIsKernelBP && !bIsBP)
				{
					bool bExceptionHandler = false;

					PBExceptionInfo((quint64)exInfo.ExceptionAddress, exInfo.ExceptionCode, debug_event.dwProcessId, debug_event.dwThreadId);

					for (size_t i = 0; i < ExceptionHandler.size();i++)
					{
						if(exInfo.ExceptionCode == ExceptionHandler[i].dwExceptionType)
						{
							bExceptionHandler = true;

							if(ExceptionHandler[i].dwHandler != NULL)
							{
								CustomHandler pCustomHandler = (CustomHandler)ExceptionHandler[i].dwHandler;
								dwContinueStatus = pCustomHandler(&debug_event);

								if(ExceptionHandler[i].dwAction == 0)
									dwContinueStatus = CallBreakDebugger(&debug_event, 0);
							}
							else
								dwContinueStatus = CallBreakDebugger(&debug_event,ExceptionHandler[i].dwAction);

							break;
						}
					}

					if(!bExceptionHandler)
					{
						if(dbgSettings.dwDefaultExceptionMode == 1)
							dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
						else if(dbgSettings.bUseExceptionAssist)
						{
							m_continueWithException = 0;
							emit AskForException((DWORD)exInfo.ExceptionCode);
							WaitForSingleObject(m_waitForGUI,INFINITE);

							if(m_continueWithException >= 10)
							{
								m_continueWithException -= 10;
								CustomExceptionAdd((DWORD)exInfo.ExceptionCode, m_continueWithException, NULL);
							}

							dwContinueStatus = CallBreakDebugger(&debug_event,m_continueWithException);
						}
						else
							dwContinueStatus = CallBreakDebugger(&debug_event,0);
					}
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
	switch(dwHandle)
	{
	case 0:
		{
			HANDLE hThread = OpenThread(THREAD_GETSET_CONTEXT,false,debug_event->dwThreadId);
			_dwCurPID = debug_event->dwProcessId;
			_dwCurTID = debug_event->dwThreadId;
			_hCurProc = GetCurrentProcessHandle(debug_event->dwProcessId);
			m_debuggerBreak = true;

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
			m_debuggerBreak = false;

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
		{
			CloseHandle(hProcessSnap);
			return true;
		}
	}while(Process32Next(hProcessSnap,&procEntry32));

	CloseHandle(hProcessSnap);
	return false;
}

bool clsDebugger::CheckIfExceptionIsBP(PIDStruct *pCurrentPID, quint64 dwExceptionOffset,quint64 dwExceptionType, bool bClearTrapFlag, bool isExceptionRelevant)
{
	if(pCurrentPID->bTrapFlag)
	{
		if(bClearTrapFlag)
			pCurrentPID->bTrapFlag = false;
		return true;
	}
	else if((dwExceptionType == EXCEPTION_SINGLE_STEP || dwExceptionType == 0x4000001e) && _bSingleStepFlag)
	{
		return isExceptionRelevant;
	}
	else if(dwExceptionType == EXCEPTION_BREAKPOINT	|| dwExceptionType == 0x4000001f)
	{
		for(int i = 0;i < m_pBreakpointManager->SoftwareBPs.size();i++)
			if(dwExceptionOffset == m_pBreakpointManager->SoftwareBPs[i].dwOffset && (m_pBreakpointManager->SoftwareBPs[i].dwPID == pCurrentPID->dwPID || m_pBreakpointManager->SoftwareBPs[i].dwPID == -1))
				return true;		
	}
	else if(dwExceptionType == EXCEPTION_GUARD_PAGE || dwExceptionType == EXCEPTION_ACCESS_VIOLATION)
	{
		DWORD64 pageBase = NULL,
				pageSize = NULL;

		MEMORY_BASIC_INFORMATION mbi;

		if(VirtualQueryEx(pCurrentPID->hProc,(LPVOID)dwExceptionOffset,&mbi,sizeof(mbi)))
		{
			if(dwExceptionOffset >= (DWORD64)mbi.BaseAddress && dwExceptionOffset <=  ((DWORD64)mbi.BaseAddress + mbi.RegionSize))
			{
				pageSize = mbi.RegionSize; 
				pageBase = (DWORD64)mbi.BaseAddress;
			}
		}

		for(int i = 0;i < m_pBreakpointManager->MemoryBPs.size(); i++)
		{
			if(m_pBreakpointManager->MemoryBPs[i].dwOffset <= (pageBase + pageSize) && m_pBreakpointManager->MemoryBPs[i].dwOffset >= pageBase)
				return true;
		}
	}
	else if(dwExceptionType == 0x4000001E || dwExceptionType == EXCEPTION_SINGLE_STEP)
	{
		for(int i = 0;i < m_pBreakpointManager->HardwareBPs.size();i++)
			if(dwExceptionOffset == m_pBreakpointManager->HardwareBPs[i].dwOffset && (m_pBreakpointManager->HardwareBPs[i].dwPID == pCurrentPID->dwPID || m_pBreakpointManager->HardwareBPs[i].dwPID == -1))
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
			hThread = OpenThread(THREAD_SUSPEND_RESUME ,false,threadEntry32.th32ThreadID);

		if(hThread != INVALID_HANDLE_VALUE)
		{
			if(bSuspend)
				SuspendThread(hThread);
			else
				ResumeThread(hThread);
		}
		
		CloseHandle(hThread);
	}while(Thread32Next(hProcessSnap,&threadEntry32));

	CloseHandle(hProcessSnap);
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

bool clsDebugger::SetThreadContextHelper(bool bDecIP, bool bSetTrapFlag, DWORD dwThreadID, PIDStruct *pCurrentPID)
{
	HANDLE hThread = OpenThread(THREAD_GETSET_CONTEXT, false, dwThreadID);
	if(hThread == INVALID_HANDLE_VALUE) 
		return false;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;

	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(pCurrentPID->hProc, &bIsWOW64);

	if(bIsWOW64)
	{
		WOW64_CONTEXT wowcTT;
		wowcTT.ContextFlags = WOW64_CONTEXT_ALL;
		clsAPIImport::pWow64GetThreadContext(hThread, &wowcTT);

		if(bDecIP)
			wowcTT.Eip--;

		if(bSetTrapFlag)
			wowcTT.EFlags |= 0x100;
		else
			wowcTT.EFlags &= ~0x100;

		clsAPIImport::pWow64SetThreadContext(hThread, &wowcTT);
	}
	else
	{
		CONTEXT cTT;
		cTT.ContextFlags = CONTEXT_ALL;
		GetThreadContext(hThread, &cTT);

		if(bDecIP)
			cTT.Rip--;

		if(bSetTrapFlag)
			cTT.EFlags |= 0x100;
		else
			cTT.EFlags &= ~0x100;

		SetThreadContext(hThread, &cTT);
	}

#else
	CONTEXT cTT;
	cTT.ContextFlags = CONTEXT_ALL;
	GetThreadContext(hThread, &cTT);

	if(bDecIP)
		cTT.Eip--;

	if(bSetTrapFlag)
		cTT.EFlags |= 0x100;
	else
		cTT.EFlags &= ~0x100;

	SetThreadContext(hThread, &cTT);
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
	return m_dbgPI.hProcess;
}

HANDLE clsDebugger::GetProcessHandleByPID(DWORD PID)
{
	if(pThis != NULL)
		return pThis->GetCurrentProcessHandle(PID);

	return NULL;
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

void clsDebugger::SetNewThreadContext(bool isWow64, CONTEXT newProcessContext, WOW64_CONTEXT newWowProcessContext)
{
	if(isWow64)
		pThis->wowProcessContext = newWowProcessContext;
	else
		pThis->ProcessContext = newProcessContext;
}

PIDStruct* clsDebugger::GetCurrentPIDDataPointer(DWORD processID)
{
	PIDStruct *pCurrentPID = NULL;
	size_t countPID = PIDs.size();

	for(size_t i = 0;i < countPID; i++)
	{
		pCurrentPID = &PIDs[i];

		if(pCurrentPID->dwPID == processID)
		{
			return pCurrentPID;
		}
	}

	return pCurrentPID; // should never happen
}