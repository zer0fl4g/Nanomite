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
#ifndef CLSAPIIMPORT
#define CLSAPIIMPORT

#include <Windows.h>

#include "NativeHeaders.h"

typedef BOOL (_stdcall *MyIsWow64Process) (HANDLE hProcess,PBOOL Wow64Process);
typedef BOOL (_stdcall *MyWow64GetThreadContext) (HANDLE hThread,PWOW64_CONTEXT lpContext);
typedef BOOL (_stdcall *MyWow64SetThreadContext) (HANDLE hThread,const WOW64_CONTEXT *lpContext);
typedef NTSTATUS (_stdcall *MyNtQuerySystemInformation) (ULONG SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength);
typedef NTSTATUS (_stdcall *MyNtDuplicateObject) (HANDLE SourceProcessHandle,HANDLE SourceHandle,HANDLE TargetProcessHandle,PHANDLE TargetHandle,ACCESS_MASK DesiredAccess,ULONG HandleAttributes,ULONG Options);
typedef NTSTATUS (_stdcall *MyNtQueryObject)(HANDLE Handle,ULONG ObjectInformationClass,PVOID ObjectInformation,ULONG ObjectInformationLength,PULONG ReturnLength);
typedef NTSTATUS (_stdcall *MyNtQueryInformationThread)(HANDLE ThreadHandle,ULONG ThreadInformationClass,PVOID ThreadInformation,ULONG ThreadInformationLength,PULONG ReturnLength);
typedef NTSTATUS (_stdcall *MyNtQueryInformationProcess)(HANDLE ProcessHandle,ULONG ProcessInformationClass,PVOID ProcessInformation,ULONG ProcessInformationLength,PULONG ReturnLength);
//typedef ULONG (_stdcall *MyRtlCreateQueryDebugBuffer)(ULONG Size,bool EventPair);
//typedef NTSTATUS (_stdcall *MyRtlQueryProcessDebugInformation)(ULONG ProcessId,ULONG DebugInfoClassMask,ULONG DebugBuffer);
//typedef NTSTATUS (_stdcall *MyRtlDestroyQueryDebugBuffer)(ULONG DebugBuffer);

class clsAPIImport
{
public:
	clsAPIImport();
	~clsAPIImport();

	static MyWow64GetThreadContext pWow64GetThreadContext;
	static MyWow64SetThreadContext pWow64SetThreadContext;
	static MyIsWow64Process pIsWow64Process;
	static MyNtQuerySystemInformation pNtQuerySystemInformation;
	static MyNtDuplicateObject pNtDuplicateObject;
	static MyNtQueryObject pNtQueryObject;
	static MyNtQueryInformationThread pNtQueryInformationThread;
	static MyNtQueryInformationProcess pNtQueryInformationProcess;
	//static MyRtlCreateQueryDebugBuffer pRtlCreateQueryDebugBuffer;
	//static MyRtlQueryProcessDebugInformation pRtlQueryProcessDebugInformation;
	//static MyRtlDestroyQueryDebugBuffer pRtlDestroyQueryDebugBuffer;

	static bool LoadFunctions();
};

#endif