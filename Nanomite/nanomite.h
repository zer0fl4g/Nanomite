#ifndef NanomiteH
#define NanomiteH

//#pragma pack(1)

#include <windows.h>
#include <WindowsX.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <CommCtrl.h>

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL

#include "BeaEngine.h"
#include "../clsDebugger/clsDebugger.h"
#include "Resource.h"

// Leak detection
#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>
// Leak detection

void LoadCallBacks();
void StartDebugging();
void DebuggingLoop(clsDebugger *newDebugger);
void LoadDisAssView(DWORD64 dwEIP);
void LoadRegView();
void LoadStackView(DWORD64 dwESP);
void UpdateStateLable(DWORD dwState);
void CleanUpGUI();

int OnThread(DWORD dwPID,DWORD dwTID,DWORD64 dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
int OnPID(DWORD dwPID,wstring sFile,DWORD dwExitCode,DWORD64 dwEP,bool bFound);
int OnException(wstring sFuncName,wstring sModName,DWORD64 dwOffset,DWORD64 dwExceptionCode,DWORD dwPID,DWORD dwTID);
int OnDbgString(wstring sMessage,DWORD dwPID);
int OnLog(tm timeInfo,wstring sLog);
int OnDll(wstring sDLLPath,DWORD dwPID,DWORD64 dwEP,bool bLoaded);
int OnCallStack(DWORD64 dwStackAddr,
			DWORD64 dwReturnTo,wstring sReturnToFunc,wstring sModuleName,
			DWORD64 dwEIP,wstring sFuncName,wstring sFuncModule,
			wstring sSourceFilePath,int iSourceLineNum);

bool MenuLoadNewFile();
bool WriteToSettingsFile();
bool ReadFromSettingsFile();
bool PrintMemToHexView(DWORD dwPID,DWORD dwOffset,DWORD dwSize,HWND hwHexView);
bool printPEView(HWND hWndDlg,int i);
bool InsertPEPartIntoLC(wstring sPEStructName,DWORD dwData,HWND hwLC);
bool InsertHandleIntoLC(HWND hwLC,DWORD dwPID,DWORD dwHandleID,PTCHAR sType,PTCHAR sName);
bool CALLBACK EnumWindowCallBack(HWND hWnd, LPARAM lParam);
BOOL CALLBACK EnumResTypes(HMODULE hModule,LPTSTR lpszType,LONG lParam);
BOOL CALLBACK EnumResNames(HMODULE hModule,LPCTSTR lpszType,LPTSTR lpszName,LONG lParam);

DWORD CalcNewOffset(DWORD dwCurrentOffset);

vector<wstring> split(const wstring& s,const wstring& f);

LRESULT CALLBACK MainDLGProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OptionDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BPManagerDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DetailInfoDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AttachToDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MemMapDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HeapMapDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AboutDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PeViewDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK StringViewDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndListDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HandleViewDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RessourceDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DebugStringDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

HMENU GenerateRunningPIDMenu(bool bWithAll);

struct EnumInfoStruct
{
	wstring sFileName;
	DWORD dwPID;
};


#define NT_SUCCESS(x) ((x) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH 0xc0000004

#define SystemHandleInformation 16
#define ObjectBasicInformation 0
#define ObjectNameInformation 1
#define ObjectTypeInformation 2

typedef NTSTATUS (NTAPI *_NtQuerySystemInformation)(
	ULONG SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength
	);
typedef NTSTATUS (NTAPI *_NtDuplicateObject)(
	HANDLE SourceProcessHandle,
	HANDLE SourceHandle,
	HANDLE TargetProcessHandle,
	PHANDLE TargetHandle,
	ACCESS_MASK DesiredAccess,
	ULONG Attributes,
	ULONG Options
	);
typedef NTSTATUS (NTAPI *_NtQueryObject)(
	HANDLE ObjectHandle,
	ULONG ObjectInformationClass,
	PVOID ObjectInformation,
	ULONG ObjectInformationLength,
	PULONG ReturnLength
	);

typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _SYSTEM_HANDLE
{
	ULONG ProcessId;
	BYTE ObjectTypeNumber;
	BYTE Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	ULONG HandleCount;
	SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef enum _POOL_TYPE
{
	NonPagedPool,
	PagedPool,
	NonPagedPoolMustSucceed,
	DontUseThisType,
	NonPagedPoolCacheAligned,
	PagedPoolCacheAligned,
	NonPagedPoolCacheAlignedMustS
} POOL_TYPE, *PPOOL_TYPE;

typedef struct _OBJECT_TYPE_INFORMATION
{
	UNICODE_STRING Name;
	ULONG TotalNumberOfObjects;
	ULONG TotalNumberOfHandles;
	ULONG TotalPagedPoolUsage;
	ULONG TotalNonPagedPoolUsage;
	ULONG TotalNamePoolUsage;
	ULONG TotalHandleTableUsage;
	ULONG HighWaterNumberOfObjects;
	ULONG HighWaterNumberOfHandles;
	ULONG HighWaterPagedPoolUsage;
	ULONG HighWaterNonPagedPoolUsage;
	ULONG HighWaterNamePoolUsage;
	ULONG HighWaterHandleTableUsage;
	ULONG InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	ULONG ValidAccess;
	BOOLEAN SecurityRequired;
	BOOLEAN MaintainHandleCount;
	USHORT MaintainTypeList;
	POOL_TYPE PoolType;
	ULONG PagedPoolUsage;
	ULONG NonPagedPoolUsage;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

#endif 