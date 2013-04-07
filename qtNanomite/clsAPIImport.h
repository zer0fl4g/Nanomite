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

	static bool LoadFunctions();
};

#endif