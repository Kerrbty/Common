#pragma once
#ifndef _HHCHINA_MNAME_H_NTDLL_H_
#define _HHCHINA_MNAME_H_NTDLL_H_

#include <Windows.h>
#include "commondef.h"
#define __WINESRC__

//////////////////////////////////////////////////////////////////////////
//  包含的lib文件及其他一些编译选项
//////////////////////////////////////////////////////////////////////////

#pragma comment(lib, "Version")
#pragma comment(lib, "shell32")
#pragma comment(lib, "imagehlp")
#pragma comment(lib, "psapi")
#pragma comment(lib, "advapi32")

#ifdef __WINESRC__ // WIN32

#pragma comment(lib, "../CommonFile/ntdll_i386")

#elif WIN64

#pragma comment(lib, "../CommonFile/ntdll_amd64")
#pragma comment(lib, "../CommonFile/ntdllp_amd64")

#else

#pragma comment(lib, "../CommonFile/ntdll_ia64")
#pragma comment(lib, "../CommonFile/ntdllp_ia64")

#endif




//////////////////////////////////////////////////////////////////////////   
//  structs 使用的结构体头
//////////////////////////////////////////////////////////////////////////

typedef struct _SYSTEM_MODULE_INFORMATION { 
	ULONG Reserved[2]; 
	PVOID Base;   // 内存加载基址
	ULONG Size;   // 占用内存大小
	ULONG Flags; 
	USHORT Index; 
	USHORT Unknown; 
	USHORT LoadCount; 
	USHORT ModuleNameOffset; 
	CHAR ImageName[256]; 
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct _STRING {
	USHORT  Length;
	USHORT  MaximumLength;
	PCHAR  Buffer;
} ANSI_STRING, *PANSI_STRING;

typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _VM_COUNTERS_ {
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG  PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
} VM_COUNTERS, *PVM_COUNTERS;

typedef struct _CLIENT_ID
{
	HANDLE PID;
	HANDLE ThreadId;
}CLIENT_ID, *PCLIENT_ID;

typedef struct _SYSTEM_THREAD_INFORMATION
{                                    /* win32/win64 */
	LARGE_INTEGER KernelTime;          /* 00/00 */
	LARGE_INTEGER UserTime;            /* 08/08 */
	LARGE_INTEGER CreateTime;          /* 10/10 */
	DWORD         dwTickCount;         /* 18/18 */
	LPVOID        StartAddress;        /* 1c/20 */
	CLIENT_ID     ClientId;            /* 20/28 */
	DWORD         dwCurrentPriority;   /* 28/38 */
	DWORD         dwBasePriority;      /* 2c/3c */
	DWORD         dwContextSwitches;   /* 30/40 */
	DWORD         dwThreadState;       /* 34/44 */
	DWORD         dwWaitReason;        /* 38/48 */
	DWORD         dwUnknown;           /* 3c/4c */
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
#ifdef __WINESRC__                  /* win32/win64 */  // ifdef WIN32
	ULONG NextEntryOffset;             /* 00/00 */
	DWORD dwThreadCount;               /* 04/04 */
	DWORD dwUnknown1[6];               /* 08/08 */
	LARGE_INTEGER CreationTime;        /* 20/20 */
	LARGE_INTEGER UserTime;            /* 28/28 */
	LARGE_INTEGER KernelTime;          /* 30/30 */
	UNICODE_STRING ProcessName;        /* 38/38 */
	DWORD dwBasePriority;              /* 40/48 */
	HANDLE UniqueProcessId;            /* 44/50 */
	HANDLE ParentProcessId;            /* 48/58 */
	ULONG HandleCount;                 /* 4c/60 */
	DWORD dwUnknown3;                  /* 50/64 */
	DWORD dwUnknown4;                  /* 54/68 */
	VM_COUNTERS vmCounters;            /* 58/70 */
	IO_COUNTERS ioCounters;            /* 88/d0 */
	SYSTEM_THREAD_INFORMATION ti[1];   /* b8/100 */
#else
	ULONG NextEntryOffset;             /* 00/00 */
	BYTE Reserved1[52];                /* 04/04 */
	PVOID Reserved2[3];                /* 38/38 */
	HANDLE UniqueProcessId;            /* 44/50 */
	PVOID Reserved3;                   /* 48/58 */
	ULONG HandleCount;                 /* 4c/60 */
	BYTE Reserved4[4];                 /* 50/64 */
	PVOID Reserved5[11];               /* 54/68 */
	SIZE_T PeakPagefileUsage;          /* 80/c0 */
	SIZE_T PrivatePageCount;           /* 84/c8 */
	LARGE_INTEGER Reserved6[6];        /* 88/d0 */
#endif
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;


// 驱动信息
typedef struct _QueryDriverModuleInfo{
	DWORD dwSize;  // Module Number
	SYSTEM_MODULE_INFORMATION SysModuleInfo[1];
}QueryDriverModuleInfo, * PQueryDriverModuleInfo;


// 进程信息
typedef struct _QueryProcessModuleInfo{
	DWORD dwSize;  // Module Number
	SYSTEM_PROCESS_INFORMATION SysModuleInfo[1];
}QueryProcessModuleInfo, * PQueryProcessModuleInfo;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation = 0,
	SystemCpuInformation = 1,
	SystemPerformanceInformation = 2,
	SystemTimeOfDayInformation = 3, /* was SystemTimeInformation */
	Unknown4,
	SystemProcessInformation = 5,
	Unknown6,
	Unknown7,
	SystemProcessorPerformanceInformation = 8,
	Unknown9,
	Unknown10,
	SystemModuleInformation = 11,
	Unknown12,
	Unknown13,
	Unknown14,
	Unknown15,
	SystemHandleInformation = 16,
	Unknown17,
	SystemPageFileInformation = 18,
	Unknown19,
	Unknown20,
	SystemCacheInformation = 21,
	Unknown22,
	SystemInterruptInformation = 23,
	SystemDpcBehaviourInformation = 24,
	SystemFullMemoryInformation = 25,
	SystemNotImplemented6 = 25,
	SystemLoadImage = 26,
	SystemUnloadImage = 27,
	SystemTimeAdjustmentInformation = 28,
	SystemTimeAdjustment = 28,
	SystemSummaryMemoryInformation = 29,
	SystemNotImplemented7 = 29,
	SystemNextEventIdInformation = 30,
	SystemNotImplemented8 = 30,
	SystemEventIdsInformation = 31,
	SystemCrashDumpInformation = 32,
	SystemExceptionInformation = 33,
	SystemCrashDumpStateInformation = 34,
	SystemKernelDebuggerInformation = 35,
	SystemContextSwitchInformation = 36,
	SystemRegistryQuotaInformation = 37,
	SystemCurrentTimeZoneInformation = 44,
	SystemTimeZoneInformation = 44,
	SystemLookasideInformation = 45,
	SystemSetTimeSlipEvent = 46,
	SystemCreateSession = 47,
	SystemDeleteSession = 48,
	SystemInvalidInfoClass4 = 49,
	SystemRangeStartInformation = 50,
	SystemVerifierInformation = 51,
	SystemAddVerifier = 52,
	SystemSessionProcessesInformation	= 53,
	SystemInformationClassMax
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS  Status;
		PVOID  Pointer;
	};
	ULONG_PTR  Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef enum _FILE_INFORMATION_CLASS {
	FileDirectoryInformation = 1,
	FileFullDirectoryInformation,
	FileBothDirectoryInformation,
	FileBasicInformation,
	FileStandardInformation,
	FileInternalInformation,
	FileEaInformation,
	FileAccessInformation,
	FileNameInformation,
	FileRenameInformation,
	FileLinkInformation,
	FileNamesInformation,
	FileDispositionInformation,
	FilePositionInformation,
	FileFullEaInformation,
	FileModeInformation,
	FileAlignmentInformation,
	FileAllInformation,
	FileAllocationInformation,
	FileEndOfFileInformation,
	FileAlternateNameInformation,
	FileStreamInformation,
	FilePipeInformation,
	FilePipeLocalInformation,
	FilePipeRemoteInformation,
	FileMailslotQueryInformation,
	FileMailslotSetInformation,
	FileCompressionInformation,
	FileObjectIdInformation,
	FileCompletionInformation,
	FileMoveClusterInformation,
	FileQuotaInformation,
	FileReparsePointInformation,
	FileNetworkOpenInformation,
	FileAttributeTagInformation,
	FileTrackingInformation,
	FileIdBothDirectoryInformation,
	FileIdFullDirectoryInformation,
	FileValidDataLengthInformation,
	FileShortNameInformation,
#if (NTDDI_VERSION >= NTDDI_VISTA)
	FileIoCompletionNotificationInformation,
	FileIoStatusBlockRangeInformation,
	FileIoPriorityHintInformation,
	FileSfioReserveInformation,
	FileSfioVolumeInformation,
	FileHardLinkInformation,
	FileProcessIdsUsingFileInformation,
	FileNormalizedNameInformation,
	FileNetworkPhysicalNameInformation,
#endif
#if (NTDDI_VERSION >= NTDDI_WIN7)
	FileIdGlobalTxDirectoryInformation,
	FileIsRemoteDeviceInformation,
	FileAttributeCacheInformation,
	FileNumaNodeInformation,
	FileStandardLinkInformation,
	FileRemoteProtocolInformation,
#endif
	FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

typedef enum _OBJECT_INFORMATION_CLASS {
	ObjectBasicInformation = 0,
	ObjectTypeInformation = 2,
} OBJECT_INFORMATION_CLASS;

typedef struct _FILE_NAME_INFORMATION {
	ULONG  FileNameLength;
	WCHAR  FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

typedef struct _PUBLIC_OBJECT_TYPE_INFORMATION {
	UNICODE_STRING  TypeName;
	ULONG  Reserved[22];
} PUBLIC_OBJECT_TYPE_INFORMATION,* PPUBLIC_OBJECT_TYPE_INFORMATION;


//////////////////////////////////////////////////////////////////////////
// Native API part
//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"{
#endif

NTSTATUS NTAPI
ZwQuerySystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
		OUT PVOID SystemInformation,
		IN ULONG Length,
		OUT PULONG UnsafeResultLength);


typedef VOID *PRELATIVE_NAME; // 这定义是错误的
NTSTATUS NTAPI 
RtlDosPathNameToNtPathName_U(
	IN PCWSTR DosPathName,
	OUT PUNICODE_STRING NtPathName,
	OUT PWSTR* FilePathInNtPathName OPTIONAL,
	OUT PRELATIVE_NAME* RelativeName OPTIONAL
	);

NTSTATUS NTAPI 
RtlNtPathNameToDosPathName(
	IN ULONG Flags,
	INOUT PUNICODE_STRING NtPathName,
	OUT DWORD Disposition,
	OUT PWSTR* FilePath
	);
NTSTATUS NTAPI
ZwQueryInformationFile(
	IN HANDLE  FileHandle,
	OUT PIO_STATUS_BLOCK  IoStatusBlock,
	OUT PVOID  FileInformation,
	IN ULONG  Length,
	IN FILE_INFORMATION_CLASS  FileInformationClass
	);

NTSTATUS NTAPI
ZwQueryObject(
	__in_opt HANDLE  Handle,
	__in OBJECT_INFORMATION_CLASS  ObjectInformationClass,
	__out_bcount_opt(ObjectInformationLength) PVOID  ObjectInformation,
	__in ULONG  ObjectInformationLength,
	__out_opt PULONG  ReturnLength
	);
ULONG NTAPI
RtlNtStatusToDosError(
	IN NTSTATUS Status);


NTSTATUS NTAPI
RtlAdjustPrivilege(
	ULONG	Privilege, 
	BOOLEAN Enable, 
	BOOLEAN CurrentThread, 
	PBOOLEAN Enabled
	);

NTSTATUS NTAPI RtlCreateUserThread(
		IN HANDLE Process,
		IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
		IN BOOLEAN CreateSuspended,
		IN ULONG ZeroBits OPTIONAL,
		IN SIZE_T MaximumStackSize OPTIONAL,
		IN SIZE_T CommittedStackSize OPTIONAL,
		IN LPTHREAD_START_ROUTINE StartAddress,
		IN PVOID Parameter OPTIONAL,
		OUT PHANDLE Thread OPTIONAL,
		OUT PCLIENT_ID ClientId OPTIONAL
);

typedef NTSTATUS (NTAPI * _NtCreateThreadEx_)(
		OUT PHANDLE hThread,
		IN ACCESS_MASK DesiredAccess,
		IN LPVOID ObjectAttributes,
		IN HANDLE ProcessHandle,
		IN LPTHREAD_START_ROUTINE lpStartAddress,
		IN LPVOID lpParameter,
		IN BOOL CreateSuspended, 
		IN ULONG StackZeroBits,
		IN ULONG SizeOfStackCommit,
		IN ULONG SizeOfStackReserve,
		OUT LPVOID lpBytesBuffer
);






#ifdef __cplusplus
};
#endif



#endif // _HHCHINA_MNAME_H_NTDLL_H_