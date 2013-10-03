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
#include "qtDLGTrace.h"

#include "clsDebugger.h"
#include "clsMemManager.h"
#include "clsAPIImport.h"
#include "clsHelperClass.h"

#include <dbghelp.h>

using namespace std;

bool clsDebugger::DetachFromProcess()
{
	m_normalDebugging = true;
	//_isDebugging = false;
	m_stopDebugging = true;

	m_pBreakpointManager->BreakpointClear();

	for(size_t d = 0;d < PIDs.size();d++)
	{
		if(!CheckProcessState(PIDs[d].dwPID))
			break;
		DebugBreakProcess(PIDs[d].hProc);
		//DebugActiveProcessStop(PIDs[d].dwPID);
		PulseEvent(m_debugEvent);
	}

	//emit OnDebuggerTerminated();
	return true;
}

bool clsDebugger::AttachToProcess(DWORD dwPID)
{
	CleanWorkSpace();
	m_normalDebugging = false;m_attachPID = dwPID;
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
				emit OnLog(QString("[!] %1 Debugging suspended!").arg(dwPID, 6, 16, QChar('0')));

				return true;
			}
		}
		else// if(dbgSettings.dwSuspendType == 0x1)
		{
			if(SuspendProcess(dwPID,true))
			{
				emit OnLog(QString("[!] %1 Debugging suspended!").arg(dwPID, 6, 16, QChar('0')));

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
	return PulseEvent(m_debugEvent);
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
	return PulseEvent(m_debugEvent);
}

bool clsDebugger::GetDebuggingState()
{
	if(m_isDebugging == true)
		return true;
	else
		return false;
}

bool clsDebugger::IsTargetSet()
{
	if(m_targetFile.length() > 0)
		return true;
	return false;
}

bool clsDebugger::StepOver(quint64 dwNewOffset)
{
	if(!m_pBreakpointManager->BreakpointAdd(SOFTWARE_BP, NULL, m_currentPID, dwNewOffset, 1, BP_STEPOVER))
		return false;

	PulseEvent(m_debugEvent);
	return true;
}

bool clsDebugger::StepIn()
{
	m_singleStepFlag = true;

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

	return PulseEvent(m_debugEvent);
}

void clsDebugger::ClearTarget()
{
	m_targetFile.clear();
}

void clsDebugger::SetTarget(QString sTarget)
{
	m_targetFile = sTarget;
	m_normalDebugging = true;
}

DWORD clsDebugger::GetCurrentPID()
{
	if(pThis->IsDebuggerSuspended())
		return pThis->m_currentPID;
	else
		return pThis->GetMainProcessID();
}

DWORD clsDebugger::GetCurrentTID()
{
	if(pThis->IsDebuggerSuspended())
		return pThis->m_currentTID;
	else
		return pThis->GetMainThreadID();
}

void clsDebugger::SetCommandLine(QString CommandLine)
{
	m_commandLine = CommandLine;
}

void clsDebugger::ClearCommandLine()
{
	m_commandLine.clear();
}

HANDLE clsDebugger::GetCurrentProcessHandle()
{
	if(IsDebuggerSuspended())
		return m_currentProcess;
	else
		return GetProcessHandleByPID(-1);
}

QString clsDebugger::GetCMDLine()
{
	return m_commandLine;
}

QString clsDebugger::GetTarget()
{
	return m_targetFile;
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