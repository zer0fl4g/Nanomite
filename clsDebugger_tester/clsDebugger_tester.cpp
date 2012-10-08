#include "../clsDebugger/clsDebugger.h"

// Leak detection
#include <stdlib.h>
#include <crtdbg.h>

#define _CRTDBG_MAP_ALLOC
// Leak detection

// Our callback function which gets triggered if log gets updated
int OnLog(wstring sLog)
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
	wprintf(L"\nCurrent:\t%08X - %s.%s\nReturnTo:\t%08X - %s.%s\n",dwEIP,sFuncName.c_str(),sFuncModule.c_str(),dwReturnTo,sReturnToFunc.c_str(),sReturnToModuleName.c_str());
	return 0;
}

int _tmain(int argc, PCHAR argv[])
{
	bool bDebugIt = true;

	/*
	| Creating new Instance with a Target
	| Without parameter for Attaching to a running Process ( or set it manual later with .SetTarget(<path>))
	*/
	//clsDebugger tempDebugger(L"C:\\Dropbox\\Projects\\clsDebugger\\Debug\\Debugme.exe");
	clsDebugger tempDebugger(L"C:\\Program Files\\Microsoft Office\\Office14\\WINWORD.EXE");
	//clsDebugger tempDebugger; // needs manual set of target with .SetTarget(<path>)

	 // Setting Option to enable child process debugging
	tempDebugger.dbgSettings.bDebugChilds = true;

	/* BreakOn: 
	| 0 = default application EP
	| 1 = SystemEP 
	| 2 = TLSCallback // Not working right now
	| 3 = Direkt run
	*/
	tempDebugger.dbgSettings.dwBreakOnEPMode = 0;

	/* Settings for Exception Handling
	| 0 = Break and wait for User
	| 1 = pass to App/Os
	| 2 = Ignore
	| 3 = automatic workaround ( jumping to the return addr which is on the stack )
	*/
	tempDebugger.dbgSettings.dwEXCEPTION_ACCESS_VIOLATION = 1;
	tempDebugger.dbgSettings.dwEXCEPTION_ILLEGAL_INSTRUCTION = 1;
	tempDebugger.dbgSettings.dwEXCEPTION_PRIV_INSTRUCTION = 1;
	tempDebugger.dbgSettings.dwEXCEPTION_BREAKPOINT = 0; // Does not affect Breakpoints ( Attaching INT3 will be ignored and program runs direct after attaching )

	/*
	| Enable autoload of symbols
	| Note: Disable may cause missing function names in callback/exception info but target will load faster
	*/
	tempDebugger.dbgSettings.bAutoLoadSymbols = true;

	/* Set Callbacks 
	| int (*dwOnThread)(DWORD dwPID,DWORD dwTID,DWORD dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
	| int (*dwOnPID)(DWORD dwPID,wstring sFile,DWORD dwExitCode,DWORD dwEP,bool bFound);
	| int (*dwOnException)(wstring sFuncName,string sModName,DWORD dwOffset,DWORD dwExceptionCode,DWORD dwPID,DWORD dwTID);
	| int (*dwOnDbgString)(wstring sMessage,DWORD dwPID);
	| int (*dwOnLog)(wstring sLog);
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
	|	Note:
	|		If you don´t know the PID ( check public PIDs vector for all running Processes in the debugger ) 
	|		use -1 to set BP in "Mother" process
	*/
	tempDebugger.AddBreakpointToList(0,-1,(DWORD)GetProcAddress(LoadLibrary(L"Kernel32.dll"),"OutputDebugStringA"),true);
	tempDebugger.AddBreakpointToList(0,-1,(DWORD)GetProcAddress(LoadLibrary(L"User32.dll"),"MessageBoxA"),true);

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
			tempDebugger.ResumeDebugging(); // Continue debugging
		}
	}
	getchar();
	_CrtDumpMemoryLeaks();
	return 0;
}