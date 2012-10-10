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
	OutputDebugString(L"This is a DBG");
	//__asm { int 3 };

	//RetTest();
	if(IsDebuggerPresent())
		MessageBoxW(NULL,L"Debugger Found",L"Debugme!",MB_OK);
	else
		MessageBoxW(NULL,L"Debugger NOT Found",L"Debugme!",MB_OK);
	
	MessageBoxW(NULL,L"TEXT",NULL,MB_OK);
	return 0;
}

