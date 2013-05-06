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

#endif