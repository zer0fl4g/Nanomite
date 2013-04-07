#include "clsAPIImport.h"

MyWow64GetThreadContext clsAPIImport::pWow64GetThreadContext = NULL;
MyWow64SetThreadContext clsAPIImport::pWow64SetThreadContext = NULL;
MyIsWow64Process clsAPIImport::pIsWow64Process = NULL;
MyNtQuerySystemInformation clsAPIImport::pNtQuerySystemInformation = NULL;
MyNtDuplicateObject clsAPIImport::pNtDuplicateObject = NULL;
MyNtQueryObject clsAPIImport::pNtQueryObject = NULL;

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
	pNtQuerySystemInformation = (MyNtQuerySystemInformation)GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtQuerySystemInformation");
	pNtDuplicateObject = (MyNtDuplicateObject)GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtDuplicateObject");
	pNtQueryObject = (MyNtQueryObject)GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtQueryObject");

	if(!pIsWow64Process &&
		!pWow64GetThreadContext && 
		!pWow64SetThreadContext && 
		!pNtQuerySystemInformation &&
		!pNtQueryObject &&
		!pNtDuplicateObject)
		return false;
	return true;
}