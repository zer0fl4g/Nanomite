// Debugme.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <Windows.h>

int RetTest()
{
	RaiseException(0xDEADBEEF,NULL,NULL,NULL);
	OutputDebugString(L"DbgMsg from Sup");
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
	
	MessageBoxW(NULL,L"TEXT",NULL,MB_OK);

	//for(int i = 0; i < 99; i++)
	//	OutputDebugStringW(L"Little Test!");
	return 0;
}

