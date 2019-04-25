#include "Hook.h"
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

// _ReturnAddress and _AddressOfReturnAddress should be prototyped before use 
EXTERNC void * _AddressOfReturnAddress(void);
EXTERNC void * _ReturnAddress(void);

#pragma intrinsic(_AddressOfReturnAddress)
#pragma intrinsic(_ReturnAddress)


typedef HANDLE (WINAPI *PCreateFileA)(LPSTR lpFileName,                         // file name
                                      DWORD dwDesiredAccess,                      // access mode
                                      DWORD dwShareMode,                          // share mode
                                      LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                                      DWORD dwCreationDisposition,                // how to create
                                      DWORD dwFlagsAndAttributes,                 // file attributes
                                      HANDLE hTemplateFile                        // handle to template file
                                      );

typedef HANDLE (WINAPI *PCreateFileW)(LPWSTR lpFileName,                         // file name
                                      DWORD dwDesiredAccess,                      // access mode
                                      DWORD dwShareMode,                          // share mode
                                      LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                                      DWORD dwCreationDisposition,                // how to create
                                      DWORD dwFlagsAndAttributes,                 // file attributes
                                      HANDLE hTemplateFile                        // handle to template file
                                      );

// 这里定义
// hook后的处理函数
// 处理函数比原函数参数多1个，就是第一个参数调用的来源函数（返回地址）
HANDLE _cdecl ComplagteCreateFileW(
                                  LPWSTR lpFileName,                         // file name
                                  DWORD dwDesiredAccess,                      // access mode
                                  DWORD dwShareMode,                          // share mode
                                  LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                                  DWORD dwCreationDisposition,                // how to create
                                  DWORD dwFlagsAndAttributes,                 // file attributes
                                  HANDLE hTemplateFile                        // handle to template file
                                  )
{
        ULONG_PTR* addri = (ULONG_PTR*)_ReturnAddress(); // 得到返回地址

//        if (wcsstr(lpFileName, L"Registry.db") != NULL )
        {
                // 找到调用地方
                HMODULE hmodule; 
                WCHAR FileName[MAX_PATH] = {0}; 
                GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (PTSTR)addri, &hmodule); 
                DWORD errorcode = GetLastError();
                GetModuleFileNameW(hmodule, FileName, MAX_PATH); 
                
                LPWSTR writestr = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH*3*sizeof(WCHAR)); 
                
                wsprintfW(        writestr, 
                        L"调用 CreateFileW 来自于:%s\r\n"
                        L" Module address:%08X, Base Offset:%08X, Memory Address:%08X \r\n" 
                        L"打开文件: %s\r\n\r\n", 
                        FileName, hmodule, (LPBYTE)addri - (LPBYTE)hmodule, addri, lpFileName); 
                
                OutputDebugStringW(writestr); 
                
                HeapFree(GetProcessHeap(), NULL, writestr); 
        }
        PCreateFileW dwCreateFile= (PCreateFileW)CHOOK::GetNewAddress(ComplagteCreateFileW);
        return dwCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}


HANDLE _cdecl ComplagteCreateFileA(
                                   LPSTR lpFileName,                         // file name
                                   DWORD dwDesiredAccess,                      // access mode
                                   DWORD dwShareMode,                          // share mode
                                   LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                                   DWORD dwCreationDisposition,                // how to create
                                   DWORD dwFlagsAndAttributes,                 // file attributes
                                   HANDLE hTemplateFile                        // handle to template file
                                   )
{
        ULONG_PTR* addri = (ULONG_PTR*)_ReturnAddress(); // 得到返回地址

//        if (strstr(lpFileName, "Registry.db") != NULL )
        {
                // 找到调用地方
                HMODULE hmodule;
                char FileName[MAX_PATH] = {0};
                GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (PTSTR)addri, &hmodule);
                GetModuleFileNameA(hmodule, FileName, MAX_PATH);
                
                LPSTR writestr = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH*3);
                
                wsprintfA(writestr, 
                        "调用 CreateFileW 来自于:%s\r\n"
                        " Module address:%08X, Base Offset:%08X, Memory Address:%08X \r\n"
                        "打开文件: %s\r\n\r\n", 
                        FileName, hmodule, (LPBYTE)addri - (LPBYTE)hmodule, addri, lpFileName);
                OutputDebugStringA( writestr );
                
                HeapFree(GetProcessHeap(), NULL, writestr);
        }
        PCreateFileA dwCreateFile= (PCreateFileA)CHOOK::GetNewAddress(ComplagteCreateFileA);
        return dwCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

// hook函数
DWORD WINAPI CHOOK::MyHookAll(LPVOID)
{
//	OutputDebugStringA("Hook called!");
    HookProcByName(TEXT("kernel32.dll"), "CreateFileA", ComplagteCreateFileA);
    HookProcByName(TEXT("kernel32.dll"), "CreateFileW", ComplagteCreateFileW);
	return 0;
}



DWORD WINAPI CHOOK::MyUnHookAll(LPVOID)
{
//	OutputDebugStringA("UnHook called!");
	UnHookProcByName(TEXT("kernel32.dll"), "CreateFileA", ComplagteCreateFileA);
	UnHookProcByName(TEXT("kernel32.dll"), "CreateFileW", ComplagteCreateFileW);
	return 0;
}