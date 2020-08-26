#ifndef _NATIVE_API_NTDLL_H_
#define _NATIVE_API_NTDLL_H_
#include <Windows.h>
#include <tchar.h>

#pragma comment(lib, "ntdll.lib")

typedef LONG NTSTATUS, *PNTSTATUS;
typedef LONG KPRIORITY;

#if _MSC_VER < 1300 
typedef unsigned long  ULONG, *ULONG_PTR;
typedef ULONG  ACCESS_MASK;
#endif


#ifndef _in
#define _in
#endif

#ifndef _out
#define _out
#endif

#ifndef _inout
#define _inout
#endif 

#ifndef _in_opt
#define _in_opt
#endif

#ifndef _out_opt
#define _out_opt
#endif

#ifndef __out_bcount_opt
#define __out_bcount_opt(size) // __allowed(size)
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif


//
// MessageId: STATUS_OBJECT_NAME_COLLISION
//
// MessageText:
//
// Object Name already exists.
//
#define STATUS_OBJECT_NAME_COLLISION     ((NTSTATUS)0xC0000035L)


//
// MessageId: STATUS_END_OF_FILE
//
// MessageText:
//
// The end-of-file marker has been reached. There is no valid data in the file beyond this marker.
//
#define STATUS_END_OF_FILE               ((NTSTATUS)0xC0000011L)


//
// These must be converted to LUIDs before use.
//

#define SE_MIN_WELL_KNOWN_PRIVILEGE         (2L)
#define SE_CREATE_TOKEN_PRIVILEGE           (2L)
#define SE_ASSIGNPRIMARYTOKEN_PRIVILEGE     (3L)
#define SE_LOCK_MEMORY_PRIVILEGE            (4L)
#define SE_INCREASE_QUOTA_PRIVILEGE         (5L)

#define SE_MACHINE_ACCOUNT_PRIVILEGE        (6L)
#define SE_TCB_PRIVILEGE                    (7L)
#define SE_SECURITY_PRIVILEGE               (8L)
#define SE_TAKE_OWNERSHIP_PRIVILEGE         (9L)
#define SE_LOAD_DRIVER_PRIVILEGE            (10L)
#define SE_SYSTEM_PROFILE_PRIVILEGE         (11L)
#define SE_SYSTEMTIME_PRIVILEGE             (12L)
#define SE_PROF_SINGLE_PROCESS_PRIVILEGE    (13L)
#define SE_INC_BASE_PRIORITY_PRIVILEGE      (14L)
#define SE_CREATE_PAGEFILE_PRIVILEGE        (15L)
#define SE_CREATE_PERMANENT_PRIVILEGE       (16L)
#define SE_BACKUP_PRIVILEGE                 (17L)
#define SE_RESTORE_PRIVILEGE                (18L)
#define SE_SHUTDOWN_PRIVILEGE               (19L)
#define SE_DEBUG_PRIVILEGE                  (20L)
#define SE_AUDIT_PRIVILEGE                  (21L)
#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE     (22L)
#define SE_CHANGE_NOTIFY_PRIVILEGE          (23L)
#define SE_REMOTE_SHUTDOWN_PRIVILEGE        (24L)
#define SE_UNDOCK_PRIVILEGE                 (25L)
#define SE_SYNC_AGENT_PRIVILEGE             (26L)
#define SE_ENABLE_DELEGATION_PRIVILEGE      (27L)
#define SE_MANAGE_VOLUME_PRIVILEGE          (28L)
#define SE_IMPERSONATE_PRIVILEGE            (29L)
#define SE_CREATE_GLOBAL_PRIVILEGE          (30L)
#define SE_TRUSTED_CREDMAN_ACCESS_PRIVILEGE (31L)
#define SE_RELABEL_PRIVILEGE                (32L)
#define SE_INC_WORKING_SET_PRIVILEGE        (33L)
#define SE_TIME_ZONE_PRIVILEGE              (34L)
#define SE_CREATE_SYMBOLIC_LINK_PRIVILEGE   (35L)
#define SE_MAX_WELL_KNOWN_PRIVILEGE         (SE_CREATE_SYMBOLIC_LINK_PRIVILEGE)
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)  


#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005

#define FILE_SUPERSEDED                 0x00000000
#define FILE_OPENED                     0x00000001
#define FILE_CREATED                    0x00000002
#define FILE_OVERWRITTEN                0x00000003
#define FILE_EXISTS                     0x00000004
#define FILE_DOES_NOT_EXIST             0x00000005

#define OBJ_INHERIT             0x00000002L  
#define OBJ_PERMANENT           0x00000010L  
#define OBJ_EXCLUSIVE           0x00000020L  
#define OBJ_CASE_INSENSITIVE    0x00000040L  
#define OBJ_OPENIF              0x00000080L  
#define OBJ_OPENLINK            0x00000100L  
#define OBJ_KERNEL_HANDLE       0x00000200L  
#define OBJ_FORCE_ACCESS_CHECK  0x00000400L  
#define OBJ_VALID_ATTRIBUTES    0x000007F2L  
#define FILE_ATTRIBUTE_NORMAL               0x00000080  
#define FILE_SHARE_DELETE                   0x00000004  
#define FILE_OPEN_IF                        0x00000003  
#define FILE_SYNCHRONOUS_IO_NONALERT        0x00000020  
#define GENERIC_WRITE                       (0x40000000L)  
#define SYNCHRONIZE                         (0x00100000L)  
#define GENERIC_READ                        (0x80000000L) 

#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008
#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080
#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_FOR_RECOVERY                  0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800
#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000
#define FILE_RESERVE_OPFILTER                   0x00100000
#define FILE_OPEN_REPARSE_POINT                 0x00200000
#define FILE_OPEN_NO_RECALL                     0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000
#define FILE_COPY_STRUCTURED_STORAGE            0x00000041
#define FILE_STRUCTURED_STORAGE                 0x00000441
#define FILE_VALID_OPTION_FLAGS                 0x00ffffff
#define FILE_VALID_PIPE_OPTION_FLAGS            0x00000032
#define FILE_VALID_MAILSLOT_OPTION_FLAGS        0x00000032
#define FILE_VALID_SET_FLAGS                    0x00000036
#define FILE_OPEN_REMOTE_INSTANCE               0x00000400
#define FILE_ATTRIBUTE_VALID_FLAGS              0x00007FB7



//++
//
// VOID
// InitializeObjectAttributes(
//     __out POBJECT_ATTRIBUTES p,
//     __in PUNICODE_STRING n,
//     __in ULONG a,
//     __in HANDLE r,
//     __in PSECURITY_DESCRIPTOR s
//     )
//
//--

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }

//////////////////////////////////////////////////////////////////////////   
//  structs 使用的结构体头
//////////////////////////////////////////////////////////////////////////

typedef struct FILE_STANDARD_INFORMATION {
    LARGE_INTEGER  AllocationSize;      //为文件非配的大小
    LARGE_INTEGER  EndOfFile;           //距离文件结尾还有多少字节
    ULONG  NumberOfLinks;               //有多少要个链接文件
    BOOLEAN  DeletePending;             //是否准备删除
    BOOLEAN  Directory;                 //是否为目录 
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

typedef struct FILE_POSITION_INFORMATION {
    LARGE_INTEGER  CurrentByteOffset; //代表当期文件指针为止
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS  Status;
        PVOID  Pointer;
    };
    ULONG_PTR  Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;


typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

// Environment information, which includes command line and
// image file name
//
typedef struct {
       ULONG            Unknown[21];     
       UNICODE_STRING   CommandLine;
       UNICODE_STRING   ImageFile;
} ENVIRONMENT_INFORMATION, *PENVIRONMENT_INFORMATION;
//
// This structure is passed as NtProcessStartup's parameter
//
typedef struct {
       ULONG                     Unknown[3];
       PENVIRONMENT_INFORMATION  Environment;
} STARTUP_ARGUMENT, *PSTARTUP_ARGUMENT;
//
// Data structure for heap definition. This includes various
// sizing parameters and callback routines, which, if left NULL,
// result in default behavior
//
typedef struct {
 ULONG     Length;
 ULONG     Unknown[11];
} RTL_HEAP_DEFINITION, *PRTL_HEAP_DEFINITION;


typedef struct _SYSTEM_MODULE_INFORMATION { 
    HANDLE Section;  
    PVOID MappedBase;  
    PVOID Base;   // 内存加载基址
    ULONG Size;   // 占用内存大小
    ULONG Flags; 
    USHORT LoadOrderIndex;  
    USHORT InitOrderIndex;  
    USHORT LoadCount; 
    USHORT ModuleNameOffset; 
    CHAR ImageName[256]; 
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct _STRING {
    USHORT  Length;
    USHORT  MaximumLength;
    PCHAR  Buffer;
} ANSI_STRING, *PANSI_STRING;

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

typedef struct _OBJECT_ATTRIBUTES {
    ULONG  Length;
    HANDLE  RootDirectory;
    PUNICODE_STRING  ObjectName;
    ULONG  Attributes;
    PVOID  SecurityDescriptor;
    PVOID  SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
typedef CONST OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;

#if _MSC_VER < 1300 
typedef struct _IO_COUNTERS
{
    LARGE_INTEGER ReadOperationCount;       //I/O读操作数目；
    LARGE_INTEGER WriteOperationCount;      //I/O写操作数目；
    LARGE_INTEGER OtherOperationCount;      //I/O其他操作数目；
    LARGE_INTEGER ReadTransferCount;        //I/O读数据数目；
    LARGE_INTEGER WriteTransferCount;       //I/O写数据数目；
    LARGE_INTEGER OtherTransferCount;       //I/O其他操作数据数目；
 }IO_COUNTERS,*PIO_COUNTERS;
#endif

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

// NtQuerySystemInformation 使用的参数
typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemProcessorInformation = 1,             // obsolete...delete
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemPathInformation,
    SystemProcessInformation = 5,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation = 8,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation = 11,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation = 16,
    SystemObjectInformation,
    SystemPageFileInformation = 18,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation = 21,
    SystemPoolTagInformation,
    SystemInterruptInformation = 23,
    SystemDpcBehaviorInformation = 24,
    SystemFullMemoryInformation = 25,
    SystemNotImplemented6 = 25,
    SystemLoadGdiDriverInformation = 26,
    SystemLoadImage = 26,
    SystemUnloadGdiDriverInformation = 27,
    SystemUnloadImage = 27,
    SystemTimeAdjustmentInformation = 28,
    SystemTimeAdjustment = 28,
    SystemSummaryMemoryInformation = 29,
    SystemNotImplemented7 = 29,
    SystemNextEventIdInformation = 30,
    SystemMirrorMemoryInformation = 30,
    SystemPerformanceTraceInformation = 31,
    SystemEventIdsInformation = 31,
    SystemObsolete0 = 32,
    SystemCrashDumpInformation = 32,
    SystemExceptionInformation = 33,
    SystemCrashDumpStateInformation = 34,
    SystemKernelDebuggerInformation = 35,
    SystemContextSwitchInformation = 36,
    SystemRegistryQuotaInformation = 37,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemVerifierAddDriverInformation,
    SystemVerifierRemoveDriverInformation,
    SystemProcessorIdleInformation,
    SystemLegacyDriverInformation,
    SystemCurrentTimeZoneInformation = 44,
    SystemTimeZoneInformation = 44,
    SystemLookasideInformation = 45,
    SystemSetTimeSlipEvent = 46,
    SystemTimeSlipNotification = 46,
    SystemSessionCreate = 47,
    SystemSessionDetach = 48,
    SystemSessionInformation = 49,
    SystemInvalidInfoClass4 = 49,
    SystemRangeStartInformation = 50,
    SystemVerifierInformation = 51,
    SystemVerifierThunkExtend = 52,
    SystemAddVerifier = 52,
    SystemSessionProcessInformation = 53,
    SystemLoadGdiDriverInSystemSpace,
    SystemNumaProcessorMap,
    SystemPrefetcherInformation,
    SystemExtendedProcessInformation,
    SystemRecommendedSharedDataAlignment,
    SystemComPlusPackage,
    SystemNumaAvailableMemory,
    SystemProcessorPowerInformation,
    SystemEmulationBasicInformation,
    SystemEmulationProcessorInformation,
    SystemExtendedHandleInformation,
    SystemLostDelayedWriteInformation,
    SystemBigPoolInformation,
    SystemSessionPoolTagInformation,
    SystemSessionMappedViewInformation,
    SystemHotpatchInformation,
    SystemObjectSecurityMode,
    SystemWatchdogTimerHandler,
    SystemWatchdogTimerInformation,
    SystemLogicalProcessorInformation,
    SystemWow64SharedInformation,
    SystemRegisterFirmwareTableInformationHandler,
    SystemFirmwareTableInformation,
    SystemModuleInformationEx,
    SystemVerifierTriageInformation,
    SystemSuperfetchInformation,
    SystemMemoryListInformation,
    SystemFileCacheInformationEx,
    MaxSystemInfoClass  // MaxSystemInfoClass should always be the last enum
} SYSTEM_INFORMATION_CLASS; 

typedef enum _SECTION_INHERIT
{
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT, *PSECTION_INHERIT;

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
    ObjectNameInformation = 1,
    ObjectTypeInformation = 2,
    ObjectAllTypesInformation = 3,
    ObjectHandleInformation = 4
} OBJECT_INFORMATION_CLASS;

typedef struct _FILE_NAME_INFORMATION {
    ULONG  FileNameLength;
    WCHAR  FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

typedef struct _PUBLIC_OBJECT_TYPE_INFORMATION {
    UNICODE_STRING  TypeName;
    ULONG  Reserved[22];
} PUBLIC_OBJECT_TYPE_INFORMATION,* PPUBLIC_OBJECT_TYPE_INFORMATION;

typedef enum _POOL_TYPE { 
    NonPagedPool,
    NonPagedPoolExecute                   = NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed               = NonPagedPool + 2,
    DontUseThisType,
    NonPagedPoolCacheAligned              = NonPagedPool + 4,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS         = NonPagedPool + 6,
    MaxPoolType,
    NonPagedPoolBase                      = 0,
    NonPagedPoolBaseMustSucceed           = NonPagedPoolBase + 2,
    NonPagedPoolBaseCacheAligned          = NonPagedPoolBase + 4,
    NonPagedPoolBaseCacheAlignedMustS     = NonPagedPoolBase + 6,
    NonPagedPoolSession                   = 32,
    PagedPoolSession                      = NonPagedPoolSession + 1,
    NonPagedPoolMustSucceedSession        = PagedPoolSession + 1,
    DontUseThisTypeSession                = NonPagedPoolMustSucceedSession + 1,
    NonPagedPoolCacheAlignedSession       = DontUseThisTypeSession + 1,
    PagedPoolCacheAlignedSession          = NonPagedPoolCacheAlignedSession + 1,
    NonPagedPoolCacheAlignedMustSSession  = PagedPoolCacheAlignedSession + 1,
    NonPagedPoolNx                        = 512,
    NonPagedPoolNxCacheAligned            = NonPagedPoolNx + 4,
    NonPagedPoolSessionNx                 = NonPagedPoolNx + 32
} POOL_TYPE;

typedef struct _OBJECT_TYPE_INFORMATION { // Information Class 2
    UNICODE_STRING Name;
    ULONG ObjectCount;
    ULONG HandleCount;
    ULONG Reserved1[4];
    ULONG PeakObjectCount;
    ULONG PeakHandleCount;
    ULONG Reserved2[4];
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccess;
    UCHAR Unknown;
    BOOLEAN MaintainHandleDatabase;
    POOL_TYPE PoolType;
    ULONG PagedPoolUsage;
    ULONG NonPagedPoolUsage;
}OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

typedef enum _MEMORY_INFORMATION_CLASS {
    MemoryBasicInformation,
    MemoryWorkingSetList,
    MemorySectionName,
    MemoryBasicVlmInformation
} MEMORY_INFORMATION_CLASS;

typedef enum _THREAD_STATE
{
    StateInitialized,
    StateReady,
    StateRunning,
    StateStandby,
    StateTerminated,
    StateWait,
    StateTransition,
    StateUnknown    
}THREAD_STATE;

typedef enum _KWAIT_REASON
{
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVertualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel
}KWAIT_REASON;

typedef struct _SYSTEM_THREADS
{
    LARGE_INTEGER KernelTime;               //CPU内核模式使用时间；
    LARGE_INTEGER UserTime;                 //CPU用户模式使用时间；
    LARGE_INTEGER CreateTime;               //线程创建时间；
    ULONG         WaitTime;                 //等待时间；
    PVOID         StartAddress;             //线程开始的虚拟地址；
    CLIENT_ID     ClientId;                 //线程标识符；
    KPRIORITY     Priority;                 //线程优先级；
    KPRIORITY     BasePriority;             //基本优先级；
    ULONG         ContextSwitchCount;       //环境切换数目；
    THREAD_STATE  State;                    //当前状态；
    KWAIT_REASON  WaitReason;               //等待原因；
}SYSTEM_THREADS,*PSYSTEM_THREADS;

typedef struct _VM_COUNTERS
{
    ULONG PeakVirtualSize;                  //虚拟存储峰值大小；
    ULONG VirtualSize;                      //虚拟存储大小；
    ULONG PageFaultCount;                   //页故障数目；
    ULONG PeakWorkingSetSize;               //工作集峰值大小；
    ULONG WorkingSetSize;                   //工作集大小；
    ULONG QuotaPeakPagedPoolUsage;          //分页池使用配额峰值；
    ULONG QuotaPagedPoolUsage;              //分页池使用配额；
    ULONG QuotaPeakNonPagedPoolUsage;       //非分页池使用配额峰值；
    ULONG QuotaNonPagedPoolUsage;           //非分页池使用配额；
    ULONG PagefileUsage;                    //页文件使用情况；
    ULONG PeakPagefileUsage;                //页文件使用峰值；
}VM_COUNTERS,*PVM_COUNTERS;

typedef struct _SYSTEM_PROCESSES
{
    ULONG          NextEntryDelta;          //构成结构序列的偏移量；
    ULONG          ThreadCount;             //线程数目；
    ULONG          Reserved1[6];           
    LARGE_INTEGER  CreateTime;              //创建时间；
    LARGE_INTEGER  UserTime;                //用户模式(Ring 3)的CPU时间；
    LARGE_INTEGER  KernelTime;              //内核模式(Ring 0)的CPU时间；
    UNICODE_STRING ProcessName;             //进程名称；
    KPRIORITY      BasePriority;            //进程优先权；
    HANDLE          ProcessId;               //进程标识符；
    HANDLE          InheritedFromProcessId;  //父进程的标识符；
    ULONG          HandleCount;             //句柄数目；
    ULONG          Reserved2[2];
    VM_COUNTERS    VmCounters;              //虚拟存储器的结构，见下；
    IO_COUNTERS    IoCounters;              //IO计数结构，见下；
    SYSTEM_THREADS Threads[1];              //进程相关线程的结构数组，见下；
}SYSTEM_PROCESSES,*PSYSTEM_PROCESSES;

typedef struct _OBJECT_NAME_INFORMATION {
    UNICODE_STRING Name; 
    WCHAR NameBuffer[1]; 
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;
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

NTSTATUS NTAPI
NtQuerySystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
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
    _in_opt HANDLE  Handle,
    _in OBJECT_INFORMATION_CLASS  ObjectInformationClass,
    __out_bcount_opt(ObjectInformationLength) PVOID  ObjectInformation,
    _in ULONG  ObjectInformationLength,
    _out_opt PULONG  ReturnLength
    );

NTSTATUS NTAPI
NtQueryObject(
    _in_opt HANDLE  Handle,
    _in OBJECT_INFORMATION_CLASS  ObjectInformationClass,
    __out_bcount_opt(ObjectInformationLength) PVOID  ObjectInformation,
    _in ULONG  ObjectInformationLength,
    _out_opt PULONG  ReturnLength
    );
    
ULONG NTAPI
RtlNtStatusToDosError(
    IN NTSTATUS Status);


NTSTATUS NTAPI
RtlAdjustPrivilege(
    ULONG    Privilege, 
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




NTSTATUS NTAPI
NtLoadDriver(
             IN PUNICODE_STRING  DriverServiceName
             );


NTSTATUS NTAPI
ZwLoadDriver(
             IN PUNICODE_STRING  DriverServiceName
             );


VOID NTAPI
RtlInitUnicodeString(
                     IN OUT PUNICODE_STRING  DestinationString,
                     IN PCWSTR  SourceString
                     );


NTSTATUS NTAPI
  ZwUnloadDriver(
    IN PUNICODE_STRING  DriverServiceName
    );

NTSTATUS NTAPI
  NtUnloadDriver(
    IN PUNICODE_STRING  DriverServiceName
    );


NTSTATUS NTAPI ZwReadVirtualMemory(
                              IN HANDLE ProcessHandle,
                              IN PVOID BaseAddress,
                              OUT PVOID Buffer,
                              IN ULONG BufferLength,
                              OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS NtReadVirtualMemory(
                             IN HANDLE ProcessHandle,
                             IN PVOID BaseAddress,
                             OUT PVOID Buffer,
                             IN ULONG BufferLength,
                             OUT PULONG ReturnLength OPTIONAL
    );


NTSTATUS  
NTAPI  
ZwQueryVirtualMemory(  
                     IN HANDLE ProcessHandle,  
                     IN PVOID BaseAddress,  
                     IN MEMORY_INFORMATION_CLASS MemoryInformationClass,  
                     OUT PVOID MemoryInformation,  
                     IN ULONG MemoryInformationLength,  
                     OUT PULONG ReturnLength OPTIONAL );  

NTSTATUS 
NTAPI
ZwCreateSection(
                OUT PHANDLE SectionHandle,
                IN ACCESS_MASK DesiredAccess,
                IN POBJECT_ATTRIBUTES ObjectAttributes,
                IN PLARGE_INTEGER MaximumSize,
                IN ULONG SectionPageProtection,
                IN ULONG AllocationAttributes,
                IN HANDLE FileHandle );

NTSTATUS 
NTAPI
NtCreateSection(
                OUT PHANDLE SectionHandle,
                IN ACCESS_MASK DesiredAccess,
                IN POBJECT_ATTRIBUTES ObjectAttributes,
                IN PLARGE_INTEGER MaximumSize,
                IN ULONG SectionPageProtection,
                IN ULONG AllocationAttributes,
                IN HANDLE FileHandle );
                
NTSTATUS
NTAPI
ZwOpenFile(
            OUT PHANDLE  FileHandle,
            IN ACCESS_MASK  DesiredAccess,
            IN POBJECT_ATTRIBUTES  ObjectAttributes,
            OUT PIO_STATUS_BLOCK  IoStatusBlock,
            IN ULONG  ShareAccess,
            IN ULONG  OpenOptions );

NTSTATUS
NTAPI
NtOpenFile(
            OUT PHANDLE  FileHandle,
            IN ACCESS_MASK  DesiredAccess,
            IN POBJECT_ATTRIBUTES  ObjectAttributes,
            OUT PIO_STATUS_BLOCK  IoStatusBlock,
            IN ULONG  ShareAccess,
            IN ULONG  OpenOptions );

NTSTATUS
NTAPI
ZwMapViewOfSection(
                    IN HANDLE  SectionHandle,
                    IN HANDLE  ProcessHandle,
                    IN OUT PVOID  *BaseAddress,
                    IN ULONG_PTR  ZeroBits,
                    IN SIZE_T  CommitSize,
                    IN OUT PLARGE_INTEGER  SectionOffset  OPTIONAL,
                    IN OUT PSIZE_T  ViewSize,
                    IN SECTION_INHERIT  InheritDisposition,
                    IN ULONG  AllocationType,
                    IN ULONG  Win32Protect  );


NTSTATUS
NTAPI
NtMapViewOfSection(
                    IN HANDLE  SectionHandle,
                    IN HANDLE  ProcessHandle,
                    IN OUT PVOID  *BaseAddress,
                    IN ULONG_PTR  ZeroBits,
                    IN SIZE_T  CommitSize,
                    IN OUT PLARGE_INTEGER  SectionOffset  OPTIONAL,
                    IN OUT PSIZE_T  ViewSize,
                    IN SECTION_INHERIT  InheritDisposition,
                    IN ULONG  AllocationType,
                    IN ULONG  Win32Protect  );

//
// Native NT api function to write something to the boot-time
// blue screen
//
NTSTATUS 
NTAPI 
NtDisplayString(
                  PUNICODE_STRING String 
                  );
//
// Native applications must kill themselves when done - the job
// of this native API
//
NTSTATUS 
NTAPI 
NtTerminateProcess(
                     HANDLE ProcessHandle, 
                     LONG ExitStatus 
                     );
//
// Definition to represent current process
//
#define NtCurrentProcess() ( (HANDLE) -1 )
//
// Heap creation routine
//
HANDLE 
NTAPI 
RtlCreateHeap(
               ULONG Flags, 
               PVOID BaseAddress, 
               ULONG SizeToReserve, 
               ULONG SizeToCommit, 
               PVOID Unknown,
               PRTL_HEAP_DEFINITION Definition
               );
//
// Heap allocation function (ala "malloc")
//
PVOID 
NTAPI 
RtlAllocateHeap(
                  HANDLE Heap, 
                  ULONG Flags, 
                  ULONG Size 
                  );
//
// Heap free function (ala "free")
//
BOOLEAN 
NTAPI 
RtlFreeHeap(
             HANDLE Heap, 
             ULONG Flags, 
             PVOID Address 
             );

VOID 
NTAPI
RtlFreeUnicodeString(
                     IN PUNICODE_STRING  UnicodeString
    );


NTSTATUS 
NTAPI
NtQueryEaFile(
              HANDLE hTemplateFile,
              PIO_STATUS_BLOCK pIoStatusBlock,
              PVOID EaBuffer,
              ULONG EaLength,
              BOOL,
              PVOID,
              ULONG,
              PVOID,
              BOOL);


NTSTATUS 
NTAPI
ZwCreateFile(
             PHANDLE  FileHandle,
             ACCESS_MASK  DesiredAccess,
             POBJECT_ATTRIBUTES  ObjectAttributes,
             PIO_STATUS_BLOCK  IoStatusBlock,
             PLARGE_INTEGER  AllocationSize,
             ULONG  FileAttributes,
             ULONG  ShareAccess,
             ULONG  CreateDisposition,
             ULONG  CreateOptions,
             PVOID  EaBuffer,
             ULONG  EaLength
    );


NTSTATUS 
NTAPI
NtCreateFile(
             PHANDLE  FileHandle,
             ACCESS_MASK  DesiredAccess,
             POBJECT_ATTRIBUTES  ObjectAttributes,
             PIO_STATUS_BLOCK  IoStatusBlock,
             PLARGE_INTEGER  AllocationSize,
             ULONG  FileAttributes,
             ULONG  ShareAccess,
             ULONG  CreateDisposition,
             ULONG  CreateOptions,
             PVOID  EaBuffer,
             ULONG  EaLength
    );


NTSTATUS 
NTAPI
ZwReadFile(
            IN HANDLE  FileHandle,
            IN HANDLE  Event  OPTIONAL,
            IN PVOID  ApcRoutine  OPTIONAL,
            IN PVOID  ApcContext  OPTIONAL,
            OUT PIO_STATUS_BLOCK  IoStatusBlock,
            OUT PVOID  Buffer,
            IN ULONG  Length,
            IN PLARGE_INTEGER  ByteOffset  OPTIONAL,
            IN PULONG  Key  OPTIONAL
            );

NTSTATUS 
NTAPI
NtReadFile(
           IN HANDLE  FileHandle,
           IN HANDLE  Event  OPTIONAL,
           IN PVOID  ApcRoutine  OPTIONAL,
           IN PVOID  ApcContext  OPTIONAL,
           OUT PIO_STATUS_BLOCK  IoStatusBlock,
           OUT PVOID  Buffer,
           IN ULONG  Length,
           IN PLARGE_INTEGER  ByteOffset  OPTIONAL,
           IN PULONG  Key  OPTIONAL
            );


NTSTATUS NTAPI ZwClose( IN HANDLE  Handle );

NTSTATUS 
NTAPI
ZwWriteFile(
            IN HANDLE  FileHandle,
            IN HANDLE  Event  OPTIONAL,
            IN PVOID  ApcRoutine  OPTIONAL,
            IN PVOID  ApcContext  OPTIONAL,
            OUT PIO_STATUS_BLOCK  IoStatusBlock,
            IN PVOID  Buffer,
            IN ULONG  Length,
            IN PLARGE_INTEGER  ByteOffset  OPTIONAL,
            IN PULONG  Key  OPTIONAL
    );

NTSTATUS 
NTAPI
NtWriteFile(
            IN HANDLE  FileHandle,
            IN HANDLE  Event  OPTIONAL,
            IN PVOID  ApcRoutine  OPTIONAL,
            IN PVOID  ApcContext  OPTIONAL,
            OUT PIO_STATUS_BLOCK  IoStatusBlock,
            IN PVOID  Buffer,
            IN ULONG  Length,
            IN PLARGE_INTEGER  ByteOffset  OPTIONAL,
            IN PULONG  Key  OPTIONAL
    );


NTSTATUS 
NTAPI
ZwQueryInformationFile(
                       IN HANDLE  FileHandle,
                       OUT PIO_STATUS_BLOCK  IoStatusBlock,
                       OUT PVOID  FileInformation,
                       IN ULONG  Length,
                       IN FILE_INFORMATION_CLASS  FileInformationClass
    );

NTSTATUS 
NTAPI
NtQueryInformationFile(
                       IN HANDLE  FileHandle,
                       OUT PIO_STATUS_BLOCK  IoStatusBlock,
                       OUT PVOID  FileInformation,
                       IN ULONG  Length,
                       IN FILE_INFORMATION_CLASS  FileInformationClass
    );

NTSTATUS
NTAPI 
ZwSetInformationFile(
                     IN HANDLE  FileHandle,
                     OUT PIO_STATUS_BLOCK  IoStatusBlock,
                     IN PVOID  FileInformation,
                     IN ULONG  Length,
                     IN FILE_INFORMATION_CLASS  FileInformationClass
    );



NTSTATUS 
NTAPI
NtSetInformationFile(
                     IN HANDLE  FileHandle,
                     OUT PIO_STATUS_BLOCK  IoStatusBlock,
                     IN PVOID  FileInformation,
                     IN ULONG  Length,
                     IN FILE_INFORMATION_CLASS  FileInformationClass
    );



#if _MSC_VER < 1300
int swprintf(    wchar_t *buffer, 
                const wchar_t *format ,
                ... );
#endif


#ifdef __cplusplus
};
#endif




#endif // _NATIVE_API_NTDLL_H_