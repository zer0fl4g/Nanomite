#include "qtDLGNanomite.h"
#include "qtDLGAbout.h"
#include "qtDLGAttach.h"
#include "qtDLGOption.h"
#include "qtDLGMemoryView.h"
#include "qtDLGHeapView.h"
#include "qtDLGStringView.h"
#include "qtDLGHandleView.h"
#include "qtDLGWindowView.h"

#include "clsCallbacks.h"
#include "clsHelperClass.h"

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "BeaEngine.h"

#include <sstream>

using namespace std;

qtDLGNanomite* qtDLGNanomite::qtDLGMyWindow = NULL;

qtDLGNanomite::qtDLGNanomite(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	setupUi(this);
	this->setFixedSize(this->width(),this->height());

	qRegisterMetaType<DWORD>("DWORD");
	qRegisterMetaType<DWORD64>("DWORD64");
	qRegisterMetaType<wstring>("wstring");

	coreDebugger = new clsDebugger;
	clsCallbacks *NanomiteCallbacks = new clsCallbacks;
	dlgDetInfo = new qtDLGDetailInfo(this,Qt::Window);
	dlgDbgStr = new qtDLGDebugStrings(this,Qt::Window);
	dlgBPManager = new qtDLGBreakPointManager(this,Qt::Window);

	InitListSizes();

	qtDLGMyWindow = this;
	lExceptionCount = 0;
	clsHelperClass::ReadFromSettingsFile(coreDebugger);


	// Callbacks from Debugger Thread to GUI
	connect(coreDebugger,SIGNAL(OnThread(DWORD,DWORD,DWORD64,bool,DWORD,bool)),
		NanomiteCallbacks,SLOT(OnThread(DWORD,DWORD,DWORD64,bool,DWORD,bool)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnPID(DWORD,std::wstring,DWORD,DWORD64,bool)),
		NanomiteCallbacks,SLOT(OnPID(DWORD,std::wstring,DWORD,DWORD64,bool)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnException(std::wstring,std::wstring,DWORD64,DWORD64,DWORD,DWORD)),
		NanomiteCallbacks,SLOT(OnException(std::wstring,std::wstring,DWORD64,DWORD64,DWORD,DWORD)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDbgString(std::wstring,DWORD)),
		NanomiteCallbacks,SLOT(OnDbgString(std::wstring,DWORD)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDll(std::wstring,DWORD,DWORD64,bool)),
		NanomiteCallbacks,SLOT(OnDll(std::wstring,DWORD,DWORD64,bool)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnLog(tm*,std::wstring)),
		NanomiteCallbacks,SLOT(OnLog(tm*,std::wstring)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnCallStack(DWORD64,DWORD64,std::wstring,std::wstring,DWORD64,std::wstring,std::wstring,std::wstring,int)),
		NanomiteCallbacks,SLOT(OnCallStack(DWORD64,DWORD64,std::wstring,std::wstring,DWORD64,std::wstring,std::wstring,std::wstring,int)),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDebuggerBreak()),this,SLOT(OnDebuggerBreak()),Qt::QueuedConnection);
	connect(coreDebugger,SIGNAL(OnDebuggerTerminated()),this,SLOT(OnDebuggerTerminated()),Qt::QueuedConnection);

	// Actions for the MainMenu and Toolbar
	connect(actionFile_OpenNew, SIGNAL(triggered()), this, SLOT(action_FileOpenNewFile()));
	connect(actionFile_AttachTo, SIGNAL(triggered()), this, SLOT(action_FileAttachTo()));
	connect(actionFile_Detach, SIGNAL(triggered()), this, SLOT(action_FileDetach()));
	connect(actionFile_Exit, SIGNAL(triggered()), this, SLOT(action_FileTerminateGUI()));
	connect(actionDebug_Start, SIGNAL(triggered()), this, SLOT(action_DebugStart()));
	connect(actionDebug_Stop, SIGNAL(triggered()), this, SLOT(action_DebugStop()));
	connect(actionDebug_Restart, SIGNAL(triggered()), this, SLOT(action_DebugRestart()));
	connect(actionDebug_Suspend, SIGNAL(triggered()), this, SLOT(action_DebugSuspend()));
	connect(actionDebug_Step_In, SIGNAL(triggered()), this, SLOT(action_DebugStepIn()));
	connect(actionDebug_Step_Over, SIGNAL(triggered()), this, SLOT(action_DebugStepOver()));
	connect(actionOptions_About, SIGNAL(triggered()), this, SLOT(action_OptionsAbout()));
	connect(actionOptions_Options, SIGNAL(triggered()), this, SLOT(action_OptionsOptions()));
	connect(actionWindow_Detail_Information, SIGNAL(triggered()), this, SLOT(action_WindowDetailInformation()));
	connect(actionWindow_Breakpoint_Manager, SIGNAL(triggered()), this, SLOT(action_WindowBreakpointManager()));
	connect(actionWindow_Show_Memory, SIGNAL(triggered()), this, SLOT(action_WindowShowMemory()));
	connect(actionWindow_Show_Heap, SIGNAL(triggered()), this, SLOT(action_WindowShowHeap()));
	connect(actionWindow_Show_Strings, SIGNAL(triggered()), this, SLOT(action_WindowShowStrings()));
	connect(actionWindow_Show_Debug_Output, SIGNAL(triggered()), this, SLOT(action_WindowShowDebugOutput()));
	connect(actionWindow_Show_Handles, SIGNAL(triggered()), this, SLOT(action_WindowShowHandles()));
	connect(actionWindow_Show_Windows, SIGNAL(triggered()), this, SLOT(action_WindowShowWindows()));

	//Actions on Window Events
	connect(scrollStackView,SIGNAL(valueChanged(int)),this,SLOT(OnStackScroll(int)));
	connect(scrollDisAs,SIGNAL(valueChanged(int)),this,SLOT(OnDisAsScroll(int)));
}

qtDLGNanomite::~qtDLGNanomite()
{

}

qtDLGNanomite* qtDLGNanomite::GetInstance()
{
	return qtDLGMyWindow;
}

void qtDLGNanomite::action_FileTerminateGUI()
{
	if(coreDebugger->GetDebuggingState())
		coreDebugger->StopDebuggingAll();
	close();
}

void qtDLGNanomite::action_FileOpenNewFile()
{
	if(coreDebugger->GetDebuggingState())
		coreDebugger->StopDebuggingAll();

	coreDebugger->RemoveBPs();
	clsHelperClass::MenuLoadNewFile(coreDebugger);

	CleanGUI();
	UpdateStateBar(0x3);
}

void qtDLGNanomite::action_FileAttachTo()
{
	if(!coreDebugger->GetDebuggingState())
	{
		 qtDLGAttach newAttaching;
		 connect(&newAttaching,SIGNAL(StartAttachedDebug(int)),this,SLOT(action_DebugAttachStart(int)));
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
			UpdateStateBar(0x3);
	}
}

void qtDLGNanomite::action_DebugStart()
{
	if(!coreDebugger->GetDebuggingState())
	{
		CleanGUI();

		if(!coreDebugger->IsTargetSet())
				if(!clsHelperClass::MenuLoadNewFile(coreDebugger))
					return;

		coreDebugger->start();

		UpdateStateBar(0x1);
	}
	else
	{
		coreDebugger->ResumeDebugging();
		UpdateStateBar(0x1);
	}
}

void qtDLGNanomite::action_DebugAttachStart(int iPID)
{
	if(!coreDebugger->GetDebuggingState())
	{
		CleanGUI();

		coreDebugger->AttachToProcess(iPID);
		coreDebugger->start();

		UpdateStateBar(0x1);
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
		coreDebugger->StopDebuggingAll();

	CleanGUI();
	if(coreDebugger->IsTargetSet())
		coreDebugger->start();
}

void qtDLGNanomite::action_DebugSuspend()
{
	_iMenuPID = -1;
	actionDebug_Suspend->setDisabled(true);

	GenerateMenu();

	if(_iMenuPID == 0)
	{
		if(coreDebugger->GetDebuggingState())
		{
			coreDebugger->SuspendDebuggingAll();
			UpdateStateBar(0x2);
		}
	}
	else
	{
		if(coreDebugger->GetDebuggingState())
		{
			coreDebugger->SuspendDebugging(_iMenuPID);
			UpdateStateBar(0x2);
		}
	}

	actionDebug_Suspend->setEnabled(true);
	_iMenuPID = -1;
}

void qtDLGNanomite::action_DebugStepIn()
{
	if(coreDebugger->GetDebuggingState())
		coreDebugger->StepIn();
}

void qtDLGNanomite::action_DebugStepOver()
{
//	if(coreDebugger->GetDebuggingState())
#ifdef _AMD64_
//		coreDebugger->StepOver(CalcNewOffset(coreDebugger->ProcessContext.Rip));
#else
//		coreDebugger->StepOver(CalcNewOffset(coreDebugger->ProcessContext.Eip));
#endif
}

void qtDLGNanomite::action_OptionsAbout()
{
	qtDLGAbout DLGAbout;
	DLGAbout.exec();
}

void qtDLGNanomite::action_OptionsOptions()
{
	qtDLGOption newOption;
	newOption.exec();
}

void qtDLGNanomite::action_WindowDetailInformation()
{
	dlgDetInfo->show();
}

void qtDLGNanomite::action_WindowBreakpointManager()
{
	dlgBPManager->show();
}

void qtDLGNanomite::action_WindowShowMemory()
{
	if(coreDebugger->GetDebuggingState())
	{
		_iMenuPID = -1;
		actionWindow_Show_Memory->setDisabled(true);
	
		GenerateMenu();

		if(_iMenuPID >= 0)
		{
			qtDLGMemoryView *dlgMemory = new qtDLGMemoryView(this,Qt::Window,_iMenuPID);
			dlgMemory->show();
		}

		actionWindow_Show_Memory->setEnabled(true);
		_iMenuPID = -1;
	}
}

void qtDLGNanomite::action_WindowShowHeap()
{
	if(coreDebugger->GetDebuggingState())
	{
		_iMenuPID = -1;
		actionWindow_Show_Heap->setDisabled(true);
	
		GenerateMenu();

		if(_iMenuPID >= 0)
		{
			qtDLGHeapView *dlgHeap = new qtDLGHeapView(this,Qt::Window,_iMenuPID);
			dlgHeap->show();
		}

		actionWindow_Show_Heap->setEnabled(true);
		_iMenuPID = -1;
	}
}

void qtDLGNanomite::action_WindowShowStrings()
{
	if(coreDebugger->GetDebuggingState())
	{
		_iMenuPID = -1;
		actionWindow_Show_Strings->setDisabled(true);
	
		GenerateMenu();

		if(_iMenuPID >= 0)
		{
			qtDLGStringView *dlgString = new qtDLGStringView(this,Qt::Window,_iMenuPID);
			dlgString->show();
		}

		actionWindow_Show_Strings->setEnabled(true);
		_iMenuPID = -1;
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
		_iMenuPID = -1;
		actionWindow_Show_Handles->setDisabled(true);
	
		GenerateMenu();

		if(_iMenuPID >= 0)
		{
			qtDLGHandleView *dlgHandle = new qtDLGHandleView(this,Qt::Window,_iMenuPID);
			dlgHandle->show();
		}

		actionWindow_Show_Handles->setEnabled(true);
		_iMenuPID = -1;
	}
}

void qtDLGNanomite::action_WindowShowWindows()
{
	if(coreDebugger->GetDebuggingState())
	{
		_iMenuPID = -1;
		actionWindow_Show_Windows->setDisabled(true);
	
		GenerateMenu();

		if(_iMenuPID >= 0)
		{
			qtDLGWindowView *dlgWindows = new qtDLGWindowView(this,Qt::Window,_iMenuPID);
			dlgWindows->show();
		}

		actionWindow_Show_Windows->setEnabled(true);
		_iMenuPID = -1;
	}
}

void qtDLGNanomite::OnDebuggerBreak()
{
	if(!coreDebugger->GetDebuggingState())
		UpdateStateBar(0x3);
	else
	{
		tblCallstack->setRowCount(0);
		coreDebugger->ShowCallStack();

		tblRegView->setRowCount(0);
		LoadRegView();

#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		HANDLE hProcess = coreDebugger->GetCurrentProcessHandle();

		IsWow64Process(hProcess,&bIsWOW64);
		if(bIsWOW64)
		{
			LoadStackView(coreDebugger->wowProcessContext.Esp,4);
			LoadDisAssView(coreDebugger->wowProcessContext.Eip);
		}
		else
		{
			LoadStackView(coreDebugger->ProcessContext.Rsp,8);
			LoadDisAssView(coreDebugger->ProcessContext.Rip);
		}
#else
		LoadStackView(coreDebugger->ProcessContext.Esp,4);
		LoadDisAssView(coreDebugger->ProcessContext.Eip);
#endif			
		UpdateStateBar(0x2);
	}
}

void qtDLGNanomite::UpdateStateBar(DWORD dwAction)
{
	QString qsStateMessage = QString().sprintf("\t\tPIDs: %d  TIDs: %d  DLLs: %d  Exceptions: %d State: ",
		coreDebugger->PIDs.size(),
		coreDebugger->TIDs.size(),
		coreDebugger->DLLs.size(),
		lExceptionCount);

	switch(dwAction)
	{
	case 0x1: // Running
		qsStateMessage.append("Running");
		break;
	case 0x2: // Suspended
		qsStateMessage.append("Suspended");
		break;
	case 0x3: // Terminated
		qsStateMessage.append("Terminated");
		break;
	}
	stateBar->showMessage(qsStateMessage);
}

void qtDLGNanomite::LoadStackView(DWORD64 dwESP, DWORD dwStackSize)
{
	bool bCheckVar = false;
	QString strTemp;
	SIZE_T dwBytesRead = NULL;
	LPBYTE bBuffer;
	PTCHAR sTemp;
	HANDLE hProcess = coreDebugger->GetCurrentProcessHandle();
	DWORD dwOldProtect = NULL,
		dwNewProtect = PAGE_EXECUTE_READWRITE,
		dwSize = 12 * dwStackSize;
	DWORD64	dwStartOffset = dwESP - dwStackSize * 6,
		dwEndOffset = dwESP + dwStackSize * 6;

	if(!coreDebugger->GetDebuggingState())
		return;

	if(hProcess == INVALID_HANDLE_VALUE)
		return;

	if(!VirtualProtectEx(hProcess,(LPVOID)dwStartOffset,dwSize,dwNewProtect,&dwOldProtect))
		return;

	bBuffer = (LPBYTE)malloc(dwSize);
	if(bBuffer == NULL)
		return;

	if(!ReadProcessMemory(hProcess,(LPVOID)dwStartOffset,(LPVOID)bBuffer,dwSize,&dwBytesRead))
		return;

	sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

	tblStack->setRowCount(0);
	for(size_t i = 0;i < (dwSize / dwStackSize); i++)
	{
		tblStack->insertRow(tblStack->rowCount());
		int itemIndex = tblStack->rowCount();

		// Current Offset
		wsprintf(sTemp,L"0x%016I64X",(dwStartOffset + i * dwStackSize));
		strTemp = QString::fromWCharArray(sTemp);
		tblStack->setItem(itemIndex - 1,0,new QTableWidgetItem(strTemp));

		// Value
		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		IsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

		if(bIsWOW64)
			for(int id = 3;id != -1;id--)
				wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * dwStackSize + id)));
		else
			for(int id = 7;id != -1;id--)
				wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * dwStackSize + id)));

#else
		for(int id = 3;id != -1;id--)
			wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * dwStackSize + id)));
#endif
		strTemp = QString::fromWCharArray(sTemp);
		tblStack->setItem(itemIndex - 1,1,new QTableWidgetItem(strTemp));

		// Comment
		DWORD dwOffset = NULL;
		wstringstream ss; ss << sTemp; ss >> hex >> dwOffset;
		wstring sFuncName,sModName;
		coreDebugger->LoadSymbolForAddr(sFuncName,sModName,dwOffset);
		if(sFuncName != L"")
			wsprintf(sTemp,L"%s.%s",sModName.c_str(),sFuncName.c_str());
		else 
			wsprintf(sTemp,L"%s",L"");
		strTemp = QString::fromWCharArray(sTemp);
		tblStack->setItem(itemIndex - 1,2,new QTableWidgetItem(strTemp));
	}

	bCheckVar = VirtualProtectEx(hProcess,(LPVOID)dwStartOffset,dwSize,dwOldProtect,NULL);
	free(bBuffer);
	free(sTemp);
}

void qtDLGNanomite::LoadDisAssView(DWORD64 dwEIP)
{
	if(coreDebugger->GetCurrentPID() == 0 || dwEIP == 0)
		return;

	DISASM newDisAss;
	bool bContinueDisAs = true;
	DWORD64 dwEndOffset = dwEIP + 100,
		dwStartOffset = dwEIP - 50;
	DWORD dwOldProtection = 0,
		dwNewProtection = PAGE_EXECUTE_READWRITE;
	int iLen = 0,iVor = 0, iNach = 0;
	QString strTemp;

	HANDLE hProc = coreDebugger->GetCurrentProcessHandle();
	LPVOID pBuffer = malloc(150);

	memset(&newDisAss, 0, sizeof(DISASM));

	bool bUnProtect = VirtualProtectEx(hProc,(LPVOID)dwStartOffset,150,dwNewProtection,&dwOldProtection);
	if(ReadProcessMemory(hProc,(LPVOID)dwStartOffset,pBuffer,150,NULL))
	{
		newDisAss.EIP = (DWORD64)pBuffer;
		newDisAss.VirtualAddr = dwStartOffset;
#ifdef _AMD64_
		newDisAss.Archi = 64;
#else
		newDisAss.Archi = 0;
#endif
		tblDisAs->setRowCount(0);

		PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
		while(bContinueDisAs)
		{
			newDisAss.SecurityBlock = (int)dwEndOffset - newDisAss.VirtualAddr;

			iLen = Disasm(&newDisAss);

			if (iLen == OUT_OF_BLOCK)
				bContinueDisAs = false;
			else
			{			
				memset(sTemp,0,MAX_PATH *  sizeof(TCHAR));
				int itemIndex = tblDisAs->rowCount();
				tblDisAs->insertRow(tblDisAs->rowCount());
				
				// Current Offset
				swprintf(sTemp,L"0x%016I64X",newDisAss.VirtualAddr);
				strTemp = QString::fromWCharArray(sTemp);
				tblDisAs->setItem(itemIndex - 1,0,new QTableWidgetItem(strTemp));

				// OpCodez
				BYTE bBuffer;
				bool bReaded = false;
				SIZE_T dwBytesRead = NULL;

				memset(sTemp,0,MAX_PATH *  sizeof(TCHAR));
				int iTempLen = ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? 1 : ((iLen == UNKNOWN_OPCODE) ? 0 : iLen));
				for(size_t i = 0;i < iTempLen;i++)
				{
					DWORD64 dwOffset = newDisAss.VirtualAddr + i;
					bReaded = ReadProcessMemory(hProc,(LPVOID)dwOffset,&bBuffer,1,&dwBytesRead);
					wsprintf(sTemp,L"%s %02X",sTemp,bBuffer);
				}
				strTemp = QString::fromWCharArray(sTemp);
				tblDisAs->setItem(itemIndex - 1,1,new QTableWidgetItem(strTemp));

				// Instruction
				if(newDisAss.Instruction.Opcode == 0x00 && iLen == 2)
					wsprintf(sTemp,L"%s",L"db 00");
				else
					wsprintf(sTemp,L"%S",newDisAss.CompleteInstr);						
				strTemp = QString::fromWCharArray(sTemp);
				tblDisAs->setItem(itemIndex - 1,2,new QTableWidgetItem(strTemp));

				// Comment
				memset(sTemp,NULL,MAX_PATH * sizeof(TCHAR));
				if(strstr(newDisAss.Instruction.Mnemonic,"call") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"jmp") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"push") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"jnz") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"je") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"jl") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"jng") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"jne") != 0)
				{
					wstring sFuncName,sModName;
					coreDebugger->LoadSymbolForAddr(sFuncName,sModName,newDisAss.Instruction.AddrValue);
					if(sFuncName.length() > 0 && sModName.length() > 0)
						wsprintf(sTemp,L"%s.%s",sModName.c_str(),sFuncName.c_str());
					else if(sModName.length() > 0 && sFuncName.length() == 0)
						wsprintf(sTemp,L"%s.0x%08X",sModName.c_str(),newDisAss.VirtualAddr);
				}
				else
				{
					//if(newDisAss.Instruction.Opcode != 0xCC &&
					//	newDisAss.Instruction.Opcode != 0x90)
					//{
					//	wstring sFuncName,sModName;
					//	coreDebugger->LoadSymbolForAddr(sFuncName,sModName,newDisAss.VirtualAddr);
					//	if(sFuncName.length() > 0)
					//		wsprintf(sTemp,L"%s",sFuncName.c_str());
					//	else 
					//		wsprintf(sTemp,L"%s",L"");
					//}
					//else
						wsprintf(sTemp,L"%s",L"");
				}

				strTemp = QString::fromWCharArray(sTemp);
				tblDisAs->setItem(itemIndex - 1,3,new QTableWidgetItem(strTemp));
			}

			newDisAss.EIP = newDisAss.EIP + ((iLen == UNKNOWN_OPCODE) ? 1 : ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? iLen -= 1 : iLen));
			newDisAss.VirtualAddr = newDisAss.VirtualAddr + ((iLen == UNKNOWN_OPCODE) ? 1 : ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? iLen -= 1 : iLen));
			if (newDisAss.VirtualAddr >= dwEndOffset)
				bContinueDisAs = false;
		}
		free(sTemp);
	}

	bool bProtect = VirtualProtectEx(hProc,(LPVOID)dwStartOffset,150,dwOldProtection,NULL);
	free(pBuffer);
	
	for(int i = 0; i < tblDisAs->rowCount(); i++)
	{
		if(dwEIP == tblDisAs->item(i,0)->text().toULongLong(0,16))
		{
			if(i > 5)
				for(int iBefore = 0; iBefore < (i - 5); iBefore++)
					tblDisAs->removeRow(iBefore);

			if(tblDisAs->rowCount() >= 25)
				for(int iAfter = 25; iAfter <= tblDisAs->rowCount(); iAfter++)
					tblDisAs->removeRow(iAfter = 25);
			break;
		}
	}
}

void qtDLGNanomite::InitListSizes()
{
	// List StackView
	tblStack->horizontalHeader()->resizeSection(0,135);
	tblStack->horizontalHeader()->resizeSection(1,135);

	// List LogBox
	tblLogBox->horizontalHeader()->resizeSection(0,70);

	// List CallStack
	tblCallstack->horizontalHeader()->resizeSection(0,135);
	tblCallstack->horizontalHeader()->resizeSection(1,135);
	tblCallstack->horizontalHeader()->resizeSection(2,275);
	tblCallstack->horizontalHeader()->resizeSection(3,135);
	tblCallstack->horizontalHeader()->resizeSection(4,275);
	tblCallstack->horizontalHeader()->resizeSection(5,50);

	// List DisAs
	tblDisAs->horizontalHeader()->resizeSection(0,135);
	tblDisAs->horizontalHeader()->resizeSection(1,200);
	tblDisAs->horizontalHeader()->resizeSection(2,200);

	// List Register
	tblRegView->horizontalHeader()->resizeSection(0,50);
	tblRegView->horizontalHeader()->resizeSection(1,155);

	// List DetInfo  Processes
	dlgDetInfo->tblPIDs->horizontalHeader()->resizeSection(0,135);
	dlgDetInfo->tblPIDs->horizontalHeader()->resizeSection(1,135);
	dlgDetInfo->tblPIDs->horizontalHeader()->resizeSection(2,135);

	// List DetInfo  Threads
	dlgDetInfo->tblTIDs->horizontalHeader()->resizeSection(0,135);
	dlgDetInfo->tblTIDs->horizontalHeader()->resizeSection(1,135);
	dlgDetInfo->tblTIDs->horizontalHeader()->resizeSection(2,135);
	dlgDetInfo->tblTIDs->horizontalHeader()->resizeSection(3,135);

	// List DetInfo  Exceptions
	dlgDetInfo->tblExceptions->horizontalHeader()->resizeSection(0,135);
	dlgDetInfo->tblExceptions->horizontalHeader()->resizeSection(1,135);
	dlgDetInfo->tblExceptions->horizontalHeader()->resizeSection(2,140);

	// List DetInfo  Modules
	dlgDetInfo->tblModules->horizontalHeader()->resizeSection(0,135);
	dlgDetInfo->tblModules->horizontalHeader()->resizeSection(1,135);
	dlgDetInfo->tblModules->horizontalHeader()->resizeSection(2,135);

	// List Debug Strings
	dlgDbgStr->tblDebugStrings->horizontalHeader()->resizeSection(0,75);

	// List BP Manager
	dlgBPManager->tblBPs->horizontalHeader()->resizeSection(0,75);
	dlgBPManager->tblBPs->horizontalHeader()->resizeSection(1,135);
	dlgBPManager->tblBPs->horizontalHeader()->resizeSection(2,135);
	dlgBPManager->tblBPs->horizontalHeader()->resizeSection(3,50);
}

void qtDLGNanomite::OnStackScroll(int iValue)
{
	if(iValue == 5) return;

	DWORD dwStackSize = NULL;
	quint64 dwOffset = NULL;
	QString strTemp;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	IsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64) dwStackSize = 4;	
	else dwStackSize = 8;
#else
	dwStackSize = 4;
#endif

	if(tblStack->rowCount() > 0)
		strTemp = tblStack->item(0,0)->text();
	if(tblStack->rowCount() <= 0 || QString().compare(strTemp,"0") == 0)
#ifdef _AMD64_
		dwOffset = coreDebugger->ProcessContext.Rsp;
#else
		dwOffset = coreDebugger->ProcessContext.Esp;
#endif
	else
		dwOffset = strTemp.toULongLong(0,16);

	if(iValue < 5)
		LoadStackView(dwOffset,dwStackSize);
	else
		LoadStackView(dwOffset + (dwStackSize * 12),dwStackSize);
	scrollStackView->setValue(5);
}

void qtDLGNanomite::OnDisAsScroll(int iValue)
{
	if(iValue == 5 || tblDisAs->rowCount() <= 0) return;
	else if(iValue < 5)
		LoadDisAssView(tblDisAs->item(0,0)->text().toULongLong(0,16));
	else
		LoadDisAssView(tblDisAs->item(10,0)->text().toULongLong(0,16));
	scrollDisAs->setValue(5);
}

void qtDLGNanomite::LoadRegView()
{
#ifdef _AMD64_
	// RAX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RAX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Rax)));
	// RBX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RBX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Rbx)));
	// RCX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RCX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Rcx)));
	// RDX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RDX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Rdx)));
	// RSP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RSP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Rsp)));
	// RBP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RBP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Rbp)));
	// RSI
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RSI"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Rsi)));
	// RIP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RIP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Rip)));
#else
	// EAX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EAX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Eax)));
	// EBX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EBX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Ebx)));
	// ECX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ECX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Ecx)));
	// EDX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EDX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Edx)));
	// ESP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ESP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Esp)));
	// EBP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EBP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Ebp)));
	// ESI
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ESI"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Esi)));
	// EIP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EIP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString().sprintf("0x%016X",coreDebugger->ProcessContext.Eip)));
#endif
}

void qtDLGNanomite::CleanGUI()
{
	tblStack->setRowCount(0);
	tblLogBox->setRowCount(0);
	tblCallstack->setRowCount(0);
	tblDisAs->setRowCount(0);
	tblRegView->setRowCount(0);
	
	dlgDetInfo->tblPIDs->setRowCount(0);
	dlgDetInfo->tblTIDs->setRowCount(0);
	dlgDetInfo->tblExceptions->setRowCount(0);
	dlgDetInfo->tblModules->setRowCount(0);

	dlgDbgStr->tblDebugStrings->setRowCount(0);

	lExceptionCount = 0;
}

void qtDLGNanomite::OnDebuggerTerminated()
{
	UpdateStateBar(0x3);
}

void qtDLGNanomite::GenerateMenuCallback(QAction *qAction)
{
	_iMenuPID = qAction->text().toULong(0,16);
}

void qtDLGNanomite::GenerateMenu()
{
	QAction *qAction;
	QMenu menu;

	for(int i = 0; i < coreDebugger->PIDs.size(); i++)
	{
		if(coreDebugger->PIDs[i].bRunning)
		{
		qAction = new QAction(QString().sprintf("%08X",coreDebugger->PIDs[i].dwPID),this);
		menu.addAction(qAction);
		}
	}
	menu.addSeparator();
	qAction = new QAction("All",this);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(GenerateMenuCallback(QAction*)));
	menu.addAction(qAction);
	menu.exec(QCursor::pos());
}