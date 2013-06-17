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
#include "clsAPIImport.h"
#include "clsMemManager.h"
#include "clsHelperClass.h"

MyWow64GetThreadContext clsAPIImport::pWow64GetThreadContext = NULL;
MyWow64SetThreadContext clsAPIImport::pWow64SetThreadContext = NULL;
MyIsWow64Process clsAPIImport::pIsWow64Process = NULL;
MyNtQuerySystemInformation clsAPIImport::pNtQuerySystemInformation = NULL;
MyNtDuplicateObject clsAPIImport::pNtDuplicateObject = NULL;
MyNtQueryObject clsAPIImport::pNtQueryObject = NULL;
MyNtQueryInformationThread clsAPIImport::pNtQueryInformationThread = NULL;
MyNtQueryInformationProcess clsAPIImport::pNtQueryInformationProcess = NULL;
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
	PTCHAR szWarning = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	bool bGotAFail = false;

#ifdef _AMD64_
	pIsWow64Process = (MyIsWow64Process)GetProcAddress(GetModuleHandle(L"Kernel32"),"IsWow64Process");
	pWow64GetThreadContext = (MyWow64GetThreadContext)GetProcAddress(GetModuleHandle(L"Kernel32"),"Wow64GetThreadContext");
	pWow64SetThreadContext = (MyWow64SetThreadContext)GetProcAddress(GetModuleHandle(L"Kernel32"),"Wow64SetThreadContext");
#endif
	pNtQuerySystemInformation = (MyNtQuerySystemInformation)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"NtQuerySystemInformation");
	pNtDuplicateObject = (MyNtDuplicateObject)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"NtDuplicateObject");
	pNtQueryObject = (MyNtQueryObject)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"NtQueryObject");
	pNtQueryInformationThread = (MyNtQueryInformationThread)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"NtQueryInformationThread");
	pNtQueryInformationProcess = (MyNtQueryInformationProcess)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"NtQueryInformationProcess");
	//pRtlCreateQueryDebugBuffer = (MyRtlCreateQueryDebugBuffer)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"RtlCreateQueryDebugBuffer");
	//pRtlQueryProcessDebugInformation = (MyRtlQueryProcessDebugInformation)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"RtlQueryProcessDebugInformation");
	//pRtlDestroyQueryDebugBuffer = (MyRtlDestroyQueryDebugBuffer)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"RtlDestroyQueryDebugBuffer");

	wsprintf(szWarning,L"%s",L"There has been some Error while importing this APIs:\r\n");

#ifdef _AMD64_
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
#endif
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
	if(!pNtQueryInformationThread)
	{
		bGotAFail = true;
		wcscat_s(szWarning,MAX_PATH,L"ntdll.dll::NtQueryInformationThread\r\n");
	}
	if(!pNtQueryInformationProcess)
	{
		bGotAFail = true;
		wcscat_s(szWarning,MAX_PATH,L"ntdll.dll::NtQueryInformationProcess\r\n");
	}

	if(bGotAFail)
	{
		wcscat_s(szWarning,MAX_PATH,L"\r\n\r\nThis can lead to unwanted errors while using the debugger!");
		MessageBoxW(NULL,szWarning,L"Nanomite",MB_OK);
	}

	clsMemManager::CFree(szWarning);
	return !bGotAFail;
}