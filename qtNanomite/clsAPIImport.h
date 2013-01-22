#ifndef CLSAPIIMPORT
#define CLSAPIIMPORT

#include <Windows.h>

typedef BOOL (_stdcall *MyIsWow64Process) (HANDLE hProcess,PBOOL Wow64Process);
typedef BOOL (_stdcall *MyWow64GetThreadContext) (HANDLE hThread,PWOW64_CONTEXT lpContext);
typedef BOOL (_stdcall *MyWow64SetThreadContext) (HANDLE hThread,const WOW64_CONTEXT *lpContext);

class clsAPIImport
{
public:
	clsAPIImport();
	~clsAPIImport();

	static MyWow64GetThreadContext pWow64GetThreadContext;
	static MyWow64SetThreadContext pWow64SetThreadContext;
	static MyIsWow64Process pIsWow64Process;
	
	static bool LoadFunctions();
};

#endif