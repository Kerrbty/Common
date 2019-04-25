#ifndef _API_INTERFACE_H_LGJ_2015_03_04_
#define _API_INTERFACE_H_LGJ_2015_03_04_
#include <windows.h>

#ifndef _UNICODE
#define API_CreateFile API_CreateFileA
#else
#define API_CreateFile API_CreateFileW
#endif


HANDLE WINAPI API_CreateFileA(  
                              LPCSTR lpFileName,                         // file name
                              DWORD dwDesiredAccess,                      // access mode
                              DWORD dwShareMode,                          // share mode
                              LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                              DWORD dwCreationDisposition,                // how to create
                              DWORD dwFlagsAndAttributes,                 // file attributes
                              HANDLE hTemplateFile                        // handle to template file
                             );


HANDLE WINAPI API_CreateFileW(  
                              LPCWSTR lpFileName,                         // file name
                              DWORD dwDesiredAccess,                      // access mode
                              DWORD dwShareMode,                          // share mode
                              LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                              DWORD dwCreationDisposition,                // how to create
                              DWORD dwFlagsAndAttributes,                 // file attributes
                              HANDLE hTemplateFile                        // handle to template file
                             );



BOOL WINAPI API_ReadFile(  
                         HANDLE hFile,                // handle to file
                         LPVOID lpBuffer,             // data buffer
                         DWORD nNumberOfBytesToRead,  // number of bytes to read
                         LPDWORD lpNumberOfBytesRead, // number of bytes read
                         LPOVERLAPPED lpOverlapped    // overlapped buffer
                        );



BOOL WINAPI API_WriteFile(  
                          HANDLE hFile,                    // handle to file
                          LPCVOID lpBuffer,                // data buffer
                          DWORD nNumberOfBytesToWrite,     // number of bytes to write
                          LPDWORD lpNumberOfBytesWritten,  // number of bytes written
                          LPOVERLAPPED lpOverlapped        // overlapped buffer
                         );


BOOL WINAPI API_CloseHandle(HANDLE hObject);


DWORD WINAPI API_SetFilePointer(  
                                HANDLE hFile,                // handle to file
                                LONG lDistanceToMove,        // bytes to move pointer
                                PLONG lpDistanceToMoveHigh,  // bytes to move pointer
                                DWORD dwMoveMethod           // starting point
                                );


BOOL WINAPI API_SetFilePointerEx(  
                                 HANDLE hFile,                    // handle to file
                                 LARGE_INTEGER liDistanceToMove,  // bytes to move pointer
                                 PLARGE_INTEGER lpNewFilePointer, // new file pointer
                                 DWORD dwMoveMethod               // starting point
                                 );

#endif // _API_INTERFACE_H_LGJ_2015_03_04_