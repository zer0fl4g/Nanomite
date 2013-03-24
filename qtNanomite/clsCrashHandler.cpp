#include "clsCrashHandler.h"

#include <dbghelp.h>
#include <stdio.h>
#include <time.h>

LONG CALLBACK clsCrashHandler::ErrorReporter(PEXCEPTION_POINTERS pExceptionPtrs)
{
	// Windows RPC Exception, happens on OpenFile Dialog, seems to come only on > XP and can be ignored
	// 0x406d1388 = SetThreadName Exception from cruntime, ignore!
	// 0x40010006 = STRG + C from cruntime, ignore!
	if(pExceptionPtrs->ExceptionRecord->ExceptionCode == 0x000006ba ||
		pExceptionPtrs->ExceptionRecord->ExceptionCode == 0x406d1388 ||
		pExceptionPtrs->ExceptionRecord->ExceptionCode == 0xE0000001 ||
		pExceptionPtrs->ExceptionRecord->ExceptionCode == 0x40010006)
		return EXCEPTION_EXECUTE_HANDLER;
	
	PTCHAR szCrashDumpName = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

	time_t tTime;
	tm* timeInfo;
	time(&tTime);
	timeInfo = localtime(&tTime);

	swprintf_s(szCrashDumpName,MAX_PATH,L"Nanomite_%d-%d_%d.%d_crash.dmp",
		timeInfo->tm_mday,
		timeInfo->tm_mon + 1,
		timeInfo->tm_hour,
		timeInfo->tm_min);

	HANDLE hFile = CreateFile(szCrashDumpName, GENERIC_READ | GENERIC_WRITE, NULL,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 

	if(hFile != INVALID_HANDLE_VALUE) 
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei; 

		mdei.ThreadId           = GetCurrentThreadId(); 
		mdei.ExceptionPointers  = pExceptionPtrs; 
		mdei.ClientPointers     = TRUE; 

		MINIDUMP_TYPE dumpTyp   = (MINIDUMP_TYPE)(MiniDumpWithFullMemoryInfo	| 
		                                          MiniDumpWithHandleData		| 
		                                          MiniDumpWithThreadInfo		| 
		                                          MiniDumpWithUnloadedModules	); 

		if(MiniDumpWriteDump(GetCurrentProcess(),GetCurrentProcessId(),hFile, dumpTyp, &mdei, NULL, NULL))
			MessageBoxW(NULL,L"Please send the dumpfile to zer0fl4g@gmail.com",L"Nanomite crashed!",MB_OK);
		else
			MessageBoxW(NULL,L"Nanomite crashed and was unable to write a dumpfile!",L"Nanomite crashed!",MB_OK);

		CloseHandle(hFile); 
	}

	free(szCrashDumpName);

	if(IsDebuggerPresent())
		return EXCEPTION_EXECUTE_HANDLER;
	else
		return TerminateProcess(GetCurrentProcess(),1);
}