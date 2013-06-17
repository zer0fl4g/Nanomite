// Debugme.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <Windows.h>

int RetTest()
{
	RaiseException(0xDEADBEEF,NULL,NULL,NULL);
	OutputDebugString(L"DbgMsg from Sup");
	return -1337;
}

int _stdcall TestThread(LPVOID pData)
{
	MessageBoxW(NULL,L"Text from Thread",NULL,MB_OK);

	return 0;
}

int _tmain(int argc,PCHAR argv[])
{
	if(argc > 0)
		MessageBoxW(NULL,GetCommandLine(),L"MessageBoxFromCMD",MB_OK);
	OutputDebugString(L"This is a DBG");
	//__asm { int 3 };
	RaiseException(0x1337,NULL,NULL,NULL);
	//RetTest();
	if(IsDebuggerPresent())
		MessageBoxW(NULL,L"Debugger Found",L"Debugme!",MB_OK);
	else
		MessageBoxW(NULL,L"Debugger NOT Found",L"Debugme!",MB_OK);
	
	HANDLE hThread = CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)TestThread,NULL,NULL,NULL);
	WaitForSingleObject(hThread,INFINITE);	

	//for(int i = 0; i < 99; i++)
	//	OutputDebugStringW(L"Little Test!");
	return 0;
}

