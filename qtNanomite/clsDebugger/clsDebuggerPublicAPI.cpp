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
#include "clsMemManager.h"
#include "clsDBManager.h"
#include "clsAPIImport.h"
#include "clsHelperClass.h"

#include <dbghelp.h>

using namespace std;

bool clsDebugger::DetachFromProcess()
{
	_NormalDebugging = true;
	//_isDebugging = false;
	_bStopDebugging = true;

	m_pBreakpointManager->BreakpointClear();

	for(size_t d = 0;d < PIDs.size();d++)
	{
		if(!CheckProcessState(PIDs[d].dwPID))
			break;
		DebugBreakProcess(PIDs[d].hProc);
		//DebugActiveProcessStop(PIDs[d].dwPID);
		PulseEvent(_hDbgEvent);
	}

	//emit OnDebuggerTerminated();
	return true;
}

bool clsDebugger::AttachToProcess(DWORD dwPID)
{
	CleanWorkSpace();
	_NormalDebugging = false;_dwPidToAttach = dwPID;
	return true;
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
	for(size_t i = 0;i < PIDs.size();i++)
		StopDebugging(PIDs[i].dwPID);
	return PulseEvent(_hDbgEvent);
}

bool clsDebugger::StopDebugging(DWORD dwPID)
{
	HANDLE hProcess = GetCurrentProcessHandle(dwPID);

	if(CheckProcessState(dwPID))
	{
		if(TerminateProcess(hProcess,0))
		{
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

bool clsDebugger::GetDebuggingState()
{
	if(_isDebugging == true)
		return true;
	else
		return false;
}

bool clsDebugger::IsTargetSet()
{
	if(_sTarget.length() > 0)
		return true;
	return false;
}

bool clsDebugger::StepOver(quint64 dwNewOffset)
{
	if(!m_pBreakpointManager->BreakpointAdd(SOFTWARE_BP, NULL, _dwCurPID, dwNewOffset, 1, BP_STEPOVER))
		return false;

	PulseEvent(_hDbgEvent);
	return true;
}

bool clsDebugger::StepIn()
{
	_bSingleStepFlag = true;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(GetCurrentProcessHandle(),&bIsWOW64);
	
	if(bIsWOW64)
		wowProcessContext.EFlags |= 0x100;
	else
		ProcessContext.EFlags |= 0x100;
#else
	ProcessContext.EFlags |= 0x100;
#endif

	return PulseEvent(_hDbgEvent);
}

void clsDebugger::ClearTarget()
{
	_sTarget.clear();
}

void clsDebugger::SetTarget(wstring sTarget)
{
	_sTarget = sTarget;
	_NormalDebugging = true;
}

DWORD clsDebugger::GetCurrentPID()
{
	if(pThis->IsDebuggerSuspended())
		return pThis->_dwCurPID;
	else
		return pThis->GetMainProcessID();
}

DWORD clsDebugger::GetCurrentTID()
{
	if(pThis->IsDebuggerSuspended())
		return pThis->_dwCurTID;
	else
		return pThis->GetMainThreadID();
}

void clsDebugger::SetCommandLine(std::wstring CommandLine)
{
	_sCommandLine = CommandLine;
}

void clsDebugger::ClearCommandLine()
{
	_sCommandLine.clear();
}

HANDLE clsDebugger::GetCurrentProcessHandle()
{
	if(IsDebuggerSuspended())
		return _hCurProc;
	else
		return GetProcessHandleByPID(-1);
}

wstring clsDebugger::GetCMDLine()
{
	return _sCommandLine;
}

wstring clsDebugger::GetTarget()
{
	return _sTarget;
}

bool clsDebugger::SetTraceFlagForPID(DWORD dwPID,bool bIsEnabled)
{
	for(int i = 0; i < PIDs.size(); i++)
	{
		if(PIDs[i].dwPID == dwPID)
		{
			PIDs[i].bTraceFlag = bIsEnabled;
			if(bIsEnabled)
			{
				qtDLGTrace::enableStatusBarTimer();
				return StepIn();
			}
			else
			{	
				qtDLGTrace::disableStatusBarTimer();
				return true;
			}
		}
	}
	return false;
}

bool clsDebugger::IsDebuggerSuspended()
{
	return m_debuggerBreak;
}

DWORD clsDebugger::GetMainProcessID()
{
	return m_dbgPI.dwProcessId;
}

DWORD clsDebugger::GetMainThreadID()
{
	return m_dbgPI.dwThreadId;
}