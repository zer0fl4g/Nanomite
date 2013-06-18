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
#ifndef NATIVEHEADER_H
#define NATIVEHEADER_H

#define NT_SUCCESS(x) ((x) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH 0xc0000004
#define SystemHandleInformation 16
#define ObjectBasicInformation 0
#define ObjectNameInformation 1
#define ObjectTypeInformation 2
#define PDI_MODULES                       0x01
#define PDI_BACKTRACE                     0x02
#define PDI_HEAPS                         0x04
#define PDI_HEAP_TAGS                     0x08
#define PDI_HEAP_BLOCKS                   0x10
#define PDI_LOCKS                         0x20

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

//typedef struct _DEBUG_BUFFER 
//{
//      HANDLE SectionHandle;
//      PVOID SectionBase;
//      PVOID RemoteSectionBase;
//      ULONG SectionBaseDelta;
//      HANDLE EventPairHandle;
//      ULONG Unknown[2];
//      HANDLE RemoteThreadHandle;
//      ULONG InfoClassMask;
//      ULONG SizeOfInfo;
//      ULONG AllocatedSize;
//      ULONG SectionSize;
//      PVOID ModuleInformation;
//      PVOID BackTraceInformation;
//      PVOID HeapInformation;
//      PVOID LockInformation;
//      PVOID Reserved[8];
//} DEBUG_BUFFER, *PDEBUG_BUFFER;
//
//typedef struct _DEBUG_HEAP_INFORMATION 
//{
//      ULONG Base; // 0x00
//      ULONG Flags; // 0x04
//      USHORT Granularity; // 0x08
//      USHORT Unknown; // 0x0A
//      ULONG Allocated; // 0x0C
//      ULONG Committed; // 0x10
//      ULONG TagCount; // 0x14
//      ULONG BlockCount; // 0x18
//      ULONG Reserved[7]; // 0x1C
//      PVOID Tags; // 0x38
//      PVOID Blocks; // 0x3C
//} DEBUG_HEAP_INFORMATION, *PDEBUG_HEAP_INFORMATION;
//
//struct HeapBlock
//{
//      PVOID dwAddress;
//      DWORD dwSize;
//      DWORD dwFlags;
//      ULONG reserved;
//};

typedef enum _THREAD_INFORMATION_CLASS {
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,
    ThreadIsIoPending,
    ThreadHideFromDebugger
} THREAD_INFORMATION_CLASS, *PTHREAD_INFORMATION_CLASS;

typedef enum _PROCESS_INFORMATION_CLASS {
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    MaxProcessInfoClass
} PROCESS_INFORMATION_CLASS, *PPROCESS_INFORMATION_CLASS;

//typedef ULONG KAFFINITY;  

typedef LONG KPRIORITY;   

typedef struct _CLIENT_ID    
{   
    HANDLE UniqueProcess;   
    HANDLE UniqueThread;   
} CLIENT_ID; 
   
typedef struct _THREAD_BASIC_INFORMATION    
{   
    DWORD       ExitStatus;    
    PVOID       TebBaseAddress;   
    CLIENT_ID   ClientId;    
    ULONG	    AffinityMask;    
    KPRIORITY   Priority;    
    KPRIORITY   BasePriority;   
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;  

typedef struct _PEB
{
	BYTE InheritedAddressSpace;
	BYTE ReadImageFileExecOptions;
	BYTE BeingDebugged;
	BYTE SpareBool;
	DWORD Mutant;
	DWORD ImageBaseAddress;
	DWORD LoaderData;
	DWORD ProcessParameters;
	DWORD SubSystemData;
	DWORD ProcessHeap;
	DWORD FastPebLock;
	DWORD FastPebLockRoutine;
	DWORD FastPebUnlockRoutine;
	DWORD EnviromentUpdateCount;
	DWORD KernelCallbackTable;
	DWORD UserSharedInfoPtr;
	DWORD ThunksOrOptions;
	DWORD FreeList;
	DWORD TlsExpansionCounter;
	DWORD TlsBitmap;
	DWORD TlsBitmapBits[2];
	DWORD ReadOnlySharedMemoryBase;
	DWORD ReadOnlySharedMemoryHeap;
	DWORD ReadOnlyStaticServerData;
	DWORD AnsiCodePageData;
	DWORD OemCodePageData;
	DWORD UnicodeCaseTableData;
	DWORD NumberOfProcessors;
	DWORD NtGlobalFlag;
	DWORD Reserved;
	LARGE_INTEGER CriticalSectionTimeout;
	DWORD HeapSegmentReserve;
	DWORD HeapSegmentCommit;
	DWORD HeapDeCommitTotalFreeThreshold;
	DWORD HeapDeCommitFreeBlockThreshold;
	DWORD NumberOfHeaps;
	DWORD MaximumNumberOfHeaps;
	DWORD ProcessHeaps;
	DWORD GdiSharedHandleTable;
	DWORD ProcessStarterHelper;
	DWORD GdiDCAttributeList;
	DWORD LoaderLock;
	DWORD OSMajorVersion;
	DWORD OSMinorVersion;
	WORD OSBuildNumber;
	WORD OSCSDVersion;
	DWORD OSPlatformId;
	DWORD ImageSubsystem;
	DWORD ImageSubsystemMajorVersion;
	DWORD ImageSubsystemMinorVersion;
	DWORD ImageProcessAffinityMask;
	DWORD GdiHandleBuffer[34];
	DWORD PostProcessInitRoutine;
	DWORD TlsExpansionBitmap;
	DWORD TlsExpansionBitmapBits[32];
	DWORD SessionId;
	ULARGE_INTEGER AppCompatFlags;
	ULARGE_INTEGER AppCompatFlagsUser;
	DWORD pShimData;
	DWORD AppCompatInfo;
	UNICODE_STRING CSDVersion;
	DWORD ActivationContextData;
	DWORD ProcessAssemblyStorageMap;
	DWORD SystemDefaultActivationContextData;
	DWORD SystemAssemblyStorageMap;
	DWORD MinimumStackCommit;
	DWORD FlsCallback;
	DWORD FlsListHead_Flink;
	DWORD FlsListHead_Blink;
	DWORD FlsBitmap;
	DWORD FlsBitmapBits[4];
	DWORD FlsHighIndex;
} PEB, *PPEB;
 
typedef struct _TEB {
	NT_TIB                  Tib;
	PVOID                   EnvironmentPointer;
	CLIENT_ID               Cid;
	PVOID                   ActiveRpcInfo;
	PVOID                   ThreadLocalStoragePointer;
	PPEB                    Peb;
	ULONG                   LastErrorValue;
	ULONG                   CountOfOwnedCriticalSections;
	PVOID                   CsrClientThread;
	PVOID                   Win32ThreadInfo;
	ULONG                   Win32ClientInfo[0x1F];
	PVOID                   WOW32Reserved;
	ULONG                   CurrentLocale;
	ULONG                   FpSoftwareStatusRegister;
	PVOID                   SystemReserved1[0x36];
	PVOID                   Spare1;
	ULONG                   ExceptionCode;
	ULONG                   SpareBytes1[0x28];
	PVOID                   SystemReserved2[0xA];
	ULONG                   GdiRgn;
	ULONG                   GdiPen;
	ULONG                   GdiBrush;
	CLIENT_ID               RealClientId;
	PVOID                   GdiCachedProcessHandle;
	ULONG                   GdiClientPID;
	ULONG                   GdiClientTID;
	PVOID                   GdiThreadLocaleInfo;
	PVOID                   UserReserved[5];
	PVOID                   GlDispatchTable[0x118];
	ULONG                   GlReserved1[0x1A];
	PVOID                   GlReserved2;
	PVOID                   GlSectionInfo;
	PVOID                   GlSection;
	PVOID                   GlTable;
	PVOID                   GlCurrentRC;
	PVOID                   GlContext;
	NTSTATUS                LastStatusValue;
	UNICODE_STRING          StaticUnicodeString;
	WCHAR                   StaticUnicodeBuffer[0x105];
	PVOID                   DeallocationStack;
	PVOID                   TlsSlots[0x40];
	LIST_ENTRY              TlsLinks;
	PVOID                   Vdm;
	PVOID                   ReservedForNtRpc;
	PVOID                   DbgSsReserved[0x2];
	ULONG                   HardErrorDisabled;
	PVOID                   Instrumentation[0x10];
	PVOID                   WinSockData;
	ULONG                   GdiBatchCount;
	ULONG                   Spare2;
	ULONG                   Spare3;
	ULONG                   Spare4;
	PVOID                   ReservedForOle;
	ULONG                   WaitingOnLoaderLock;
	PVOID                   StackCommit;
	PVOID                   StackCommitMax;
	PVOID                   StackReserved;
} TEB, *PTEB;

typedef struct _PROCESS_BASIC_INFORMATION
{
    LONG		ExitStatus;
    PPEB		PebBaseAddress;
    ULONG_PTR	AffinityMask;
    LONG		BasePriority;
    ULONG_PTR	UniqueProcessId;
    ULONG_PTR	InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

#endif