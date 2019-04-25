#include "FileFilter.h"
#include "../List.h"
#include "../detours/detours.h"
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi")

typedef struct _EncryptFileList 
{
    LIST_ENTRY next; 
    HANDLE hCreateHandle; 
    LARGE_INTEGER laFileSize;
    BYTE bKey;
}EncryptFileList, *PEncryptFileList;


// 加入加密保护列表 
BOOL WINAPI InsertFileHandle( HANDLE hFile, LONGLONG llFileSize, BYTE bKey);
BOOL WINAPI InsertMapHandle( HANDLE hFile, LONGLONG llFileSize, BYTE bKey);

// 判断是否在列表中 
BOOL WINAPI FileHandleInList(HANDLE handle);
BOOL WINAPI MapHandleInList(HANDLE handle);

// 获取加密key 
// 不存在返回 -1  
BYTE WINAPI GetFileKeyAndSize(HANDLE handle, PLARGE_INTEGER laFileSize);
BYTE WINAPI GetMapKeyAndSize(HANDLE handle, PLARGE_INTEGER laFileSize);


//////////////////////////////////////////////////////////////////////////
// 全局变量
//////////////////////////////////////////////////////////////////////////
EncryptFileList Global_EncryptFile;   // File List 
EncryptFileList Global_EncryptMap;    // Map List 
static CRITICAL_SECTION g_Encrypt_File_cs;
static CRITICAL_SECTION g_Encrypt_Map_cs;


// 文件读写打开操作
pCreateFileW pRealCreateFileW = CreateFileW;
pReadFile pRealReadFile = ReadFile;
pWriteFile pRealWriteFile = WriteFile;
pCloseHandle pRealCloseHandle = CloseHandle;

// 文件大小
F_GetFileSize pRealGetFileSize = GetFileSize;
F_GetFileSizeEx pRealGetFileSizeEx = GetFileSizeEx;

// 文件偏移
F_SetFilePointer pRealSetFilePointer = SetFilePointer;
F_SetFilePointerEx pRealSetFilePointerEx = SetFilePointerEx;

// 文件映射
F_CreateFileMappingA pRealCreateFileMappingA = CreateFileMappingA;
F_CreateFileMappingW pRealCreateFileMappingW = CreateFileMappingW;
F_MapViewOfFile pRealMapViewOfFile = MapViewOfFile;
F_MapViewOfFileEx pRealMapViewOfFileEx = MapViewOfFileEx;
F_UnmapViewOfFile pRealUnmapViewOfFile = UnmapViewOfFile;


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 解码函数    
// 只需要修改这里的函数，适配所有加密 
//////////////////////////////////////////////////////////////////////////
// 拦截到的读操作解码 
#define TOTAL_NUMBER (1*1024*1024)  // 加密大小 


__forceinline VOID WINAPI RealDecode(LPBYTE pbuf, int nStart, int nCount, BYTE bKey)
{
    BYTE bKeyValue = (BYTE)((bKey + nStart) % 255);
    for (int i = 0; i < nCount; i++)
    {
        BYTE tmpVal = (BYTE)((bKeyValue + i) % 255);
        pbuf[i] = pbuf[i]^tmpVal;
    }
}

// 读hook，拦截规则，写入列表 
HANDLE WINAPI HookCreateFileW(
                              LPCWSTR lpFileName,                         // file name
                              DWORD dwDesiredAccess,                      // access mode
                              DWORD dwShareMode,                          // share mode
                              LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                              DWORD dwCreationDisposition,                // how to create
                              DWORD dwFlagsAndAttributes,                 // file attributes
                              HANDLE hTemplateFile                        // handle to template file
                              )
{
    LPWSTR lpNewFileName = (LPWSTR)lpFileName;
    HANDLE handle = pRealCreateFileW(lpNewFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    
    if (lpNewFileName != NULL)
    {
        if (handle == INVALID_HANDLE_VALUE)
        {
            lpNewFileName = (LPWSTR)AllocMemory(wcslen(lpFileName)+MAX_PATH);
            wcscpy(lpNewFileName, lpFileName);
            wcscat(lpNewFileName, L".mtk");
            handle = pRealCreateFileW(lpNewFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
            if (handle == INVALID_HANDLE_VALUE)
            {
                FreeMemory(lpNewFileName);
                return handle;
            }
        }

        LPCWSTR lpext = PathFindExtensionW(lpNewFileName);
        if (lpext != NULL && _wcsicmp(lpext, L".mtk") == 0  )
        {
            LARGE_INTEGER la = {0};
            pRealGetFileSizeEx(handle, &la);
            InsertFileHandle(handle, la.QuadPart, la.QuadPart%250);
        }

        if (lpNewFileName != lpFileName)
        {
            FreeMemory(lpNewFileName);
        }
    }

    return handle;
}


// 写函数hook 
INT WINAPI EncodeCodeBuf(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
    return pRealWriteFile(hFile, lpBuffer,  nNumberOfBytesToWrite,  lpNumberOfBytesWritten, lpOverlapped);
}


BOOL WINAPI FileFilterInitialize()
{
	_InitializeListHead(&Global_EncryptFile.next);
    _InitializeListHead(&Global_EncryptMap.next);
	
    InitializeCriticalSection(&g_Encrypt_File_cs);
    InitializeCriticalSection(&g_Encrypt_Map_cs);


    DetourTransactionBegin();  
    DetourUpdateThread(GetCurrentThread());  
    DetourAttach(&(PVOID&)pRealCreateFileW, HookCreateFileW);  
    DetourAttach(&(PVOID&)pRealGetFileSizeEx, HookGetFileSizeEx);  
    DetourAttach(&(PVOID&)pRealGetFileSize, HookGetFileSize);  
    DetourAttach(&(PVOID&)pRealSetFilePointerEx, HookSetFilePointerEx);  
    DetourAttach(&(PVOID&)pRealSetFilePointer, HookSetFilePointer);  
    DetourAttach(&(PVOID&)pRealReadFile, HookReadFile);  
    DetourAttach(&(PVOID&)pRealWriteFile, HookWriteFile);  
    DetourAttach(&(PVOID&)pRealCloseHandle, HookCloseHandle);  
    DetourAttach(&(PVOID&)pRealCreateFileMappingW, HookCreateFileMappingW);  
    DetourAttach(&(PVOID&)pRealCreateFileMappingA, HookCreateFileMappingA);  
    DetourAttach(&(PVOID&)pRealMapViewOfFileEx, HookMapViewOfFileEx);  
    DetourAttach(&(PVOID&)pRealMapViewOfFile, HookMapViewOfFile);  
    DetourAttach(&(PVOID&)pRealUnmapViewOfFile, HookUnmapViewOfFile);  
    DetourTransactionCommit();  
    return TRUE;
}


BOOL WINAPI FileFilterRelease()
{

    DetourTransactionBegin();  
    DetourUpdateThread(GetCurrentThread());  
    DetourDetach(&(PVOID&)pRealCreateFileW, HookCreateFileW);  
    DetourDetach(&(PVOID&)pRealGetFileSizeEx, HookGetFileSizeEx);  
    DetourDetach(&(PVOID&)pRealGetFileSize, HookGetFileSize);  
    DetourDetach(&(PVOID&)pRealSetFilePointerEx, HookSetFilePointerEx);  
    DetourDetach(&(PVOID&)pRealSetFilePointer, HookSetFilePointer);  
    DetourDetach(&(PVOID&)pRealReadFile, HookReadFile);  
    DetourDetach(&(PVOID&)pRealWriteFile, HookWriteFile);  
    DetourDetach(&(PVOID&)pRealCloseHandle, HookCloseHandle);  
    DetourDetach(&(PVOID&)pRealCreateFileMappingW, HookCreateFileMappingW);  
    DetourDetach(&(PVOID&)pRealCreateFileMappingA, HookCreateFileMappingA);  
    DetourDetach(&(PVOID&)pRealMapViewOfFileEx, HookMapViewOfFileEx);  
    DetourDetach(&(PVOID&)pRealMapViewOfFile, HookMapViewOfFile);  
    DetourDetach(&(PVOID&)pRealUnmapViewOfFile, HookUnmapViewOfFile);  
    DetourTransactionCommit();  


    DeleteCriticalSection(&g_Encrypt_Map_cs);
    DeleteCriticalSection(&g_Encrypt_File_cs);
    return TRUE;
}





















#pragma region Not_Need_ChangeCode

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// CreateFile 链表
BOOL WINAPI FileHandleInList(HANDLE handle)
{
    // CreateFile 表
    EnterCriticalSection(&g_Encrypt_File_cs);
    if ( !_IsListEmpty(&Global_EncryptFile.next) )
    {
        for (PLIST_ENTRY plist = Global_EncryptFile.next.Flink; plist != &Global_EncryptFile.next; plist = plist->Flink)
        {
            PEncryptFileList phandlelist = CONTAINING_RECORD(plist, EncryptFileList, next);
            if (phandlelist->hCreateHandle == handle)
            {
                LeaveCriticalSection(&g_Encrypt_File_cs);
                return TRUE;
            }
        }
    }
    LeaveCriticalSection(&g_Encrypt_File_cs);
    return FALSE;
}


BOOL WINAPI MapHandleInList(HANDLE handle)
{
    // CreateFileMapping 表
    EnterCriticalSection(&g_Encrypt_Map_cs);
    if ( !_IsListEmpty(&Global_EncryptMap.next) )
    {
        for (PLIST_ENTRY plist = Global_EncryptMap.next.Flink; plist != &Global_EncryptMap.next; plist = plist->Flink)
        {
            PEncryptFileList phandlelist = CONTAINING_RECORD(plist, EncryptFileList, next);
            if (phandlelist->hCreateHandle == handle)
            {
                LeaveCriticalSection(&g_Encrypt_Map_cs);
                return TRUE;
            }
        }
    }
    LeaveCriticalSection(&g_Encrypt_Map_cs);

    return FALSE;
}



VOID WINAPI DelFileHandle(HANDLE handle)
{
    // CreateFile 表
    EnterCriticalSection(&g_Encrypt_File_cs);
    if ( !_IsListEmpty(&Global_EncryptFile.next) )
    {
        for (PLIST_ENTRY plist = Global_EncryptFile.next.Flink; plist != &Global_EncryptFile.next; plist = plist->Flink)
        {
            PEncryptFileList phandlelist = CONTAINING_RECORD(plist, EncryptFileList, next);
            if (phandlelist->hCreateHandle == handle)
            {
                _RemoveEntryList(plist);
                FreeMemory(phandlelist);
                LeaveCriticalSection(&g_Encrypt_File_cs);
                return;
            }
        }
    }
    LeaveCriticalSection(&g_Encrypt_File_cs);
}


VOID WINAPI DelMapHandle(HANDLE handle)
{
    // CreateFileMapping 表
    EnterCriticalSection(&g_Encrypt_Map_cs);
    if ( !_IsListEmpty(&Global_EncryptMap.next) )
    {
        for (PLIST_ENTRY plist = Global_EncryptMap.next.Flink; plist != &Global_EncryptMap.next; plist = plist->Flink)
        {
            PEncryptFileList phandlelist = CONTAINING_RECORD(plist, EncryptFileList, next);
            if (phandlelist->hCreateHandle == handle)
            {
                _RemoveEntryList(plist);
                FreeMemory(phandlelist);
                LeaveCriticalSection(&g_Encrypt_Map_cs);
                return;
            }
        }
    }
    LeaveCriticalSection(&g_Encrypt_Map_cs);
}

//////////////////////////////////////////////////////////////////////////
// 插入列表 
BOOL WINAPI InsertFileHandle( HANDLE hFile, LONGLONG llFileSize, BYTE bKey)
{
    PEncryptFileList list = (PEncryptFileList)AllocMemory(sizeof(EncryptFileList));

    if (list != NULL)
    {
        list->bKey = bKey;
        list->hCreateHandle = hFile;
        list->laFileSize.QuadPart = llFileSize;

        EnterCriticalSection(&g_Encrypt_File_cs);
        _InsertTailList(&Global_EncryptFile.next, &list->next);
        LeaveCriticalSection(&g_Encrypt_File_cs);

        return TRUE;
    }
    return FALSE;    
}

BOOL WINAPI InsertMapHandle( HANDLE hFile, LONGLONG llFileSize, BYTE bKey)
{
    PEncryptFileList list = (PEncryptFileList)AllocMemory(sizeof(EncryptFileList));

    if (list != NULL)
    {
        list->bKey = bKey;
        list->hCreateHandle = hFile;
        list->laFileSize.QuadPart = llFileSize;

        EnterCriticalSection(&g_Encrypt_Map_cs);
        _InsertTailList(&Global_EncryptMap.next, &list->next);
        LeaveCriticalSection(&g_Encrypt_Map_cs);

        return TRUE;
    }
    return FALSE;    
}


//////////////////////////////////////////////////////////////////////////
// 获取列表信息 
BYTE WINAPI GetFileKeyAndSize(HANDLE handle, PLARGE_INTEGER laFileSize)
{
    // CreateFile 表
    EnterCriticalSection(&g_Encrypt_File_cs);
    if ( !_IsListEmpty(&Global_EncryptFile.next) )
    {
        for (PLIST_ENTRY plist = Global_EncryptFile.next.Flink; plist != &Global_EncryptFile.next; plist = plist->Flink)
        {
            PEncryptFileList phandlelist = CONTAINING_RECORD(plist, EncryptFileList, next);
            if (phandlelist->hCreateHandle == handle)
            {
                BYTE bKey = phandlelist->bKey;
                if (laFileSize)
                {
                    laFileSize->QuadPart = phandlelist->laFileSize.QuadPart;
                }
                LeaveCriticalSection(&g_Encrypt_File_cs);
                return bKey;
            }
        }
    }
    LeaveCriticalSection(&g_Encrypt_File_cs);
    return -1;
}

BYTE WINAPI GetMapKeyAndSize(HANDLE handle, PLARGE_INTEGER laFileSize)
{
    // CreateFileMapping 表
    EnterCriticalSection(&g_Encrypt_Map_cs);
    if ( !_IsListEmpty(&Global_EncryptMap.next) )
    {
        for (PLIST_ENTRY plist = Global_EncryptMap.next.Flink; plist != &Global_EncryptMap.next; plist = plist->Flink)
        {
            PEncryptFileList phandlelist = CONTAINING_RECORD(plist, EncryptFileList, next);
            if (phandlelist->hCreateHandle == handle)
            {
                BYTE bKey = phandlelist->bKey;
                if (laFileSize)
                {
                    laFileSize->QuadPart = phandlelist->laFileSize.QuadPart;
                }
                LeaveCriticalSection(&g_Encrypt_Map_cs);
                return bKey;
            }
        }
    }
    LeaveCriticalSection(&g_Encrypt_Map_cs);
    return -1;
}


//////////////////////////////////////////////////////////////////////////
// 获取文件大小
BOOL  WINAPI HookGetFileSizeEx( HANDLE hFile, PLARGE_INTEGER lpFileSize )
{
    if( FileHandleInList(hFile) && lpFileSize != NULL )
    {
        GetFileKeyAndSize(hFile, lpFileSize);
        return TRUE;
    }
    else
    {
        return pRealGetFileSizeEx(hFile, lpFileSize);
    }
}

DWORD WINAPI HookGetFileSize( HANDLE hFile, LPDWORD lpFileSizeHigh )
{
    LARGE_INTEGER laFileSize = {0};
    if ( HookGetFileSizeEx(hFile, &laFileSize) )
    {
        if (lpFileSizeHigh)
        {
            *lpFileSizeHigh = laFileSize.HighPart;
        }
        return laFileSize.LowPart;
    }

    if (lpFileSizeHigh)
    {
        *lpFileSizeHigh = -1;
    }
    return -1;
}

// 设置文件偏移
BOOL WINAPI HookSetFilePointerEx(  HANDLE hFile,                    // handle to file
                                 LARGE_INTEGER liDistanceToMove,  // bytes to move pointer
                                 PLARGE_INTEGER lpNewFilePointer, // new file pointer
                                 DWORD dwMoveMethod               // starting point
                                 )
{
    if ( FileHandleInList(hFile) && dwMoveMethod == FILE_END)
    {
        LARGE_INTEGER laFileSize = {0};
        LARGE_INTEGER laRealSize = {0};
        if( HookGetFileSizeEx(hFile, &laFileSize) &&  pRealGetFileSizeEx(hFile, &laRealSize) )
        {
            // 加密前文件比现在文件要短 
            if (laFileSize.QuadPart < laRealSize.QuadPart)
            {
                liDistanceToMove.QuadPart -= (laRealSize.QuadPart - laFileSize.QuadPart); 
            }

            // 如果长 
        }
    }
    return pRealSetFilePointerEx(hFile, liDistanceToMove, lpNewFilePointer, dwMoveMethod);
}

DWORD WINAPI HookSetFilePointer(  HANDLE hFile,                // handle to file
                                LONG lDistanceToMove,        // bytes to move pointer
                                PLONG lpDistanceToMoveHigh,  // bytes to move pointer
                                DWORD dwMoveMethod           // starting point
                                )
{
    LARGE_INTEGER laToMove = {0};
    LARGE_INTEGER laCurrent = {0};
    if (lpDistanceToMoveHigh)
    {
        laToMove.HighPart = *lpDistanceToMoveHigh;
    }

    laToMove.LowPart = lDistanceToMove;
    if( HookSetFilePointerEx(hFile, laToMove, &laCurrent, dwMoveMethod) )
    {
        if (lpDistanceToMoveHigh)
        {
            *lpDistanceToMoveHigh = laCurrent.HighPart;
        }
//        SetLastError(NO_ERROR);
        return laCurrent.LowPart;
    } 
    // ERROR_NEGATIVE_SEEK
    return INVALID_SET_FILE_POINTER;
}


DWORD WINAPI DecryptCodeBuf(
                            HANDLE hFile, 
                            LPBYTE pbuf, 
                            DWORD dwFileOffsetHigh, 
                            DWORD dwFileOffsetLow, 
                            SIZE_T dwNumberOfBytesToMap, 
                            BOOL IsMapFile
                            )
{
    LARGE_INTEGER la = {0};
    BOOL bKey = 0;
    if (IsMapFile)
    {
        // Map 的内容
        bKey = GetMapKeyAndSize(hFile, &la);
    }
    else
    {
        // CreateFile 内容 
        bKey = GetFileKeyAndSize(hFile, &la);
    }

    if (dwNumberOfBytesToMap == 0 )
    {
        if (IsMapFile)
        {
            // 这个是map的整个文件啦 
            DWORD nCount = ((la.QuadPart > TOTAL_NUMBER) ? TOTAL_NUMBER : la.LowPart);
            RealDecode(pbuf, 0, nCount, bKey);
        }
    }
    else
    {
        if ((dwFileOffsetLow < TOTAL_NUMBER) && (dwFileOffsetHigh == 0))
        {
            DWORD left = TOTAL_NUMBER - dwFileOffsetLow;
            DWORD nCount = ((dwNumberOfBytesToMap > left) ? left : dwNumberOfBytesToMap);
            RealDecode(pbuf, dwFileOffsetLow, nCount, bKey);
        }
    }
    return 0;
}

BOOL WINAPI HookReadFile(  
                            HANDLE hFile,                // handle to file
                            LPVOID lpBuffer,             // data buffer
                            DWORD nNumberOfBytesToRead,  // number of bytes to read
                            LPDWORD lpNumberOfBytesRead, // number of bytes read
                            LPOVERLAPPED lpOverlapped    // overlapped buffer
                            )
{
    
    
    if ( FileHandleInList(hFile) )
    {
        // 先获取当前位置 
        LONG high = 0;
        DWORD dwRead = 0;
        DWORD low = SetFilePointer(hFile, 0, &high, FILE_CURRENT);

        if( pRealReadFile(hFile, lpBuffer,  nNumberOfBytesToRead,  &dwRead,  lpOverlapped) && dwRead > 0)
        {
            DecryptCodeBuf(hFile, (LPBYTE)lpBuffer, high, low, dwRead, FALSE);
            if (lpNumberOfBytesRead != NULL)
            {
                *lpNumberOfBytesRead = dwRead;
            }
            return TRUE;
        }
        return FALSE;        
    }

    return pRealReadFile(hFile, lpBuffer,  nNumberOfBytesToRead,  lpNumberOfBytesRead,  lpOverlapped);
}



BOOL WINAPI HookWriteFile( 
                          HANDLE hFile,                    // handle to file
                          LPCVOID lpBuffer,                // data buffer
                          DWORD nNumberOfBytesToWrite,     // number of bytes to write
                          LPDWORD lpNumberOfBytesWritten,  // number of bytes written
                          LPOVERLAPPED lpOverlapped        // overlapped buffer
                          )
{
    if ( FileHandleInList(hFile) )
    {
        return EncodeCodeBuf(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
    }
    return pRealWriteFile(hFile, lpBuffer,  nNumberOfBytesToWrite,  lpNumberOfBytesWritten, lpOverlapped);
}


BOOL WINAPI HookCloseHandle(HANDLE hObject)
{
    if ( FileHandleInList(hObject) )
    {
        DelFileHandle(hObject);
    }
    else if ( MapHandleInList(hObject) )
    {
        DelMapHandle(hObject);
    }
    return pRealCloseHandle(hObject);
}



HANDLE WINAPI HookCreateFileMappingW(  
                                     HANDLE hFile,                       // handle to file
                                     LPSECURITY_ATTRIBUTES lpAttributes, // security
                                     DWORD flProtect,                    // protection
                                     DWORD dwMaximumSizeHigh,            // high-order DWORD of size
                                     DWORD dwMaximumSizeLow,             // low-order DWORD of size
                                     LPCWSTR lpName                      // object name
                                     )
{
    BYTE bKey;
    DWORD dwValue;
    LARGE_INTEGER laMap = {0};
    
    HANDLE hMapHandle = pRealCreateFileMappingW(
                                            hFile, 
                                            lpAttributes, 
                                            flProtect, 
                                            dwMaximumSizeHigh, 
                                            dwMaximumSizeLow, 
                                            lpName);
    dwValue = GetLastError();

    // 失败
    if ( hMapHandle == NULL )
    {
        return NULL;
    }

    if ( MapHandleInList(hMapHandle) )
    {
        DelMapHandle(hMapHandle);
    }

    // 是"共享内存" 或者 "不是加密文件"  或者  "已经映射过了"
    if (hFile == INVALID_HANDLE_VALUE || !FileHandleInList(hFile) || dwValue == ERROR_ALREADY_EXISTS)
    {
        return hMapHandle;
    }


    // 插入Map列表 
    bKey = GetFileKeyAndSize(hFile, &laMap);
    InsertMapHandle(hMapHandle, laMap.QuadPart, bKey);

    return hMapHandle;
}

// 0026DEAC   000003F8  |hFile = 000003F8 (window)
// 0026DEB0   00000000  |pSecurity = NULL
// 0026DEB4   00000002  |Protection = PAGE_READONLY
// 0026DEB8   00000000  |MaximumSizeHigh = 0x0
// 0026DEBC   00000000  |MaximumSizeLow = 0x0
// 0026DEC0   00000000  \MapName = NULL
HANDLE WINAPI HookCreateFileMappingA(  
                                     HANDLE hFile,                       // handle to file
                                     LPSECURITY_ATTRIBUTES lpAttributes, // security
                                     DWORD flProtect,                    // protection
                                     DWORD dwMaximumSizeHigh,            // high-order DWORD of size
                                     DWORD dwMaximumSizeLow,             // low-order DWORD of size
                                     LPCSTR lpName                      // object name
                                     )
{
    if (lpName != NULL)
    {
        HANDLE hRethandle = INVALID_HANDLE_VALUE;
        PWSTR  pElementText = NULL;
        int    iTextLen = 0;
        // wide char to multi char
        iTextLen = MultiByteToWideChar( CP_UTF8,
            0,
            lpName,
            -1,
            NULL,
            0 );

        pElementText = (PWSTR)AllocMemory((iTextLen+2)*sizeof(WCHAR));        
        MultiByteToWideChar( CP_UTF8,
            0,
            lpName,
            -1,
            pElementText,
            iTextLen );    

        hRethandle = HookCreateFileMappingW(hFile, 
            lpAttributes, 
            flProtect, 
            dwMaximumSizeHigh, 
            dwMaximumSizeLow, 
            pElementText);

        FreeMemory(pElementText);
        return hRethandle;
    }

    // lpName == NULL
    return HookCreateFileMappingW(hFile, 
        lpAttributes, 
        flProtect, 
        dwMaximumSizeHigh, 
        dwMaximumSizeLow, 
        NULL);
}

LPVOID WINAPI HookMapViewOfFileEx(
                                  HANDLE hFileMappingObject,   // handle to file-mapping object
                                  DWORD dwDesiredAccess,       // access mode
                                  DWORD dwFileOffsetHigh,      // high-order DWORD of offset
                                  DWORD dwFileOffsetLow,       // low-order DWORD of offset
                                  SIZE_T dwNumberOfBytesToMap, // number of bytes to map
                                  LPVOID lpBaseAddress         // starting address
                                  )
{
    BOOL bFileEncrype = MapHandleInList(hFileMappingObject);
    if ( bFileEncrype )
    {
        dwDesiredAccess = FILE_MAP_COPY;
    }

    LPBYTE lpBuf = (LPBYTE)pRealMapViewOfFileEx(  
                                                    hFileMappingObject, 
                                                    dwDesiredAccess,
                                                    dwFileOffsetHigh,
                                                    dwFileOffsetLow,
                                                    dwNumberOfBytesToMap,
                                                    lpBaseAddress);
    if ( lpBuf != NULL && bFileEncrype )
    {
        // 解密,需要内存重新申请吗
        DecryptCodeBuf(hFileMappingObject, lpBuf, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap, TRUE);
    }
    return lpBuf; 
}


// If the function fails, the return value is NULL
LPVOID WINAPI HookMapViewOfFile(
                                HANDLE hFileMappingObject,   // handle to file-mapping object
                                DWORD dwDesiredAccess,       // FILE_MAP_WRITE=FILE_MAP_ALL_ACCESS   FILE_MAP_READ   FILE_MAP_COPY
                                DWORD dwFileOffsetHigh,      // high-order DWORD of offset
                                DWORD dwFileOffsetLow,       // low-order DWORD of offset
                                SIZE_T dwNumberOfBytesToMap  // number of bytes to map
                                )
{
    return HookMapViewOfFileEx(hFileMappingObject, 
        dwDesiredAccess,
        dwFileOffsetHigh, 
        dwFileOffsetLow, 
        dwNumberOfBytesToMap, 
        NULL);
}

BOOL WINAPI HookUnmapViewOfFile( LPCVOID lpBaseAddress )
{
    // 重新申请内存的话需要改写
    return pRealUnmapViewOfFile(lpBaseAddress);
}

#pragma endregion Not_Need_ChangeCode