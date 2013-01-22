#include "clsAPIImport.h"

MyWow64GetThreadContext clsAPIImport::pWow64GetThreadContext = NULL;
MyWow64SetThreadContext clsAPIImport::pWow64SetThreadContext = NULL;
MyIsWow64Process clsAPIImport::pIsWow64Process = NULL;

clsAPIImport::clsAPIImport()
{
}

clsAPIImport::~clsAPIImport()
{

}

bool clsAPIImport::LoadFunctions()
{
	pIsWow64Process = (MyIsWow64Process)GetProcAddress(GetModuleHandle(L"Kernel32"),"IsWow64Process");
	pWow64GetThreadContext = (MyWow64GetThreadContext)GetProcAddress(GetModuleHandle(L"Kernel32"),"Wow64GetThreadContext");
	pWow64SetThreadContext = (MyWow64SetThreadContext)GetProcAddress(GetModuleHandle(L"Kernel32"),"Wow64SetThreadContext");
	
	if(!pIsWow64Process &&
		!pWow64GetThreadContext && 
		!pWow64SetThreadContext)
		return false;
	return true;
}