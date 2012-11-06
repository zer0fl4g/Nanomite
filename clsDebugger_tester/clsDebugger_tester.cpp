#include "../clsDebugger/clsDebugger.h"

// Leak detection
#include <stdlib.h>
#include <crtdbg.h>

#define _CRTDBG_MAP_ALLOC
// Leak detection

// Our callback function which gets triggered if log gets updated
int OnLog(tm timeInfo,wstring sLog)
{
	wprintf(L"%s\n",sLog.c_str()); // Just print the new string to console
	return 0;
}

// Our callback function which gets triggered on "clsDebugger.ShowCallstack()"
int OnCallStack(DWORD dwStackAddr,
	DWORD dwReturnTo,wstring sReturnToFunc,wstring sReturnToModuleName,
	DWORD dwEIP,wstring sFuncName,wstring sFuncModule,
	wstring sSourceFilePath,int iSourceLineNum)
{
	wprintf(L"\nCurrent:\t%08X - %s.%s\nReturnTo:\t%08X - %s.%s\n",dwEIP,sFuncModule.c_str(),sFuncName.c_str(),dwReturnTo,sReturnToModuleName.c_str(),sReturnToFunc.c_str());
	return 0;
}

// Our callback for a 0x1337 exception
// needs to be a stdcall else stack gets corrupted
DWORD __stdcall On1337Exception(DEBUG_EVENT *debug_event)
{
	MessageBoxW(NULL,L"You got a 1337 exception!",L"Nanomite!",MB_OK);
	return DBG_CONTINUE;
}

int _tmain(int argc, PCHAR argv[])
{
	bool bDebugIt = true;

	/*
	| Creating new Instance with a Target
	| Without parameter for Attaching to a running Process ( or set it manual later with .SetTarget(<path>))
	*/
	clsDebugger tempDebugger(L"C:\\Dropbox\\Projects\\clsDebugger\\Debug\\Debugme.exe");
	//clsDebugger tempDebugger(L"C:\\Program Files\\Microsoft Office\\Office14\\WINWORD.EXE");
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

	/*
	| Enable auto load of symbols
	| Note: Disable may cause missing function names in callback/exception info but target will load faster
	*/
	tempDebugger.dbgSettings.bAutoLoadSymbols = true;

	/* Set Callbacks 
	| int (*dwOnThread)(DWORD dwPID,DWORD dwTID,DWORD dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
	| int (*dwOnPID)(DWORD dwPID,wstring sFile,DWORD dwExitCode,DWORD dwEP,bool bFound);
	| int (*dwOnException)(wstring sFuncName,string sModName,DWORD dwOffset,DWORD dwExceptionCode,DWORD dwPID,DWORD dwTID);
	| int (*dwOnDbgString)(wstring sMessage,DWORD dwPID);
	| int (*dwOnLog)(tm timeInfo,wstring sLog);
	| int (*dwOnDll)(wstring sDLLPath,DWORD dwPID,DWORD dwEP,bool bLoaded);
	| int (*dwOnCallStack)(DWORD dwStackAddr,DWORD dwReturnTo,wstring sReturnToFunc,wstring sModuleName,DWORD dwEIP,wstring sFuncName,wstring sFuncModule,wstring sSourceFilePath,int iSourceLineNum);
	*/
	tempDebugger.dwOnCallStack = &OnCallStack;
	tempDebugger.dwOnLog = &OnLog; 
	
	/* Set manual BPs
	| dwType:
	|		0 = SoftwareBP
	|	    1 = MemoryBP
	|		2 = HardwareBP 
	|
	| dwPID: ProcessID where you want to place the BP ( must be a process that is debugged )
	|	     Can be -1 for mother process if you have different child processes
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
	*/
	tempDebugger.AddBreakpointToList(0,-1,(DWORD)GetProcAddress(LoadLibrary(L"Kernel32.dll"),"OutputDebugStringW"),0,true);
	tempDebugger.AddBreakpointToList(2,-1,(DWORD)GetProcAddress(LoadLibrary(L"User32.dll"),"MessageBoxW"),0,true);

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
	tempDebugger.CustomExceptionAdd(0x1337,0,(DWORD)&On1337Exception);
	tempDebugger.CustomExceptionAdd(EXCEPTION_ACCESS_VIOLATION,1,NULL);
	tempDebugger.CustomExceptionAdd(EXCEPTION_ILLEGAL_INSTRUCTION,1,NULL);
	tempDebugger.CustomExceptionAdd(EXCEPTION_PRIV_INSTRUCTION,1,NULL);


	// Run Debugging
	tempDebugger.StartDebugging();
	// For attaching to a running Process we need first to call .AttachToProcess(PID) and then .StartDebugging()
	//tempDebugger.AttachToProcess(5800);
	//tempDebugger.StartDebugging();

	// Our debugging loop which handles the events
	// There you can do all things you want to have be done when the targets breaks cause of a error / manual suspend
	while(bDebugIt)
	{
		WaitForSingleObject(tempDebugger.hDebuggingHandle,INFINITE); // Waiting for Debugevent

		if(!tempDebugger.GetDebuggingState()) // Check if debugging is still running
			bDebugIt = false; // if not just quit
		else
		{
			// if still running print CPU registers
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
			tempDebugger.ShowCallStack(); // Call our registered callback
			getchar(); // Wait for User
			//tempDebugger.StepIn();
			tempDebugger.ResumeDebugging(); // Continue debugging
		}
	}
	getchar();
	_CrtDumpMemoryLeaks();
	return 0;
}