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
#include "clsBreakpointManager.h"

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
	StartDebugging();
}

bool clsDebugger::StartDebugging()
{
	_bStopDebugging = false;
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

				size_t iPid = 0;
				for(size_t i = 0;i < PIDs.size();i++)
				{
					if(PIDs[i].dwPID == debug_event.dwProcessId)
					{
						iPid = i;
						break;
					}
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

				m_pBreakpointManager->BreakpointAdd(SOFTWARE_BP,NULL,debug_event.dwProcessId,(quint64)debug_event.u.CreateProcessInfo.lpStartAddress,BP_DONOTKEEP);
				
				if(dbgSettings.bBreakOnTLS)
				{
					QList<quint64> tlsCallback = clsPEManager::getTLSCallbackOffset((wstring)tcDllFilepath,debug_event.dwProcessId);
					if(tlsCallback.length() > 0)
					{
						for(int i = 0; i < tlsCallback.count(); i++)
						{
							m_pBreakpointManager->BreakpointAdd(SOFTWARE_BP,NULL,debug_event.dwProcessId,(quint64)debug_event.u.CreateProcessInfo.lpBaseOfImage + tlsCallback.at(i),BP_DONOTKEEP);
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
				HANDLE hProc = 0;
				size_t iPid = 0;

				for(size_t i = 0;i < PIDs.size();i++)
				{
					if(PIDs[i].dwPID == debug_event.dwProcessId)
					{
						hProc = PIDs[i].hProc;
						iPid = i;
						break;
					}
				}

				PTCHAR sDLLFileName = GetFileNameFromModuleBase(hProc, debug_event.u.LoadDll.lpBaseOfDll); 
				PBDLLInfo(sDLLFileName,debug_event.dwProcessId,(quint64)debug_event.u.LoadDll.lpBaseOfDll,true);

				if(PIDs[iPid].bSymLoad && dbgSettings.bAutoLoadSymbols)
					SymLoadModuleExW(hProc,NULL,sDLLFileName,0,(quint64)debug_event.u.LoadDll.lpBaseOfDll,0,0,0);
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
			for(size_t i = 0;i < DLLs.size(); i++)
			{
				if(DLLs[i].dwBaseAdr == (quint64)debug_event.u.UnloadDll.lpBaseOfDll && DLLs[i].dwPID == debug_event.dwProcessId)
				{
					PBDLLInfo(DLLs[i].sPath, DLLs[i].dwPID, DLLs[i].dwBaseAdr, false, i);
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
				{
					if(PIDs[i].dwPID == debug_event.dwProcessId)
					{
						iPid = i;
						break;
					}
				}

				switch (exInfo.ExceptionRecord.ExceptionCode)
				{
				case 0x4000001f: // Breakpoint in x86 Process which got executed in a x64 environment
					if(PIDs[iPid].bKernelBP && !PIDs[iPid].bWOW64KernelBP)
					{
						memset(tcLogString,0x00,LOGBUFFER);
#ifdef _AMD64_
						swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] WOW64 Kernel EP - PID %06X - %016I64X", debug_event.dwProcessId, (quint64)exInfo.ExceptionRecord.ExceptionAddress);
#else
						swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] WOW64 Kernel EP - PID %06X - %08X", debug_event.dwProcessId, (DWORD)exInfo.ExceptionRecord.ExceptionAddress);
#endif
						PBLogInfo();
							
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
							memset(tcLogString,0x00,LOGBUFFER);
	#ifdef _AMD64_
							swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Kernel EP - PID %06X - %016I64X", debug_event.dwProcessId, (quint64)exInfo.ExceptionRecord.ExceptionAddress);
	#else
							swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Kernel EP - PID %06X - %08X", debug_event.dwProcessId, (DWORD)exInfo.ExceptionRecord.ExceptionAddress);
	#endif
							PBLogInfo();
								
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
								
								m_pBreakpointManager->BreakpointUpdateOffsets();
								emit UpdateOffsetsPatches(PIDs[iPid].hProc,PIDs[iPid].dwPID);

								m_pBreakpointManager->BreakpointInit(debug_event.dwProcessId);
							}

							bIsBP = CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,EXCEPTION_BREAKPOINT,debug_event.dwProcessId,true);
							if(bIsBP)
							{
								HANDLE hProc = PIDs[iPid].hProc;
								bool bpNeedsReplace = false;

								BPStruct *pCurrentBP;
								if(m_pBreakpointManager->BreakpointFind((DWORD64)exInfo.ExceptionRecord.ExceptionAddress, SOFTWARE_BP, debug_event.dwProcessId, true, &pCurrentBP))
								{
									bool b = WriteProcessMemory(hProc, (LPVOID)pCurrentBP->dwOffset, (LPVOID)&pCurrentBP->bOrgByte, pCurrentBP->dwSize,NULL);
									bool c = FlushInstructionCache(hProc, (LPVOID)pCurrentBP->dwOffset, pCurrentBP->dwSize);

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
									SetThreadContextHelper(true,true,debug_event.dwThreadId,debug_event.dwProcessId);
									PIDs[iPid].bTrapFlag = true;
									PIDs[iPid].dwBPRestoreFlag = RESTORE_BP_SOFTWARE;
								}
								else
									SetThreadContextHelper(true,false,debug_event.dwThreadId,debug_event.dwProcessId);

								if(bIsEP && !dbgSettings.bBreakOnModuleEP)
									dwContinueStatus = CallBreakDebugger(&debug_event,2);
								else
								{
									if(!bStepOver)
									{
										memset(tcLogString,0x00,LOGBUFFER);
										if(bIsEP)
#ifdef _AMD64_
											swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Entrypoint - PID %06X - %016I64X", debug_event.dwProcessId, (quint64)exInfo.ExceptionRecord.ExceptionAddress);
										else
											swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Software BP - PID %06X - %016I64X", debug_event.dwProcessId, (quint64)exInfo.ExceptionRecord.ExceptionAddress);
#else
											swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Entrypoint - PID %06X - %08X", debug_event.dwProcessId, (DWORD)exInfo.ExceptionRecord.ExceptionAddress);
										else
											swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Software BP - PID %06X - %08X", debug_event.dwProcessId, (DWORD)exInfo.ExceptionRecord.ExceptionAddress);
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
						if(PIDs[iPid].bTraceFlag && _bSingleStepFlag)
						{
							bIsBP = true;
							SetThreadContextHelper(false,true,debug_event.dwThreadId,debug_event.dwProcessId);
							qtDLGTrace::addTraceData((quint64)debug_event.u.Exception.ExceptionRecord.ExceptionAddress,debug_event.dwProcessId,debug_event.dwThreadId);
							break;
						}
						else if(_bSingleStepFlag)
						{
							_bSingleStepFlag = false;
							bIsBP = true;

							dwContinueStatus = CallBreakDebugger(&debug_event,0);
							break;
						}

						bIsBP = CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,EXCEPTION_SINGLE_STEP,debug_event.dwProcessId,true,false);
						if(bIsBP)
						{
							if(PIDs[iPid].dwBPRestoreFlag == RESTORE_BP_SOFTWARE) // Restore SoftwareBP
							{
								for(int i = 0;i < m_pBreakpointManager->SoftwareBPs.size(); i++)
								{
									if(m_pBreakpointManager->SoftwareBPs[i].bRestoreBP && 
										m_pBreakpointManager->SoftwareBPs[i].dwHandle == BP_KEEP && 
										(m_pBreakpointManager->SoftwareBPs[i].dwPID == debug_event.dwProcessId || m_pBreakpointManager->SoftwareBPs[i].dwPID == -1))
									{
										clsBreakpointSoftware::wSoftwareBP(m_pBreakpointManager->SoftwareBPs[i].dwPID,m_pBreakpointManager->SoftwareBPs[i].dwOffset,m_pBreakpointManager->SoftwareBPs[i].dwSize,m_pBreakpointManager->SoftwareBPs[i].bOrgByte);
										m_pBreakpointManager->SoftwareBPs[i].bRestoreBP = false;

										break;
									}									
								}

								PIDs[iPid].bTrapFlag = false;
								PIDs[iPid].dwBPRestoreFlag = RESTORE_NON;
							}
							else if(PIDs[iPid].dwBPRestoreFlag == RESTORE_BP_MEMORY) // Restore MemBP
							{
								for(int i = 0;i < m_pBreakpointManager->MemoryBPs.size(); i++)
								{
									if(m_pBreakpointManager->MemoryBPs[i].bRestoreBP &&
										m_pBreakpointManager->MemoryBPs[i].dwHandle == BP_KEEP &&
										(m_pBreakpointManager->MemoryBPs[i].dwPID == debug_event.dwProcessId || m_pBreakpointManager->MemoryBPs[i].dwPID == -1))
									{
										clsBreakpointMemory::wMemoryBP(m_pBreakpointManager->MemoryBPs[i].dwPID,m_pBreakpointManager->MemoryBPs[i].dwOffset,m_pBreakpointManager->MemoryBPs[i].dwSize,m_pBreakpointManager->MemoryBPs[i].dwTypeFlag,&m_pBreakpointManager->MemoryBPs[i].dwOldProtection);
										m_pBreakpointManager->MemoryBPs[i].bRestoreBP = false;

										break;
									}									
								}

								PIDs[iPid].bTrapFlag = false;
								PIDs[iPid].dwBPRestoreFlag = RESTORE_NON;
							}
							else if(PIDs[iPid].dwBPRestoreFlag == RESTORE_BP_HARDWARE) // Restore HwBp
							{
								for(int i = 0;i < m_pBreakpointManager->HardwareBPs.size();i++)
								{
									if(m_pBreakpointManager->HardwareBPs[i].bRestoreBP &&
										m_pBreakpointManager->HardwareBPs[i].dwHandle == BP_KEEP &&
										(m_pBreakpointManager->HardwareBPs[i].dwPID == debug_event.dwProcessId || m_pBreakpointManager->HardwareBPs[i].dwPID == -1))
									{
										clsBreakpointHardware::wHardwareBP(debug_event.dwProcessId,m_pBreakpointManager->HardwareBPs[i].dwOffset,m_pBreakpointManager->HardwareBPs[i].dwSize,m_pBreakpointManager->HardwareBPs[i].dwSlot,m_pBreakpointManager->HardwareBPs[i].dwTypeFlag);
										m_pBreakpointManager->HardwareBPs[i].bRestoreBP = false;

										break;
									}
								}

								PIDs[iPid].bTrapFlag = false;
								PIDs[iPid].dwBPRestoreFlag = RESTORE_NON;
							}
							else if(PIDs[iPid].dwBPRestoreFlag == RESTORE_NON) // First time hit HwBP
							{
								BPStruct *pCurrentBP;
								if(m_pBreakpointManager->BreakpointFind((DWORD64)exInfo.ExceptionRecord.ExceptionAddress, HARDWARE_BP, debug_event.dwProcessId, true, &pCurrentBP))
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

									PIDs[iPid].dwBPRestoreFlag = RESTORE_BP_HARDWARE;
									PIDs[iPid].bTrapFlag = true;

									SetThreadContextHelper(false, true, debug_event.dwThreadId, debug_event.dwProcessId);
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
						bIsBP = CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,exInfo.ExceptionRecord.ExceptionCode,debug_event.dwProcessId,true);
						if(bIsBP)
						{
							SetThreadContextHelper(false,true,debug_event.dwThreadId,debug_event.dwProcessId);
							PIDs[iPid].dwBPRestoreFlag = RESTORE_BP_MEMORY;
							PIDs[iPid].bTrapFlag = true;

							BPStruct *pCurrentBP;
							if(m_pBreakpointManager->BreakpointFind((DWORD64)exInfo.ExceptionRecord.ExceptionAddress, MEMORY_BP, debug_event.dwProcessId, true, &pCurrentBP))
							{
								pCurrentBP->bRestoreBP = true;

								memset(tcLogString,0x00,LOGBUFFER);
#ifdef _AMD64_
								swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Memory BP - PID: %06X - %016I64X", debug_event.dwProcessId, pCurrentBP->dwOffset);
#else
								swprintf_s(tcLogString,LOGBUFFERCHAR,L"[!] Break on - Memory BP - PID: %06X - %08X", debug_event.dwProcessId, (DWORD)pCurrentBP->dwOffset);
#endif
								PBLogInfo();

								if(exInfo.ExceptionRecord.ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
								{
									DWORD currentProtection = NULL;
									VirtualProtectEx(PIDs[iPid].hProc, (LPVOID)exInfo.ExceptionRecord.ExceptionAddress, pCurrentBP->dwSize, pCurrentBP->dwOldProtection, &currentProtection);
								}

								dwContinueStatus = CallBreakDebugger(&debug_event,0);
								break;							
							}
							else
							{ // PAGE_GUARD on a page where we placed an BP
								MEMORY_BASIC_INFORMATION mbi;

								quint64 pageBase = NULL,
										pageSize = NULL;

								HANDLE processHandle = PIDs[iPid].hProc;

								if(VirtualQueryEx(processHandle,(LPVOID)exInfo.ExceptionRecord.ExceptionAddress,&mbi,sizeof(mbi)))
								{
									if((DWORD64)debug_event.u.Exception.ExceptionRecord.ExceptionAddress >= (DWORD64)mbi.BaseAddress && (DWORD64)debug_event.u.Exception.ExceptionRecord.ExceptionAddress <=  ((DWORD64)mbi.BaseAddress + mbi.RegionSize))
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

										if(exInfo.ExceptionRecord.ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
										{
											DWORD currentProtection = NULL;
											VirtualProtectEx(PIDs[iPid].hProc, (LPVOID)exInfo.ExceptionRecord.ExceptionAddress, m_pBreakpointManager->MemoryBPs[i].dwSize, m_pBreakpointManager->MemoryBPs[i].dwOldProtection, &currentProtection);
										}

										break;
									}
								}
							}
						}
						break;
					}
				}

				bool bExceptionHandler = false;

				if(!bIsEP && !bIsKernelBP && !bIsBP)
				{
					if(!CheckIfExceptionIsBP((quint64)exInfo.ExceptionRecord.ExceptionAddress,exInfo.ExceptionRecord.ExceptionCode,debug_event.dwProcessId,false))
						PBExceptionInfo((quint64)exInfo.ExceptionRecord.ExceptionAddress,exInfo.ExceptionRecord.ExceptionCode,debug_event.dwProcessId,debug_event.dwThreadId);

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
						dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
					else if(dbgSettings.bUseExceptionAssist)
					{
						m_continueWithException = 0;
						emit AskForException((DWORD)exInfo.ExceptionRecord.ExceptionCode);
						WaitForSingleObject(m_waitForGUI,INFINITE);

						if(m_continueWithException >= 10)
						{
							m_continueWithException -= 10;
							CustomExceptionAdd((DWORD)exInfo.ExceptionRecord.ExceptionCode,m_continueWithException,NULL);
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

bool clsDebugger::CheckIfExceptionIsBP(quint64 dwExceptionOffset,quint64 dwExceptionType,DWORD dwPID,bool bClearTrapFlag, bool isExceptionRelevant)
{
	size_t iPID = NULL;
	for(size_t i = 0;i < PIDs.size();i++)
	{
		if(PIDs[i].dwPID == dwPID)
		{
			iPID = i;
			break;
		}
	}

	if(PIDs[iPID].bTrapFlag)
	{
		if(bClearTrapFlag)
			PIDs[iPID].bTrapFlag = false;
		return true;
	}
	else if((dwExceptionType == EXCEPTION_SINGLE_STEP || dwExceptionType == 0x4000001e) && _bSingleStepFlag)
	{
		return isExceptionRelevant;
	}
	else if(dwExceptionType == EXCEPTION_BREAKPOINT	|| dwExceptionType == 0x4000001f)
	{
		for(int i = 0;i < m_pBreakpointManager->SoftwareBPs.size();i++)
			if(dwExceptionOffset == m_pBreakpointManager->SoftwareBPs[i].dwOffset && (m_pBreakpointManager->SoftwareBPs[i].dwPID == dwPID || m_pBreakpointManager->SoftwareBPs[i].dwPID == -1))
				return true;		
	}
	else if(dwExceptionType == EXCEPTION_GUARD_PAGE || dwExceptionType == EXCEPTION_ACCESS_VIOLATION)
	{
		DWORD64 pageBase = NULL,
				pageSize = NULL;

		MEMORY_BASIC_INFORMATION mbi;

		if(VirtualQueryEx(PIDs[iPID].hProc,(LPVOID)dwExceptionOffset,&mbi,sizeof(mbi)))
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
			if(dwExceptionOffset == m_pBreakpointManager->HardwareBPs[i].dwOffset && (m_pBreakpointManager->HardwareBPs[i].dwPID == dwPID || m_pBreakpointManager->HardwareBPs[i].dwPID == -1))
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

bool clsDebugger::SetThreadContextHelper(bool bDecIP,bool bSetTrapFlag, DWORD dwThreadID, DWORD dwPID)
{
	HANDLE hThread = OpenThread(THREAD_GETSET_CONTEXT,false,dwThreadID);
	if(hThread == INVALID_HANDLE_VALUE) 
		return false;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;

	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(GetProcessHandleByPID(dwPID),&bIsWOW64);

	if(bIsWOW64)
	{
		WOW64_CONTEXT wowcTT;
		wowcTT.ContextFlags = WOW64_CONTEXT_ALL;
		clsAPIImport::pWow64GetThreadContext(hThread,&wowcTT);

		if(bDecIP)
			wowcTT.Eip--;

		if(bSetTrapFlag)
			wowcTT.EFlags |= 0x100;
		else
			wowcTT.EFlags &= ~0x100;

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
		else
			cTT.EFlags &= ~0x100;

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
