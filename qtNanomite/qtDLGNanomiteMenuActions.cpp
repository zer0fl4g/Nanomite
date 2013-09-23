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
#include "qtDLGNanomite.h"
#include "qtDLGAbout.h"
#include "qtDLGAttach.h"
#include "qtDLGOption.h"
#include "qtDLGMemoryView.h"
#include "qtDLGHeapView.h"
#include "qtDLGStringView.h"
#include "qtDLGHandleView.h"
#include "qtDLGWindowView.h"
#include "qtDLGHexView.h"
#include "qtDLGRegEdit.h"
#include "qtDLGPEEditor.h"
#include "qtDLGFunctions.h"
#include "qtDLGProcessPrivilege.h"
#include "qtDLGOpenNewFile.h"

#include "clsHelperClass.h"
#include "clsDisassembler.h"
#include "clsAPIImport.h"
#include "clsMemManager.h"

#include "..\NanomiteUpdater\UpdaterWidget\uupdatewidget.h"

#include <TlHelp32.h>
#include <Psapi.h>

using namespace std;

void qtDLGNanomite::action_FileTerminateGUI()
{
	if(coreDebugger->GetDebuggingState())
		coreDebugger->StopDebuggingAll();
	close();
}

void qtDLGNanomite::action_FileOpenNewFile()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_IsRestart = true;
		action_DebugStop();
		coreBPManager->BreakpointClear();
		coreDebugger->ClearTarget();
		coreDebugger->ClearCommandLine();
	}
	else
	{
		coreBPManager->BreakpointClear();
		coreDebugger->ClearTarget();
		coreDebugger->ClearCommandLine();
		action_DebugStart();
	}
}

void qtDLGNanomite::action_FileAttachTo()
{
	if(!coreDebugger->GetDebuggingState())
	{
		qtDLGAttach newAttaching(this,Qt::Window);
		connect(&newAttaching,SIGNAL(StartAttachedDebug(int,QString)),this,SLOT(action_DebugAttachStart(int,QString)));
		newAttaching.exec();
	}
}

void qtDLGNanomite::action_FileDetach()
{
	if(coreDebugger->GetDebuggingState())
	{
		if(!coreDebugger->DetachFromProcess())
			MessageBox(NULL,L"Failed to detach from Process!",L"Nanomite",MB_OK);
		else
			UpdateStateBar(STATE_TERMINATE);
	}
}

void qtDLGNanomite::action_DebugStart()
{
	if(!coreDebugger->GetDebuggingState())
	{
		CleanGUI();

		if(!coreDebugger->IsTargetSet())
		{
			qtDLGOpenNewFile newFile(this);
			QString fileName, commandLine;

			newFile.exec();
			newFile.GetFilePathAndCommandLine(fileName, commandLine);

			if(fileName.length() > 0 && commandLine.length() > 0)
			{
				coreDebugger->SetTarget(fileName.toStdWString());
				coreDebugger->SetCommandLine(commandLine.toStdWString());
			}
			else if(fileName.length() > 0 && commandLine.length() <= 0)
			{
				coreDebugger->SetTarget(fileName.toStdWString());
				coreDebugger->ClearCommandLine();		
			}
			else
				return;

			qtDLGPatchManager::ClearAllPatches();
		}

		if(!PEManager->OpenFile(coreDebugger->GetTarget()) || 
#ifdef _AMD64_
			!PEManager->isValidPEFile(coreDebugger->GetTarget()))
#else
			!PEManager->isValidPEFile(coreDebugger->GetTarget()) ||
			PEManager->is64BitFile(coreDebugger->GetTarget()))
#endif
		{
			QMessageBox::critical(this,"Nanomite","This is a invalid File! Please select another one!",QMessageBox::Ok,QMessageBox::Ok);

			PEManager->CloseFile(coreDebugger->GetTarget(),0);
			coreDebugger->ClearTarget();
			return;
		}
		
		InsertRecentDebuggedFile(QString::fromStdWString(coreDebugger->GetTarget()));
		coreDebugger->start();

		UpdateStateBar(STATE_RUN);
	}
	else
	{
		qtDLGTrace::clearTraceData();
		coreDebugger->ResumeDebugging();
		UpdateStateBar(STATE_RUN);
	}
}

void qtDLGNanomite::action_DebugAttachStart(int iPID,QString FilePath)
{
	if(!coreDebugger->GetDebuggingState())
	{
		CleanGUI();

		wstring filePath = FilePath.toStdWString();

		PEManager->OpenFile(filePath);
		if(!PEManager->isValidPEFile(filePath))
		{
			QMessageBox::critical(this,"Nanomite","This is a invalid File! Please select another one!",QMessageBox::Ok,QMessageBox::Ok);

			PEManager->CloseFile(coreDebugger->GetTarget(),0);
			return;
		}

		qtDLGPatchManager::ClearAllPatches();

		InsertRecentDebuggedFile(FilePath);
		coreDebugger->SetTarget(filePath);
		coreDebugger->AttachToProcess(iPID);
		coreDebugger->start();

		UpdateStateBar(STATE_RUN);
	}
}

void qtDLGNanomite::action_DebugStop()
{
	if(coreDebugger->GetDebuggingState())
		coreDebugger->StopDebuggingAll();
}

void qtDLGNanomite::action_DebugRestart()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_IsRestart = true;
		action_DebugStop();
	}
	else
		action_DebugStart();
}

void qtDLGNanomite::action_DebugSuspend()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_iMenuProcessID = -1;
		actionDebug_Suspend->setDisabled(true);

		GenerateMenu();

		if(m_iMenuProcessID == 0)
		{
			coreDebugger->SuspendDebuggingAll();
			UpdateStateBar(STATE_SUSPEND);
		}
		else
		{
			coreDebugger->SuspendDebugging(m_iMenuProcessID);
			UpdateStateBar(STATE_SUSPEND);
		}

		actionDebug_Suspend->setEnabled(true);
		m_iMenuProcessID = -1;
	}
}

void qtDLGNanomite::action_DebugStepIn()
{
	if(coreDebugger->GetDebuggingState())
	{
		qtDLGTrace::clearTraceData();
		coreDebugger->StepIn();
	}
}

void qtDLGNanomite::action_DebugStepOver()
{
	if(!coreDebugger->GetDebuggingState()) return;
	qtDLGTrace::clearTraceData();

	DWORD eFlags = NULL;
	quint64 dwEIP = NULL;
#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64)
	{
		dwEIP = coreDebugger->wowProcessContext.Eip;
		eFlags = coreDebugger->wowProcessContext.EFlags;
	}
	else
	{
		dwEIP = coreDebugger->ProcessContext.Rip;
		eFlags = coreDebugger->ProcessContext.EFlags;
	}
#else
	dwEIP = coreDebugger->ProcessContext.Eip;
	eFlags = coreDebugger->ProcessContext.EFlags;
#endif

	QMap<QString,DisAsDataRow>::const_iterator i = coreDisAs->SectionDisAs.constFind(QString("%1").arg(dwEIP,16,16,QChar('0')).toUpper());
	if(i == coreDisAs->SectionDisAs.constEnd())
		return;

	if(i.value().ASM.contains("ret") || i.value().ASM.contains("retn") || i.value().ASM.contains("retf"))
	{
		action_DebugStepIn();
		return;
	}

	bool bOF = (eFlags & 0x800) ? true : false;
	bool bDF = (eFlags & 0x400) ? true : false;
	bool bTF = (eFlags & 0x100) ? true : false;
	bool bSF = (eFlags & 0x80) ? true : false;
	bool bZF = (eFlags & 0x40) ? true : false;
	bool bAF = (eFlags & 0x10) ? true : false;
	bool bPF = (eFlags & 0x4) ? true : false;
	bool bCF = (eFlags & 0x1) ? true : false;

	// based on http://download.intel.com/products/processor/manual/325462.pdf
		// jmp - always
	if((i.value().ASM.contains("jmp"))																				||
		// of = 1, jo
		(i.value().ASM.contains("jo") && bOF)																		||
		// of = 0, jno
		(i.value().ASM.contains("jno") && !bOF)																		||
		// sf = 1, js
		((i.value().ASM.contains("js") && bSF))																		||
		// sf = 0, jns
		((i.value().ASM.contains("jns") && !bSF))																	||
		// zf = 1, jz - je
		((i.value().ASM.contains("jz") || i.value().ASM.contains("je")) && bZF)										||
		// pf = 1, jp - jpe
		((i.value().ASM.contains("jp") || i.value().ASM.contains("jpe")) && bPF)									||
		// pf = 0, jnp - jpo 
		((i.value().ASM.contains("jnp") || i.value().ASM.contains("jpo")) && !bPF)									||
		// zf = 0, jnz - jne
		((i.value().ASM.contains("jnz") || i.value().ASM.contains("jne")) && !bZF)									||
		// sf != of, jl - jnge
		((i.value().ASM.contains("jl") || i.value().ASM.contains("jnge")) && (bSF != bOF))							||
		// sf == of, jnl - jge
		((i.value().ASM.contains("jnl") || i.value().ASM.contains("jge")) && (bSF == bOF))							||
		// cf = 1 + zf = 1, jbe - jna
		((i.value().ASM.contains("jbe") || i.value().ASM.contains("jna")) && (bZF && bCF))							||
		// cf = 0 + zf = 0, jnbe - ja
		((i.value().ASM.contains("jnbe") || i.value().ASM.contains("ja")) && (!bZF && !bCF))						||
		// zf = 1 || sf != of, jle - jng
		((i.value().ASM.contains("jle") || i.value().ASM.contains("jng")) && (bZF && bSF != bOF))					||
		// zf = 0 && sf == of, jnle - jg
		((i.value().ASM.contains("jnle") || i.value().ASM.contains("jg")) && (!bZF && bSF == bOF))					||
		// cf = 1, jb - jc - jnae
		((i.value().ASM.contains("jb") || i.value().ASM.contains("jc") || i.value().ASM.contains("jnae")) && bCF)	||
		// cf = 0, jnb - jae - jnc
		((i.value().ASM.contains("jnb") || i.value().ASM.contains("jnc") || i.value().ASM.contains("jae")) && !bCF))
	{
		// jump conditions are set so lets step over jump
		if(i.value().ASM.contains("ptr"))
		{
			quint64 dwSelectedVA = NULL;
			QStringList splittedSelectedString = i.value().ASM.split(" ");
			
			if(splittedSelectedString[1].compare("qword") == 0)
			{
				ReadProcessMemory(coreDebugger->GetCurrentProcessHandle(),
					(LPVOID)splittedSelectedString[3].replace("h","").replace("[","").replace("]","").toULongLong(0,16),
					(LPVOID)&dwSelectedVA, sizeof(DWORD64),NULL);
			}
			else if(splittedSelectedString[1].compare("dword") == 0)
			{
				ReadProcessMemory(coreDebugger->GetCurrentProcessHandle(),
					(LPVOID)splittedSelectedString[3].replace("h","").replace("[","").replace("]","").toULongLong(0,16),
					(LPVOID)&dwSelectedVA, sizeof(DWORD),NULL);
			}

			coreDebugger->StepOver(dwSelectedVA);	
		}
		else
			coreDebugger->StepOver(i.value().ASM.split(" ")[1].replace("h","").toULongLong(0,16));
	}
	else
	{
		// normal step over
		++i;
		if((QMapData::Node *)i == (QMapData::Node *)coreDisAs->SectionDisAs.constEnd())
			coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwEIP);

		coreDebugger->StepOver(i.value().Offset.toULongLong(0,16));
	}
}

void qtDLGNanomite::action_DebugStepOut()
{
	if(!coreDebugger->GetDebuggingState()) return;
	qtDLGTrace::clearTraceData();

	coreDebugger->StepOver(callstackView->tblCallstack->item(0,3)->text().toULongLong(0,16));
}

void qtDLGNanomite::action_DebugRunToUserCode()
{
	if(!coreDebugger->GetDebuggingState()) return;
	qtDLGTrace::clearTraceData();

	DWORD64 ModuleBase = NULL,
			ModuleSize = NULL,
			CurAddress = NULL;
	HANDLE	hProcess = coreDebugger->GetCurrentProcessHandle();
	PTCHAR	lpFileName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR)),
			lpCurrentNameTemp = NULL,
			lpCurrentName = NULL,
			lpCurrentFileName = NULL;
	bool	bWeGotIt = false;

	for(size_t i = 0; i < coreDebugger->PIDs.size(); i++)
	{
		if(coreDebugger->PIDs[i].hProc == hProcess)
		{
			lpCurrentNameTemp = coreDebugger->PIDs[i].sFileName;
			break;
		}
	}

	if(lpCurrentNameTemp == NULL) 
	{
		clsMemManager::CFree(lpFileName);
		return;
	}

	lpCurrentName = clsHelperClass::reverseStrip(lpCurrentNameTemp,'\\');

	MEMORY_BASIC_INFORMATION mbi;
		
	while(VirtualQueryEx(hProcess,(LPVOID)CurAddress,&mbi,sizeof(mbi)))
	{
		if(GetMappedFileName(hProcess, (LPVOID)CurAddress, lpFileName, MAX_PATH) > 0)
		{
			lpCurrentFileName = clsHelperClass::reverseStrip(lpFileName,'\\');
			if(lpCurrentFileName != NULL && wcslen(lpCurrentFileName) > 0)
			{
				if(wcscmp(lpCurrentFileName,lpCurrentName) == 0)
				{
					if(!bWeGotIt)
					{
						bWeGotIt = true;
						ModuleBase = (DWORD64)mbi.BaseAddress;
					}
					
					ModuleSize += mbi.RegionSize;
				}
				else
				{
					if(bWeGotIt)
					{
						clsMemManager::CFree(lpCurrentFileName);
						break;
					}
				}

				clsMemManager::CFree(lpCurrentFileName);
			}
		}
		CurAddress += mbi.RegionSize;
	}

	for(int i = 0; i < callstackView->tblCallstack->rowCount(); i++)
	{
		DWORD64 currentFunction = callstackView->tblCallstack->item(i,3)->text().toULongLong(0,16);

		if(currentFunction >= ModuleBase && currentFunction < (ModuleBase + ModuleSize))
		{
			coreDebugger->StepOver(currentFunction);
			break;
		}
	}

	clsMemManager::CFree(lpCurrentName);
	clsMemManager::CFree(lpFileName);
}

void qtDLGNanomite::action_OptionsAbout()
{
	qtDLGAbout DLGAbout(this,Qt::Window);
	DLGAbout.exec();
}

void qtDLGNanomite::action_OptionsOptions()
{
	qtDLGOption newOption(this,Qt::Window);
	newOption.exec();
}

void qtDLGNanomite::action_OptionsUpdate()
{
	UUpdateWidget *pUpdater = new UUpdateWidget();
	pUpdater->show();
}

void qtDLGNanomite::action_WindowDetailInformation()
{
	dlgDetInfo->show();
}

void qtDLGNanomite::action_WindowBreakpointManager()
{
	dlgBPManager->show();
}

void qtDLGNanomite::action_WindowPatches()
{
	dlgPatchManager->show();
}

void qtDLGNanomite::action_WindowShowMemory()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_iMenuProcessID = -1;
		actionWindow_Show_Memory->setDisabled(true);

		GenerateMenu();

		if(m_iMenuProcessID >= 0)
		{
			qtDLGMemoryView *dlgMemory = new qtDLGMemoryView(this,Qt::Window,m_iMenuProcessID);
			dlgMemory->show();
		}

		actionWindow_Show_Memory->setEnabled(true);
		m_iMenuProcessID = -1;
	}
}

void qtDLGNanomite::action_WindowShowHeap()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_iMenuProcessID = -1;
		actionWindow_Show_Heap->setDisabled(true);

		GenerateMenu();

		if(m_iMenuProcessID >= 0)
		{
			qtDLGHeapView *dlgHeap = new qtDLGHeapView(this,Qt::Window,m_iMenuProcessID);
			dlgHeap->show();
		}

		actionWindow_Show_Heap->setEnabled(true);
		m_iMenuProcessID = -1;
	}
}

void qtDLGNanomite::action_WindowShowStrings()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_iMenuProcessID = -1;
		actionWindow_Show_Strings->setDisabled(true);

		GenerateMenu();

		if(m_iMenuProcessID >= 0)
		{
			qtDLGStringView *dlgString = new qtDLGStringView(this,Qt::Window,m_iMenuProcessID);
			dlgString->show();
		}

		actionWindow_Show_Strings->setEnabled(true);
		m_iMenuProcessID = -1;
	}
}

void qtDLGNanomite::action_WindowShowDebugOutput()
{
	dlgDbgStr->show();
}

void qtDLGNanomite::action_WindowShowHandles()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_iMenuProcessID = -1;
		actionWindow_Show_Handles->setDisabled(true);

		GenerateMenu();

		if(m_iMenuProcessID >= 0)
		{
			qtDLGHandleView *dlgHandle = new qtDLGHandleView(this,Qt::Window,m_iMenuProcessID);
			dlgHandle->show();
		}

		actionWindow_Show_Handles->setEnabled(true);
		m_iMenuProcessID = -1;
	}
}

void qtDLGNanomite::action_WindowShowWindows()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_iMenuProcessID = -1;
		actionWindow_Show_Windows->setDisabled(true);

		GenerateMenu();

		if(m_iMenuProcessID >= 0)
		{
			qtDLGWindowView *dlgWindows = new qtDLGWindowView(this,Qt::Window,m_iMenuProcessID);
			dlgWindows->show();
		}

		actionWindow_Show_Windows->setEnabled(true);
		m_iMenuProcessID = -1;
	}
}

void qtDLGNanomite::action_WindowShowPEEditor()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_iMenuProcessID = -1;
		actionWindow_Show_PEEditor->setDisabled(true);

		GenerateMenu(false);

		if(m_iMenuProcessID >= 0)
		{
			qtDLGPEEditor *dlgPEEditor = new qtDLGPEEditor(PEManager,this,Qt::Window,m_iMenuProcessID);
			dlgPEEditor->show();
		}

		actionWindow_Show_PEEditor->setEnabled(true);
		m_iMenuProcessID = -1;
	}
}

void qtDLGNanomite::action_WindowShowPrivileges()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_iMenuProcessID = -1;
		actionWindow_Show_Privileges->setDisabled(true);

		GenerateMenu(false);

		if(m_iMenuProcessID >= 0)
		{
			qtDLGProcessPrivilege *dlgProcPrivs = new qtDLGProcessPrivilege(this,Qt::Window,m_iMenuProcessID);
			dlgProcPrivs->show();
		}

		actionWindow_Show_Privileges->setEnabled(true);
		m_iMenuProcessID = -1;
	}
}

void qtDLGNanomite::action_DebugTraceShow()
{
	dlgTraceWindow->show();
}

void qtDLGNanomite::action_DebugTraceStart()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_iMenuProcessID = -1;
		actionDebug_Trace_Start->setDisabled(true);

		GenerateMenu(false);

		if(m_iMenuProcessID >= 0)
		{
			qtDLGTrace::clearTraceData();
			coreDebugger->SetTraceFlagForPID(m_iMenuProcessID,true);
			actionDebug_Trace_Stop->setEnabled(true);
			UpdateStateBar(STATE_TRACE);
		}
		else
			actionDebug_Trace_Start->setEnabled(true);

		m_iMenuProcessID = -1;
	}
}

void qtDLGNanomite::action_DebugTraceStop()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_iMenuProcessID = -1;
		actionDebug_Trace_Stop->setDisabled(true);

		GenerateMenu(false);

		if(m_iMenuProcessID >= 0)
		{
			coreDebugger->SetTraceFlagForPID(m_iMenuProcessID,false);
			actionDebug_Trace_Start->setEnabled(true);
		}
		else
			actionDebug_Trace_Stop->setEnabled(true);

		m_iMenuProcessID = -1;
	}
}

void qtDLGNanomite::action_WindowShowFunctions()
{
	if(coreDebugger->GetDebuggingState())
	{
		m_iMenuProcessID = -1;
		actionWindow_Show_Functions->setDisabled(true);

		GenerateMenu();

		if(m_iMenuProcessID >= 0)
		{
			qtDLGFunctions *dlgFunctions = new qtDLGFunctions(m_iMenuProcessID,this,Qt::Window);
			connect(dlgFunctions,SIGNAL(ShowInDisAs(quint64)),DisAsGUI,SLOT(OnDisplayDisassembly(quint64)),Qt::QueuedConnection);
			dlgFunctions->show();
		}

		actionWindow_Show_Functions->setEnabled(true);
		m_iMenuProcessID = -1;
	}
}