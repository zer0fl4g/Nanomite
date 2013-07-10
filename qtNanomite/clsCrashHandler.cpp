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
		pExceptionPtrs->ExceptionRecord->ExceptionCode == 0x000006A6 ||
		pExceptionPtrs->ExceptionRecord->ExceptionCode == 0x40010006
		)
		return EXCEPTION_EXECUTE_HANDLER;
	
	PTCHAR szCrashDumpName = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

	time_t tTime;
	tm timeInfo;
	time(&tTime);
	localtime_s(&timeInfo,&tTime);

	swprintf_s(szCrashDumpName,MAX_PATH,L"Nanomite_%d-%d_%d.%d_crash.dmp",
		timeInfo.tm_mday,
		timeInfo.tm_mon + 1,
		timeInfo.tm_hour,
		timeInfo.tm_min);

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