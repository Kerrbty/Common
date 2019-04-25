#include "APIInterface.h"
#include "mykernel32.h"
#include "ntdll.h"

void BaseSetLastNTError(NTSTATUS status)
{
    SetLastError(RtlNtStatusToDosError(status));
}


HANDLE WINAPI API_CreateFileA(  
                              LPCSTR lpFileName,                         // file name
                              DWORD dwDesiredAccess,                      // access mode
                              DWORD dwShareMode,                          // share mode
                              LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                              DWORD dwCreationDisposition,                // how to create
                              DWORD dwFlagsAndAttributes,                 // file attributes
                              HANDLE hTemplateFile                        // handle to template file
                              )
{
    PWSTR  pElementText;
    int    iTextLen;
    
    iTextLen = MultiByteToWideChar( CP_ACP,
        0,
        (PCHAR)lpFileName,
        -1,
        NULL,
        0 );
    
    pElementText = 
        (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (iTextLen+1)*sizeof(WCHAR));
    
    MultiByteToWideChar( CP_ACP,
        0,
        (PCHAR)lpFileName,
        -1,
        pElementText,
		iTextLen );

    HANDLE handle = API_CreateFileW(
        pElementText, 
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile
        );

    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pElementText);
    return handle;
}


HANDLE WINAPI API_CreateFileW(  
                              LPCWSTR lpFileName,                         // file name
                              DWORD dwDesiredAccess,                      // access mode
                              DWORD dwShareMode,                          // share mode
                              LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                              DWORD dwCreationDisposition,                // how to create
                              DWORD dwFlagsAndAttributes,                 // file attributes
                              HANDLE hTemplateFile                        // handle to template file
                              )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING NtPathU;
    HANDLE FileHandle;
    NTSTATUS Status;
    ULONG FileAttributes, Flags = 0;
    PVOID EaBuffer = NULL;
    ULONG EaLength = 0;

    if (!lpFileName || !lpFileName[0])
    {
        SetLastError( ERROR_PATH_NOT_FOUND );
        return INVALID_HANDLE_VALUE;
    }

    /* validate & translate the creation disposition */
    switch (dwCreationDisposition)
    {
        case CREATE_NEW:
            dwCreationDisposition = FILE_CREATE;
            break;

        case CREATE_ALWAYS:
            dwCreationDisposition = FILE_OVERWRITE_IF;
            break;

        case OPEN_EXISTING:
            dwCreationDisposition = FILE_OPEN;
            break;

        case OPEN_ALWAYS:
            dwCreationDisposition = FILE_OPEN_IF;
            break;

        case TRUNCATE_EXISTING:
            dwCreationDisposition = FILE_OVERWRITE;
            break;

        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return (INVALID_HANDLE_VALUE);
    }

    /* check for console input/output */
    // 打开控制台输入输出句柄，不管
//     if (0 == wcsicmp(L"CONOUT$", lpFileName)
//         || 0 == wcsicmp(L"CONIN$", lpFileName))
//     {
//         return OpenConsoleW(lpFileName,
//                             dwDesiredAccess,
//                             lpSecurityAttributes ? lpSecurityAttributes->bInheritHandle : FALSE,
//                             FILE_SHARE_READ | FILE_SHARE_WRITE);
//     }

    /* validate & translate the flags */

    /* translate the flags that need no validation */
    if (!(dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED))
    {
        /* yes, nonalert is correct! apc's are not delivered
        while waiting for file io to complete */
        Flags |= FILE_SYNCHRONOUS_IO_NONALERT;
    }

    if(dwFlagsAndAttributes & FILE_FLAG_WRITE_THROUGH)
        Flags |= FILE_WRITE_THROUGH;

    if(dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING)
        Flags |= FILE_NO_INTERMEDIATE_BUFFERING;

    if(dwFlagsAndAttributes & FILE_FLAG_RANDOM_ACCESS)
        Flags |= FILE_RANDOM_ACCESS;

    if(dwFlagsAndAttributes & FILE_FLAG_SEQUENTIAL_SCAN)
        Flags |= FILE_SEQUENTIAL_ONLY;

    if(dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE)
        Flags |= FILE_DELETE_ON_CLOSE;

    if(dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS)
    {
        if(dwDesiredAccess & GENERIC_ALL)
        Flags |= FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REMOTE_INSTANCE;
        else
        {
            if(dwDesiredAccess & GENERIC_READ)
                Flags |= FILE_OPEN_FOR_BACKUP_INTENT;

            if(dwDesiredAccess & GENERIC_WRITE)
                Flags |= FILE_OPEN_REMOTE_INSTANCE;
        }
    }
    else
        Flags |= FILE_NON_DIRECTORY_FILE;

    if(dwFlagsAndAttributes & FILE_FLAG_OPEN_REPARSE_POINT)
        Flags |= FILE_OPEN_REPARSE_POINT;

    if(dwFlagsAndAttributes & FILE_FLAG_OPEN_NO_RECALL)
        Flags |= FILE_OPEN_NO_RECALL;

    FileAttributes = (dwFlagsAndAttributes & (FILE_ATTRIBUTE_VALID_FLAGS & ~FILE_ATTRIBUTE_DIRECTORY));

    /* handle may always be waited on and querying attributes are always allowed */
    dwDesiredAccess |= SYNCHRONIZE | FILE_READ_ATTRIBUTES;

    /* FILE_FLAG_POSIX_SEMANTICS is handled later */

    /* validate & translate the filename */
    if (!RtlDosPathNameToNtPathName_U (lpFileName,
                                    &NtPathU,
                                    NULL,
                                    NULL)
         )
    {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    if (hTemplateFile != NULL)
    {
        // FILE_EA 不实现
        return INVALID_HANDLE_VALUE;
    }

    /* build the object attributes */
    InitializeObjectAttributes(&ObjectAttributes,
                                &NtPathU,
                                0,
                                NULL,
                                NULL);

    if (lpSecurityAttributes)
    {
        if(lpSecurityAttributes->bInheritHandle)
        ObjectAttributes.Attributes |= OBJ_INHERIT;

        ObjectAttributes.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
    }

    if(!(dwFlagsAndAttributes & FILE_FLAG_POSIX_SEMANTICS))
        ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    /* perform the call */
    Status = NtCreateFile (&FileHandle,
                        dwDesiredAccess,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        NULL,
                        FileAttributes,
                        dwShareMode,
                        dwCreationDisposition,
                        Flags,
                        EaBuffer,
                        EaLength);

    RtlFreeHeap(GetProcessHeap(),
                0,
                NtPathU.Buffer);

    /* free the extended attributes buffer if allocated */
    if (EaBuffer != NULL)
    {
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, EaBuffer);
    }

    /* error */
    if (!NT_SUCCESS(Status))
    {
        /* In the case file creation was rejected due to CREATE_NEW flag
        * was specified and file with that name already exists, correct
        * last error is ERROR_FILE_EXISTS and not ERROR_ALREADY_EXISTS.
        * Note: RtlNtStatusToDosError is not the subject to blame here.
        */
        if ( Status == STATUS_OBJECT_NAME_COLLISION && dwCreationDisposition == FILE_CREATE)
        {
            SetLastError( ERROR_FILE_EXISTS );
        }
        else
        {
            BaseSetLastNTError (Status);
        }

        return INVALID_HANDLE_VALUE;
    }

    /*
    create with OPEN_ALWAYS (FILE_OPEN_IF) returns info = FILE_OPENED or FILE_CREATED
    create with CREATE_ALWAYS (FILE_OVERWRITE_IF) returns info = FILE_OVERWRITTEN or FILE_CREATED
    */
    if (dwCreationDisposition == FILE_OPEN_IF)
    {
        SetLastError( (ULONG)IoStatusBlock.Information == FILE_OPENED ? ERROR_ALREADY_EXISTS : ERROR_SUCCESS);
    }
    else if (dwCreationDisposition == FILE_OVERWRITE_IF)
    {
        SetLastError( (ULONG)IoStatusBlock.Information == FILE_OVERWRITTEN ? ERROR_ALREADY_EXISTS : ERROR_SUCCESS);
    }
    else
    {
        SetLastError(ERROR_SUCCESS);
    }

    return FileHandle;
}



BOOL WINAPI API_ReadFile(  
                         HANDLE hFile,                // handle to file
                         LPVOID lpBuffer,             // data buffer
                         DWORD nNumberOfBytesToRead,  // number of bytes to read
                         LPDWORD lpNumberOfBytesRead, // number of bytes read
                         LPOVERLAPPED lpOverlapped    // overlapped buffer
                         )
{
    NTSTATUS Status;


    if (lpNumberOfBytesRead != NULL)
    {
        *lpNumberOfBytesRead = 0;
    }

    if (!nNumberOfBytesToRead)
    {
        return TRUE;
    }

    if (lpOverlapped != NULL)
    {
        LARGE_INTEGER Offset;
        PVOID ApcContext;

        Offset.u.LowPart = lpOverlapped->Offset;
        Offset.u.HighPart = lpOverlapped->OffsetHigh;
        lpOverlapped->Internal = STATUS_PENDING;
        ApcContext = (((ULONG)lpOverlapped->hEvent & 0x1) ? NULL : lpOverlapped);

        Status = NtReadFile(hFile,
                                lpOverlapped->hEvent,
                                NULL,
                                ApcContext,
                                (PIO_STATUS_BLOCK)lpOverlapped,
                                lpBuffer,
                                nNumberOfBytesToRead,
                                &Offset,
                                NULL);

            /* return FALSE in case of failure and pending operations! */
            if (!NT_SUCCESS(Status) || Status == STATUS_PENDING)
            {
                if (Status == STATUS_END_OF_FILE &&
                lpNumberOfBytesRead != NULL)
                {
                    *lpNumberOfBytesRead = 0;
                }

                BaseSetLastNTError(Status);
                return FALSE;
            }

            if (lpNumberOfBytesRead != NULL)
            {
                *lpNumberOfBytesRead = lpOverlapped->InternalHigh;
            }
            }
            else
            {
            IO_STATUS_BLOCK Iosb;

            Status = NtReadFile(hFile,
                                NULL,
                                NULL,
                                NULL,
                                &Iosb,
                                lpBuffer,
                                nNumberOfBytesToRead,
                                NULL,
                                NULL);

            /* wait in case operation is pending */
            if (Status == STATUS_PENDING)
            {
                Status = WaitForSingleObject(hFile, INFINITE);
                if (NT_SUCCESS(Status))
                {
                    Status = Iosb.Status;
                }
            }

            if (Status == STATUS_END_OF_FILE)
            {
                /* lpNumberOfBytesRead must not be NULL here, in fact Win doesn't
                check that case either and crashes (only after the operation
                completed) */
                *lpNumberOfBytesRead = 0;
                return TRUE;
            }
                                    
            if (NT_SUCCESS(Status))
            {
                /* lpNumberOfBytesRead must not be NULL here, in fact Win doesn't
                check that case either and crashes (only after the operation
                completed) */
                *lpNumberOfBytesRead = (ULONG)Iosb.Information;
            }
            else
            {
                BaseSetLastNTError(Status);
                return FALSE;
            }
        }

   return TRUE;
}



BOOL WINAPI API_WriteFile(  
                          HANDLE hFile,                    // handle to file
                          LPCVOID lpBuffer,                // data buffer
                          DWORD nNumberOfBytesToWrite,     // number of bytes to write
                          LPDWORD lpNumberOfBytesWritten,  // number of bytes written
                          LPOVERLAPPED lpOverlapped        // overlapped buffer
                          )
{
    NTSTATUS Status;

    if (lpNumberOfBytesWritten != NULL)
    {
        *lpNumberOfBytesWritten = 0;
    }


    if (lpOverlapped != NULL)
    {
        LARGE_INTEGER Offset;
        PVOID ApcContext;

        Offset.u.LowPart = lpOverlapped->Offset;
        Offset.u.HighPart = lpOverlapped->OffsetHigh;
        lpOverlapped->Internal = STATUS_PENDING;
        ApcContext = (((ULONG)lpOverlapped->hEvent & 0x1) ? NULL : lpOverlapped);

        Status = NtWriteFile(hFile,
                            lpOverlapped->hEvent,
                            NULL,
                            ApcContext,
                            (PIO_STATUS_BLOCK)lpOverlapped,
                            (PVOID)lpBuffer,
                            nNumberOfBytesToWrite,
                            &Offset,
                            NULL);

        /* return FALSE in case of failure and pending operations! */
        if (!NT_SUCCESS(Status) || Status == STATUS_PENDING)
        {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        if (lpNumberOfBytesWritten != NULL)
        {
            *lpNumberOfBytesWritten = lpOverlapped->InternalHigh;
        }
    }
    else
    {
        IO_STATUS_BLOCK Iosb;

        Status = NtWriteFile(hFile,
                            NULL,
                            NULL,
                            NULL,
                            &Iosb,
                            (PVOID)lpBuffer,
                            nNumberOfBytesToWrite,
                            NULL,
                            NULL);

        /* wait in case operation is pending */
        if (Status == STATUS_PENDING)
        {
            Status = WaitForSingleObject(hFile, INFINITE);
            if (NT_SUCCESS(Status))
            {
                Status = Iosb.Status;
            }
        }

        if (NT_SUCCESS(Status))
        {
            /* lpNumberOfBytesWritten must not be NULL here, in fact Win doesn't
            check that case either and crashes (only after the operation
            completed) */
            *lpNumberOfBytesWritten = (ULONG)Iosb.Information;
        }
        else
        {
            BaseSetLastNTError(Status);
            return FALSE;
        }
    }

    return TRUE;
}


BOOL WINAPI API_CloseHandle(HANDLE hObject)
{
    NTSTATUS status = ZwClose(hObject);
    if (!NT_SUCCESS(status))
    {
        BaseSetLastNTError(status);
        return FALSE;
    }
    return TRUE;
}

#define INVALID_SET_FILE_POINTER  0xFFFFFFFF

DWORD WINAPI API_SetFilePointer(  
                                HANDLE hFile,                // handle to file
                                LONG lDistanceToMove,        // bytes to move pointer
                                PLONG lpDistanceToMoveHigh,  // bytes to move pointer
                                DWORD dwMoveMethod           // starting point
                                )
{
    FILE_POSITION_INFORMATION FilePosition;
    FILE_STANDARD_INFORMATION FileStandard;
    NTSTATUS errCode;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER Distance;

    if (lpDistanceToMoveHigh)
    {
        Distance.u.HighPart = *lpDistanceToMoveHigh;
        Distance.u.LowPart = lDistanceToMove;
    }
    else
    {
        Distance.QuadPart = lDistanceToMove;
    }

    switch(dwMoveMethod)
    {
        case FILE_CURRENT:
            errCode = NtQueryInformationFile(hFile,
            &IoStatusBlock,
            &FilePosition,
            sizeof(FILE_POSITION_INFORMATION),
            FilePositionInformation);
            FilePosition.CurrentByteOffset.QuadPart += Distance.QuadPart;
            if (!NT_SUCCESS(errCode))
            {
                if (lpDistanceToMoveHigh != NULL)
                *lpDistanceToMoveHigh = -1;
                BaseSetLastNTError(errCode);
                return INVALID_SET_FILE_POINTER;
            }
            break;
        case FILE_END:
            errCode = NtQueryInformationFile(hFile,
            &IoStatusBlock,
            &FileStandard,
            sizeof(FILE_STANDARD_INFORMATION),
            FileStandardInformation);
            FilePosition.CurrentByteOffset.QuadPart =
            FileStandard.EndOfFile.QuadPart + Distance.QuadPart;
            if (!NT_SUCCESS(errCode))
            {
                if (lpDistanceToMoveHigh != NULL)
                *lpDistanceToMoveHigh = -1;
                BaseSetLastNTError(errCode);
                return INVALID_SET_FILE_POINTER;
            }
            break;
        case FILE_BEGIN:
            FilePosition.CurrentByteOffset.QuadPart = Distance.QuadPart;
            break;
        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return INVALID_SET_FILE_POINTER;
    }

    if(FilePosition.CurrentByteOffset.QuadPart < 0)
    {
        SetLastError(ERROR_NEGATIVE_SEEK);
        return INVALID_SET_FILE_POINTER;
    }

    if (lpDistanceToMoveHigh == NULL && FilePosition.CurrentByteOffset.HighPart != 0)
    {
        /* If we're moving the pointer outside of the 32 bit boundaries but
        the application only passed a 32 bit value we need to bail out! */
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_SET_FILE_POINTER;
    }

    errCode = NtSetInformationFile(hFile,
                                    &IoStatusBlock,
                                    &FilePosition,
                                    sizeof(FILE_POSITION_INFORMATION),
                                    FilePositionInformation);
    if (!NT_SUCCESS(errCode))
    {
        if (lpDistanceToMoveHigh != NULL)
        *lpDistanceToMoveHigh = -1;

        BaseSetLastNTError(errCode);
        return INVALID_SET_FILE_POINTER;
    }

    if (lpDistanceToMoveHigh != NULL)
    {
        *lpDistanceToMoveHigh = FilePosition.CurrentByteOffset.u.HighPart;
    }

    if (FilePosition.CurrentByteOffset.u.LowPart == MAXDWORD)
    {
        /* The value of -1 is valid here, especially when the new
        file position is greater than 4 GB. Since NtSetInformationFile
        succeeded we never set an error code and we explicitly need
        to clear a previously set error code in this case, which
        an application will check if INVALID_SET_FILE_POINTER is returned! */
        SetLastError(ERROR_SUCCESS);
    }

   return FilePosition.CurrentByteOffset.u.LowPart;
}


BOOL WINAPI API_SetFilePointerEx(  
                                 HANDLE hFile,                    // handle to file
                                 LARGE_INTEGER liDistanceToMove,  // bytes to move pointer
                                 PLARGE_INTEGER lpNewFilePointer, // new file pointer
                                 DWORD dwMoveMethod               // starting point
                                 )
{
    FILE_POSITION_INFORMATION FilePosition;
    FILE_STANDARD_INFORMATION FileStandard;
    NTSTATUS errCode;
    IO_STATUS_BLOCK IoStatusBlock;


    switch(dwMoveMethod)
    {
        case FILE_CURRENT:
            NtQueryInformationFile(hFile,
            &IoStatusBlock,
            &FilePosition,
            sizeof(FILE_POSITION_INFORMATION),
            FilePositionInformation);
            FilePosition.CurrentByteOffset.QuadPart += liDistanceToMove.QuadPart;
            break;
        case FILE_END:
            NtQueryInformationFile(hFile,
            &IoStatusBlock,
            &FileStandard,
            sizeof(FILE_STANDARD_INFORMATION),
            FileStandardInformation);
            FilePosition.CurrentByteOffset.QuadPart =
            FileStandard.EndOfFile.QuadPart + liDistanceToMove.QuadPart;
            break;
        case FILE_BEGIN:
            FilePosition.CurrentByteOffset.QuadPart = liDistanceToMove.QuadPart;
            break;
        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
    }

    if(FilePosition.CurrentByteOffset.QuadPart < 0)
    {
        SetLastError(ERROR_NEGATIVE_SEEK);
        return FALSE;
    }

    errCode = NtSetInformationFile(hFile,
                                    &IoStatusBlock,
                                    &FilePosition,
                                    sizeof(FILE_POSITION_INFORMATION),
                                    FilePositionInformation);
    if (!NT_SUCCESS(errCode))
    {
        BaseSetLastNTError(errCode);
        return FALSE;
    }

    if (lpNewFilePointer)
    {
        *lpNewFilePointer = FilePosition.CurrentByteOffset;
    }
    return TRUE;
}