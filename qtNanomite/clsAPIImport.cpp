#include "clsAPIImport.h"
#include "clsHelperClass.h"

MyWow64GetThreadContext clsAPIImport::pWow64GetThreadContext = NULL;
MyWow64SetThreadContext clsAPIImport::pWow64SetThreadContext = NULL;
MyIsWow64Process clsAPIImport::pIsWow64Process = NULL;
MyNtQuerySystemInformation clsAPIImport::pNtQuerySystemInformation = NULL;
MyNtDuplicateObject clsAPIImport::pNtDuplicateObject = NULL;
MyNtQueryObject clsAPIImport::pNtQueryObject = NULL;
//MyRtlCreateQueryDebugBuffer clsAPIImport::pRtlCreateQueryDebugBuffer = NULL;
//MyRtlQueryProcessDebugInformation clsAPIImport::pRtlQueryProcessDebugInformation = NULL;
//MyRtlDestroyQueryDebugBuffer clsAPIImport::pRtlDestroyQueryDebugBuffer = NULL;

clsAPIImport::clsAPIImport()
{
}

clsAPIImport::~clsAPIImport()
{

}

bool clsAPIImport::LoadFunctions()
{
	PTCHAR szWarning = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
	bool bGotAFail = false;

	pIsWow64Process = (MyIsWow64Process)GetProcAddress(GetModuleHandle(L"Kernel32"),"IsWow64Process");
	pWow64GetThreadContext = (MyWow64GetThreadContext)GetProcAddress(GetModuleHandle(L"Kernel32"),"Wow64GetThreadContext");
	pWow64SetThreadContext = (MyWow64SetThreadContext)GetProcAddress(GetModuleHandle(L"Kernel32"),"Wow64SetThreadContext");
	pNtQuerySystemInformation = (MyNtQuerySystemInformation)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"NtQuerySystemInformation");
	pNtDuplicateObject = (MyNtDuplicateObject)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"NtDuplicateObject");
	pNtQueryObject = (MyNtQueryObject)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"NtQueryObject");
	//pRtlCreateQueryDebugBuffer = (MyRtlCreateQueryDebugBuffer)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"RtlCreateQueryDebugBuffer");
	//pRtlQueryProcessDebugInformation = (MyRtlQueryProcessDebugInformation)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"RtlQueryProcessDebugInformation");
	//pRtlDestroyQueryDebugBuffer = (MyRtlDestroyQueryDebugBuffer)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"RtlDestroyQueryDebugBuffer");

	wsprintf(szWarning,L"%s",L"There has been some Error while importing this APIs:\r\n");

	if(!pIsWow64Process)
	{
		bGotAFail = true;
		wcscat_s(szWarning,MAX_PATH,L"Kernel32.dll::IsWow64Process\r\n");
	}
	if(!pWow64GetThreadContext && !clsHelperClass::IsWindowsXP())
	{
		bGotAFail = true;
		wcscat_s(szWarning,MAX_PATH,L"Kernel32.dll::Wow64GetThreadContext\r\n");
	}
	if(!pWow64SetThreadContext && !clsHelperClass::IsWindowsXP())
	{
		bGotAFail = true;
		wcscat_s(szWarning,MAX_PATH,L"Kernel32.dll::Wow64SetThreadContext\r\n");
	}
	if(!pNtQuerySystemInformation)
	{
		bGotAFail = true;
		wcscat_s(szWarning,MAX_PATH,L"ntdll.dll::NtQuerySystemInformation\r\n");
	}
	if(!pNtQueryObject)
	{
		bGotAFail = true;
		wcscat_s(szWarning,MAX_PATH,L"ntdll.dll::NtDuplicateObject\r\n");
	}
	if(!pNtDuplicateObject)
	{
		bGotAFail = true;
		wcscat_s(szWarning,MAX_PATH,L"ntdll.dll::NtQueryObject\r\n");
	}

	if(bGotAFail)
	{
		wcscat_s(szWarning,MAX_PATH,L"\r\n\r\nThis can lead to unwanted errors while using the debugger!");
		MessageBoxW(NULL,szWarning,L"Nanomite",MB_OK);
	}

	free(szWarning);
	return !bGotAFail;
}