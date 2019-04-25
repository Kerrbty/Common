#ifndef _FILE_FILTER_HEADER_HEAD_HH_H_
#define _FILE_FILTER_HEADER_HEAD_HH_H_
#include "../defs.h"


typedef HANDLE (WINAPI* pCreateFileW)(
                                      LPCWSTR lpFileName,                         // file name
                                      DWORD dwDesiredAccess,                      // access mode
                                      DWORD dwShareMode,                          // share mode
                                      LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                                      DWORD dwCreationDisposition,                // how to create
                                      DWORD dwFlagsAndAttributes,                 // file attributes
                                      HANDLE hTemplateFile                        // handle to template file
                                      );

typedef DWORD (WINAPI* F_GetFileSize)( HANDLE hFile, LPDWORD lpFileSizeHigh );
typedef BOOL  (WINAPI* F_GetFileSizeEx)( HANDLE hFile, PLARGE_INTEGER lpFileSize );

typedef BOOL (WINAPI* pReadFile)( HANDLE hFile, 
                                 LPVOID lpBuffer, 
                                 DWORD nNumberOfBytesToRead, 
                                 LPDWORD lpNumberOfBytesRead, 
                                 LPOVERLAPPED lpOverlapped);

typedef BOOL(WINAPI* pWriteFile)(
                                 HANDLE hFile,                    // handle to file
                                 LPCVOID lpBuffer,                // data buffer
                                 DWORD nNumberOfBytesToWrite,     // number of bytes to write
                                 LPDWORD lpNumberOfBytesWritten,  // number of bytes written
                                 LPOVERLAPPED lpOverlapped        // overlapped buffer
                                 );

typedef BOOL (WINAPI* pCloseHandle)(
                                    HANDLE hObject   // handle to object
                                    );


typedef HANDLE (WINAPI* F_CreateFileMappingA)(  
                                                HANDLE hFile,                       // handle to file
                                                LPSECURITY_ATTRIBUTES lpAttributes, // security
                                                DWORD flProtect,                    // protection
                                                DWORD dwMaximumSizeHigh,            // high-order DWORD of size
                                                DWORD dwMaximumSizeLow,             // low-order DWORD of size
                                                LPCSTR lpName                      // object name
                                                );

typedef HANDLE (WINAPI* F_CreateFileMappingW)(  
                                                HANDLE hFile,                       // handle to file
                                                LPSECURITY_ATTRIBUTES lpAttributes, // security
                                                DWORD flProtect,                    // protection
                                                DWORD dwMaximumSizeHigh,            // high-order DWORD of size
                                                DWORD dwMaximumSizeLow,             // low-order DWORD of size
                                                LPCWSTR lpName                      // object name
                                                );


// If the function fails, the return value is NULL
typedef LPVOID (WINAPI* F_MapViewOfFile)(
                                            HANDLE hFileMappingObject,   // handle to file-mapping object
                                            DWORD dwDesiredAccess,       // FILE_MAP_WRITE=FILE_MAP_ALL_ACCESS   FILE_MAP_READ   FILE_MAP_COPY
                                            DWORD dwFileOffsetHigh,      // high-order DWORD of offset
                                            DWORD dwFileOffsetLow,       // low-order DWORD of offset
                                            SIZE_T dwNumberOfBytesToMap  // number of bytes to map
                                            );

typedef LPVOID (WINAPI* F_MapViewOfFileEx)(
                                            HANDLE hFileMappingObject,   // handle to file-mapping object
                                            DWORD dwDesiredAccess,       // access mode
                                            DWORD dwFileOffsetHigh,      // high-order DWORD of offset
                                            DWORD dwFileOffsetLow,       // low-order DWORD of offset
                                            SIZE_T dwNumberOfBytesToMap, // number of bytes to map
                                            LPVOID lpBaseAddress         // starting address
                                            );

typedef BOOL (WINAPI* F_SetFilePointerEx)(  HANDLE hFile,                    // handle to file
                                          LARGE_INTEGER liDistanceToMove,  // bytes to move pointer
                                          PLARGE_INTEGER lpNewFilePointer, // new file pointer
                                          DWORD dwMoveMethod               // starting point
                                          );

typedef DWORD (WINAPI* F_SetFilePointer)(  HANDLE hFile,                // handle to file
                                         LONG lDistanceToMove,        // bytes to move pointer
                                         PLONG lpDistanceToMoveHigh,  // bytes to move pointer
                                         DWORD dwMoveMethod           // starting point
                                         );

typedef BOOL (WINAPI* F_UnmapViewOfFile)( LPCVOID lpBaseAddress );




#ifdef __cplusplus
extern "C" {
#endif

// Hook”√µƒ…Í√˜ 
HANDLE WINAPI HookCreateFileW(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
BOOL   WINAPI HookGetFileSizeEx( HANDLE, PLARGE_INTEGER );
DWORD  WINAPI HookGetFileSize( HANDLE, LPDWORD );
BOOL   WINAPI HookSetFilePointerEx(HANDLE, LARGE_INTEGER, PLARGE_INTEGER, DWORD);
DWORD  WINAPI HookSetFilePointer(HANDLE, LONG, PLONG, DWORD);
BOOL   WINAPI HookReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
BOOL   WINAPI HookWriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
BOOL   WINAPI HookCloseHandle(HANDLE);
HANDLE WINAPI HookCreateFileMappingW(HANDLE, LPSECURITY_ATTRIBUTES, DWORD, DWORD, DWORD, LPCWSTR);
HANDLE WINAPI HookCreateFileMappingA(HANDLE, LPSECURITY_ATTRIBUTES, DWORD, DWORD, DWORD, LPCSTR);
LPVOID WINAPI HookMapViewOfFileEx(HANDLE, DWORD, DWORD, DWORD, SIZE_T, LPVOID);
LPVOID WINAPI HookMapViewOfFile(HANDLE, DWORD, DWORD, DWORD, SIZE_T);
BOOL   WINAPI HookUnmapViewOfFile(LPCVOID);





BOOL WINAPI FileFilterInitialize();
BOOL WINAPI FileFilterRelease();





#ifdef __cplusplus
};
#endif

#endif  // _FILE_FILTER_HEADER_HEAD_HH_H_