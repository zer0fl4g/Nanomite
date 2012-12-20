#include "../clsDebugger/clsDebugger.h"

// Leak detection
#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>
// Leak detection

// Our callback function which gets triggered if log gets updated
int OnLog(tm timeInfo,wstring sLog)
{
	wprintf(L"%s\n",sLog.c_str()); // Just print the new string to console
	return 0;
}

// Our callback function which gets triggered on "clsDebugger.ShowCallstack()"
int OnCallStack(DWORD64 dwStackAddr,
	DWORD64 dwReturnTo,wstring sReturnToFunc,wstring sReturnToModuleName,
	DWORD64 dwEIP,wstring sFuncName,wstring sFuncModule,
	wstring sSourceFilePath,int iSourceLineNum)
{
#ifdef _AMD64_
	wprintf(L"\nCurrent:\t%016I64X - %s.%s\nReturnTo:\t%016I64X - %s.%s\n",dwEIP,sFuncModule.c_str(),sFuncName.c_str(),dwReturnTo,sReturnToModuleName.c_str(),sReturnToFunc.c_str());
#else
	wprintf(L"\nCurrent:\t%08X - %s.%s\nReturnTo:\t%08X - %s.%s\n",(DWORD)dwEIP,sFuncModule.c_str(),sFuncName.c_str(),(DWORD)dwReturnTo,sReturnToModuleName.c_str(),sReturnToFunc.c_str());
#endif
	return 0;
}

// Our callback for a 0x1337 exception
DWORD On1337Exception(DEBUG_EVENT *debug_event)
{
	MessageBoxW(NULL,L"You got a 1337 exception!",L"Nanomite!",MB_OK);
	return DBG_CONTINUE;
}

/* 
| This is only in a extra function because it simplifies the process of memory leak fixing 
*/
DWORD TheDebuggingFunction()
{
	bool bDebugIt = true;

	/* Creating new Instance with a Target
	| Without parameter for Attaching to a running Process (or set it manual later with tempDebugger.SetTarget(<path>))
	*/

#ifdef _AMD64_
	clsDebugger tempDebugger(L"C:\\DropBox\\Projects\\clsDebugger\\x64\\Debug\\Debugme.exe");
	//clsDebugger tempDebugger(L"C:\\Dropbox\\Projects\\clsDebugger\\Debug\\Debugme.exe");
#else
	clsDebugger tempDebugger(L"C:\\Dropbox\\Projects\\clsDebugger\\Debug\\Debugme.exe");
#endif
	//clsDebugger tempDebugger; // needs manual set of target with .SetTarget(<path>)

	 // Setting Option to enable child process debugging
	tempDebugger.dbgSettings.bDebugChilds = true;

	/* BreakOn: 
	| 0 = default application EP
	| 1 = SystemEP 
	| 2 = TLSCallback // Not working right now
	| 3 = Direct run
	*/
	tempDebugger.dbgSettings.dwBreakOnEPMode = 3;

	/* Enable auto load of symbols
	| Note: Disable may cause missing function names in callback/exception info but target will load faster
	*/
	tempDebugger.dbgSettings.bAutoLoadSymbols = true;

	/* Set Callbacks 
	| int (*dwOnThread)(DWORD dwPID,DWORD dwTID,DWORD64 dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
	| int (*dwOnPID)(DWORD dwPID,wstring sFile,DWORD dwExitCode,DWORD64 dwEP,bool bFound);
	| int (*dwOnException)(wstring sFuncName,wstring sModName,DWORD64 dwOffset,DWORD64 dwExceptionCode,DWORD dwPID,DWORD dwTID);
	| int (*dwOnDbgString)(wstring sMessage,DWORD dwPID);
	| int (*dwOnLog)(tm timeInfo,wstring sLog);
	| int (*dwOnDll)(wstring sDLLPath,DWORD dwPID,DWORD64 dwEP,bool bLoaded);
	| int (*dwOnCallStack)(DWORD64 dwStackAddr,DWORD64 dwReturnTo,wstring sReturnToFunc,wstring sModuleName,DWORD64 dwEIP,wstring sFuncName,wstring sFuncModule,wstring sSourceFilePath,int iSourceLineNum);
	*/
	tempDebugger.dwOnCallStack = &OnCallStack;
	tempDebugger.dwOnLog = &OnLog; 
	
	/* Set manual BPs
	|
	| dwType:
	|		0 = SoftwareBP
	|	    1 = MemoryBP
	|		2 = HardwareBP 
	|
	| dwTypeFlag: Only for HardwareBP currently
	|		DR_EXECUTE	(0x00) - Break on execute
	|		DR_WRITE	(0x01) - Break on data writes
	|		DR_READ		(0x11) - Break on data reads or writes.
	|
	| dwPID: ProcessID where you want to place the BP ( must be a process that is debugged )
	|		 Can be -1 for mother process if you have different child processes
	|
	| dwOffset: Address where the BP will be placed
	|	
	| dwSlot: Only for HW Breakpoints
	|		0 - Dr0
	|		1 - Dr1
	|		2 - Dr2
	|		3 - Dr3	
	|
	| dwKeep: set true if the BP should be placed again after one hit
	|
	| Notes: - For HardwareBPs you will need admin rights, else they will not work properly...
	|
	*/
	tempDebugger.AddBreakpointToList(0,NULL,-1,(DWORD64)GetProcAddress(LoadLibrary(L"Kernel32.dll"),"OutputDebugStringW"),0,true);
	tempDebugger.AddBreakpointToList(0,DR_EXECUTE,-1,(DWORD64)GetProcAddress(LoadLibrary(L"User32.dll"),"MessageBoxW"),0,true);

	/* Add a Exception Handler / Action
	|
	| dwExceptionType:	- The value of the exception e.g. 0xC0000005L for AccessViolation
	|					- can´t be EXCEPTION_BREAKPOINT or EXCEPTION_SINGLE_STEP
	|
	| dwAction:	0 = Break and wait for User
	|			1 = pass to App/Os
	|			2 = Ignore
	|			3 = automatic workaround ( jumping to the return addr which is on the stack )
	|
	| dwHandler: Optional! The Address of your custom handler - DWORD (__stdcall *CustomHandler)(DEBUG_EVENT *debug_event);
	|
	*/
	tempDebugger.CustomExceptionAdd(0x1337,0,(DWORD64)&On1337Exception);
	tempDebugger.CustomExceptionAdd(EXCEPTION_ACCESS_VIOLATION,1,NULL);
	tempDebugger.CustomExceptionAdd(EXCEPTION_ILLEGAL_INSTRUCTION,1,NULL);
	tempDebugger.CustomExceptionAdd(EXCEPTION_PRIV_INSTRUCTION,1,NULL);


	// Run Debugging
	tempDebugger.StartDebugging();
	// For attaching to a running Process we need first to call .AttachToProcess(PID) and then .StartDebugging()
	//tempDebugger.AttachToProcess(5800);
	//tempDebugger.StartDebugging();

	// Our debugging loop which handles the events
	// There you can do all things you want to have be done when the targets breaks cause of a error, manual suspend or breakpoints
	while(bDebugIt)
	{
		WaitForSingleObject(tempDebugger.hDebuggingHandle,INFINITE); // Waiting for Debugevent

		if(!tempDebugger.GetDebuggingState()) // Check if debugging is still running
			bDebugIt = false; // if not just quit
		else
		{
			// if still running print CPU registers
#ifdef _AMD64_
			BOOL bIsWow64Process = false;
			HANDLE hProcess = NULL;

			for(size_t i = 0; i < tempDebugger.PIDs.size(); i++)
				if(tempDebugger.PIDs[i].dwPID == tempDebugger.GetCurrentPID())
					hProcess = tempDebugger.PIDs[i].hProc;

			IsWow64Process(hProcess,&bIsWow64Process);
			if(bIsWow64Process)
			{
				wprintf(L"\nCONTEXT:\n\tEAX: %016X\n\tEBX: %016X\n\tECX: %016X\n\tEDX: %016X\n\tEIP: %016X\n\tESP: %016X\n\tEBP: %016X\n\tESI: %016X\n\tEDI: %016X\n\n",
					tempDebugger.wowProcessContext.Eax,
					tempDebugger.wowProcessContext.Ebx,
					tempDebugger.wowProcessContext.Ecx,
					tempDebugger.wowProcessContext.Edx,
					tempDebugger.wowProcessContext.Eip,
					tempDebugger.wowProcessContext.Esp,
					tempDebugger.wowProcessContext.Ebp,
					tempDebugger.wowProcessContext.Esi,
					tempDebugger.wowProcessContext.Edi
					);
			}
			else
			{
				wprintf(L"\nCONTEXT:\n\tRAX: %016I64X\n\tRBX: %016I64X\n\tRCX: %016I64X\n\tRDX: %016I64X\n\tRIP: %016I64X\n\tRSP: %016I64X\n\tRBP: %016I64X\n\tRSI: %016I64X\n\tRDI: %016I64X\n\n",
					tempDebugger.ProcessContext.Rax,
					tempDebugger.ProcessContext.Rbx,
					tempDebugger.ProcessContext.Rcx,
					tempDebugger.ProcessContext.Rdx,
					tempDebugger.ProcessContext.Rip,
					tempDebugger.ProcessContext.Rsp,
					tempDebugger.ProcessContext.Rbp,
					tempDebugger.ProcessContext.Rsi,
					tempDebugger.ProcessContext.Rdi
					);
			}

#else
			wprintf(L"\nCONTEXT:\n\tEAX: %08X\n\tEBX: %08X\n\tECX: %08X\n\tEDX: %08X\n\tEIP: %08X\n\tESP: %08X\n\tEBP: %08X\n\tESI: %08X\n\tEDI: %08X\n\n",
				tempDebugger.ProcessContext.Eax,
				tempDebugger.ProcessContext.Ebx,
				tempDebugger.ProcessContext.Ecx,
				tempDebugger.ProcessContext.Edx,
				tempDebugger.ProcessContext.Eip,
				tempDebugger.ProcessContext.Esp,
				tempDebugger.ProcessContext.Ebp,
				tempDebugger.ProcessContext.Esi,
				tempDebugger.ProcessContext.Edi		
			);
#endif
			tempDebugger.ShowCallStack(); // Call our registered callback
			getchar(); // Wait for User
			//tempDebugger.StepIn();
			tempDebugger.ResumeDebugging(); // Continue debugging
		}
	}
	getchar();

	OutputDebugString(L"------------- Before .class gets destroyed ------------\r\n");
	_CrtDumpMemoryLeaks();
	return 0;
}

int _tmain(int argc, PCHAR argv[])
{
	TheDebuggingFunction();
	
	// Test if the class leaked some memory after it finished.
	OutputDebugString(L"------------- After .class gets destroyed ------------\r\n");
	return _CrtDumpMemoryLeaks();
}