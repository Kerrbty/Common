#ifndef _MSPORTS_H
#define _MSPORTS_H
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include <windows.h>
#include <Setupapi.h>
#pragma comment(lib, "msports")

DECLARE_HANDLE(HCOMDB);
typedef HCOMDB *PHCOMDB;
#define HCOMDB_INVALID_HANDLE_VALUE ((HCOMDB) INVALID_HANDLE_VALUE)

//
// Minimum through maximum number of COM names arbitered
//
#define COMDB_MIN_PORTS_ARBITRATED 256
#define COMDB_MAX_PORTS_ARBITRATED 4096

#ifndef __out
#define __out
#endif
#ifndef __in_opt
#define __in_opt
#endif
#ifndef __out
#define __out
#endif
#ifndef __out_bcount_opt
#define __out_bcount_opt(x)
#endif
#ifndef __in
#define __in
#endif
#ifndef __out_opt
#define __out_opt
#endif

/* CM_PARTIAL_RESOURCE_DESCRIPTOR.Type */
#define CmResourceTypeNull                0
#define CmResourceTypePort                1
#define CmResourceTypeInterrupt           2
#define CmResourceTypeMemory              3
#define CmResourceTypeDma                 4
#define CmResourceTypeDeviceSpecific      5
#define CmResourceTypeBusNumber           6
#define CmResourceTypeMemoryLarge         7
#define CmResourceTypeNonArbitrated       128
#define CmResourceTypeConfigData          128
#define CmResourceTypeDevicePrivate       129
#define CmResourceTypePcCardConfig        130
#define CmResourceTypeMfCardConfig        131

typedef enum _INTERFACE_TYPE { 
    InterfaceTypeUndefined  = -1,
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    PCIBus,
    VMEBus,
    NuBus,
    PCMCIABus,
    CBus,
    MPIBus,
    MPSABus,
    ProcessorInternal,
    InternalPowerBus,
    PNPISABus,
    PNPBus,
    Vmcs,
    ACPIBus,
    MaximumInterfaceType
} INTERFACE_TYPE, *PINTERFACE_TYPE;

#if _MSC_VER < 1400 
typedef unsigned __int64  KAFFINITY;
#endif

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

typedef struct _CM_PARTIAL_RESOURCE_DESCRIPTOR {
    UCHAR  Type;
    UCHAR  ShareDisposition;
    USHORT  Flags;
    union {
        struct {
            PHYSICAL_ADDRESS  Start;
            ULONG  Length;
        } Generic;
        struct {
            PHYSICAL_ADDRESS  Start;
            ULONG  Length;
        } Port;
        struct {
#if defined(NT_PROCESSOR_GROUPS)
            USHORT  Level;
            USHORT  Group;
#else
            ULONG  Level;
#endif
            ULONG  Vector;
            KAFFINITY Affinity;
        } Interrupt;

        // This member exists only on Windows Vista and later
        struct {
            union {
               struct {
#if defined(NT_PROCESSOR_GROUPS)
                   USHORT  Group;
#else
                   USHORT  Reserved;
#endif
                   USHORT  MessageCount;
                   ULONG  Vector;
                   KAFFINITY  Affinity;
               } Raw;

               struct {
#if defined(NT_PROCESSOR_GROUPS)
                   USHORT  Level;
                   USHORT  Group;
#else
                   ULONG  Level;
#endif
                   ULONG  Vector;
                   KAFFINITY  Affinity;
               } Translated;        
            };
        } MessageInterrupt;
        struct {
            PHYSICAL_ADDRESS  Start;
            ULONG  Length;
        } Memory;
        struct {
            ULONG  Channel;
            ULONG  Port;
            ULONG  Reserved1;
        } Dma;
        struct {  
            ULONG Channel;  
            ULONG RequestLine;  
            UCHAR TransferWidth;  
            UCHAR Reserved1;  
            UCHAR Reserved2;  
            UCHAR Reserved3;  
        } DmaV3;
        struct {
            ULONG  Data[3];
        } DevicePrivate;
        struct {
            ULONG  Start;
            ULONG  Length;
            ULONG  Reserved;
        } BusNumber;
        struct {
            ULONG  DataSize;
            ULONG  Reserved1;
            ULONG  Reserved2;
        } DeviceSpecificData;
        // The following structures provide support for memory-mapped
        // IO resources greater than MAXULONG
        struct {
            PHYSICAL_ADDRESS  Start;
            ULONG  Length40;
        } Memory40;
        struct {
            PHYSICAL_ADDRESS  Start;
            ULONG  Length48;
        } Memory48;
        struct {
            PHYSICAL_ADDRESS  Start;
            ULONG  Length64;
        } Memory64;
        struct {
            UCHAR Class;
            UCHAR Type;
            UCHAR Reserved1;
            UCHAR Reserved2;
            ULONG IdLowPart;
            ULONG IdHighPart;
        } Connection;		  
    } u;
} CM_PARTIAL_RESOURCE_DESCRIPTOR, *PCM_PARTIAL_RESOURCE_DESCRIPTOR;

typedef struct _CM_PARTIAL_RESOURCE_LIST {
    USHORT                         Version;
    USHORT                         Revision;
    ULONG                          Count;
    CM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptors[1];
} CM_PARTIAL_RESOURCE_LIST, *PCM_PARTIAL_RESOURCE_LIST;

typedef struct _CM_FULL_RESOURCE_DESCRIPTOR {
    INTERFACE_TYPE           InterfaceType;
    ULONG                    BusNumber;
    CM_PARTIAL_RESOURCE_LIST PartialResourceList;
} CM_FULL_RESOURCE_DESCRIPTOR, *PCM_FULL_RESOURCE_DESCRIPTOR;

typedef struct _CM_RESOURCE_LIST {
    ULONG                       Count;
    CM_FULL_RESOURCE_DESCRIPTOR List[1];
} CM_RESOURCE_LIST, *PCM_RESOURCE_LIST;

#ifdef SERIAL_ADVANCED_SETTINGS

/*++

Routine Description:

    Displays the advanced properties dialog for the COM port specified by
    DeviceInfoSet and DeviceInfoData.

Arguments:

    ParentHwnd  - the parent window of the window to be displayed

    DeviceInfoSet, DeviceInfoData - SetupDi structures representing the COM port

Return Value:

    ERROR_SUCCESS if the dialog was shown

  --*/
LONG
SerialDisplayAdvancedSettings(__in HWND             ParentHwnd,
                              __in HDEVINFO         DeviceInfoSet,
                              __in PSP_DEVINFO_DATA DeviceInfoData
                              );

/*++

Routine Description:

    Prototype to allow serial port vendors to override the advanced dialog
    represented by the COM port specified by DeviceInfoSet and DeviceInfoData.

    To override the advanced page, place a value named EnumAdvancedDialog under
    the same key in which you would put your EnumPropPages32 value.  The format
    of the value is exactly the same as Enum...32 as well.

Arguments:

    ParentHwnd  - the parent window of the window to be displayed

    HidePollingUI - If TRUE, hide all UI that deals with polling.

    DeviceInfoSet, DeviceInfoData - SetupDi structures representing the COM port

    Reserved - Unused

Return Value:

    TRUE if the user pressed OK, FALSE if Cancel was pressed
--*/

typedef
BOOL
(*PPORT_ADVANCED_DIALOG) (
    __in HWND             ParentHwnd,
    __in BOOL             HidePollingUI,
    __in HDEVINFO         DeviceInfoSet,
    __in PSP_DEVINFO_DATA DeviceInfoData,
    __in_opt PVOID        Reserved
    );

#endif

LONG
WINAPI
ComDBOpen (
           __out PHCOMDB PHComDB
    );
/*++

Routine Description:

    Opens name data base, and returns a handle to be used in future calls.

Arguments:

    None.

Return Value:

    INVALID_HANDLE_VALUE if the call fails, otherwise a valid handle

    If INVALID_HANDLE_VALUE, call GetLastError() to get details (??)

--*/


LONG
WINAPI
ComDBClose (
    __in HCOMDB HComDB
    );
/*++

Routine Description:

    frees a handle to the database returned from OpenComPortDataBase

Arguments:

    Handle returned from OpenComPortDataBase.

Return Value:

    None

--*/

#define CDB_REPORT_BITS      0x0
#define CDB_REPORT_BYTES     0x1

LONG
WINAPI
ComDBGetCurrentPortUsage (
    __in        HCOMDB   HComDB,
    __out_bcount_opt(BufferSize) PBYTE Buffer,
    __in        DWORD    BufferSize,
    __in        ULONG    ReportType, // CDB_REPORT value
    __out_opt   LPDWORD  MaxPortsReported
    );
/*++

Routine Description:

    if Buffer is NULL, than MaxPortsReported will contain the max number of ports
        the DB will report (this value is NOT the number of bytes need for Buffer).
        ReportType is ignored in this case.

    if ReportType == CDB_REPORT_BITS
        returns a bit array indicating if a comX name is claimed.
        ie, Bit 0 of Byte 0 is com1, bit 1 of byte 0 is com2 and so on.

        BufferSize >= MaxPortsReported / 8


    if ReportType == CDB_REPORT_BYTES
        returns a byte array indicating if a comX name is claimed.  Zero unused, non zero
        used, ie, byte 0 is com1, byte 1 is com2, etc

        BufferSize >= MaxPortsReported

Arguments:

    Handle returned from OpenComPortDataBase.

    Buffer pointes to memory to place bit array

    BufferSize   Size of buffer in bytes

    MaxPortsReported    Pointer to DWORD that holds the number of bytes in buffer filled in

Return Value:

    returns ERROR_SUCCESS if successful.
            ERROR_NOT_CONNECTED cannot connect to DB
            ERROR_MORE_DATA if buffer not large enough

--*/


LONG
WINAPI
ComDBClaimNextFreePort (
    __in HCOMDB   HComDB,
    __out LPDWORD ComNumber
    );
/*++

Routine Description:

    returns the first free COMx value

Arguments:

    Handle returned from OpenComPortDataBase.

Return Value:


    returns ERROR_SUCCESS if successful. or other ERROR_ if not

    if successful, then ComNumber will be that next free com value and claims it in the database


--*/



LONG
WINAPI
ComDBClaimPort (
    __in        HCOMDB   HComDB,
    __in        DWORD    ComNumber,
    __in        BOOL     ForceClaim,
    __out_opt   PBOOL    Forced
    );
/*++

Routine Description:

    Attempts to claim a com name in the database

Arguments:

    DataBaseHandle - returned from OpenComPortDataBase.

    ComNumber      - The port value to be claimed

    Force          - If TRUE, will force the port to be claimed even if in use already

    Forced         - will reflect the event that the claim was forced

Return Value:


    returns ERROR_SUCCESS if port name was not already claimed, or if it was claimed
                          and Force was TRUE.

            ERROR_SHARING_VIOLATION if port name is use and Force is false


--*/

LONG
WINAPI
ComDBReleasePort (
    __in HCOMDB   HComDB,
    __in DWORD    ComNumber
    );
/*++

Routine Description:

    Releases the port in the database

Arguments:

    DatabaseHandle - returned from OpenComPortDataBase.

    ComNumber      - port to be unclaimed in database

Return Value:


    returns ERROR_SUCCESS if successful
            ERROR_CANTWRITE if the changes cannot be committed
            ERROR_INVALID_PARAMETER if ComNumber is greater than the number of
                                    ports arbitrated


--*/

LONG
WINAPI
ComDBResizeDatabase (
    __in HCOMDB   HComDB,
    __in DWORD    NewSize
    );
/*++

Routine Description:

    Resizes the database to the new size.  To get the current size, call
    ComDBGetCurrentPortUsage with a Buffer == NULL.

Arguments:

    DatabaseHandle - returned from OpenComPortDataBase.

    NewSize        - must be a multiple of 1024, with a max of 4096

Return Value:


    returns ERROR_SUCCESS if successful
            ERROR_CANTWRITE if the changes cannot be committed
            ERROR_BAD_LENGTH if NewSize is not greater than the current size or
                             NewSize is greater than COMDB_MAX_PORTS_ARBITRATED

--*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // _MSPORTS_H