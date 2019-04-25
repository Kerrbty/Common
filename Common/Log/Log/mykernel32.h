#pragma once
#ifndef _LGJ_MY_KERNEL32_H_
#define _LGJ_MY_KERNEL32_H_
#include <windows.h>
#pragma comment(lib, "mykernel32.lib")


// ��̬����\�곣������
extern "C" const IMAGE_DOS_HEADER __ImageBase; // ָ��ָ��ģ����׵�ַ,������ָ����


#if _MSC_VER < 1300
// GetModuleHandleEx ����flags����ֵ
#define  GET_MODULE_HANDLE_EX_FLAG_PIN                  (0x01)  // ģ��һֱӳ���ڵ��øú����Ľ����У�ֱ���ý��̽��������ܵ��ö��ٴ�FreeLibrary
#define  GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT   (0x02)  // �� GetModuleHandle��ͬ�����������ü���
#define  GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS         (0x04)  // lpModuleName��ģ���е�һ����ַ


typedef struct _MEMORYSTATUSEX {
    DWORD dwLength; 
    DWORD dwMemoryLoad; 
    DWORDLONG ullTotalPhys; 
    DWORDLONG ullAvailPhys; 
    DWORDLONG ullTotalPageFile; 
    DWORDLONG ullAvailPageFile; 
    DWORDLONG ullTotalVirtual; 
    DWORDLONG ullAvailVirtual; 
    DWORDLONG ullAvailExtendedVirtual;
} MEMORYSTATUSEX, *LPMEMORYSTATUSEX; 
#endif

#ifdef __cplusplus  
extern "C" {  
#endif  
#pragma warning(disable:4273)

// API ��������
BOOL WINAPI DebugSetProcessKillOnExit( BOOL KillOnExit ); // ���õ������˳�ʱ�������Գ����Ƿ��˳�

BOOL WINAPI DebugActiveProcessStop(DWORD PID); // ����������뱻���Գ���Ĺ���

#if _MSC_VER < 1300
// ���ݺ������õ��������ڵ�ģ���ַ
BOOL WINAPI GetModuleHandleExA(DWORD flags, 
							   PCHAR FuncName, 
							   HMODULE* hModule); 

// ���ݺ������õ��������ڵ�ģ���ַ
BOOL WINAPI GetModuleHandleExW(DWORD flags, 
							   PWCHAR FuncName, 
							   HMODULE* hModule); 

BOOL WINAPI GlobalMemoryStatusEx(
						  LPMEMORYSTATUSEX lpBuffer   // memory status structure
						  );
						  
HANDLE WINAPI OpenThread( DWORD dwDesiredAccess,  // access right
						  BOOL bInheritHandle,    // handle inheritance option
						  DWORD dwThreadId        // thread identifier
						  );
  

HANDLE WINAPI OpenConsoleW(
							LPCWSTR lpFileName, 
							DWORD dwDesiredAccess, 
							BOOL bInitialOwner, 
							DWORD dwShareMode
						  );
#endif
// �ڿ��ֽ���խ�ֽ��ж���ͳһ�ӿ�
#ifdef _UNICODE

#define GetModuleHandleEx GetModuleHandleExW

#else

#define GetModuleHandleEx GetModuleHandleExA

#endif















   
#ifdef __cplusplus  
};  
#endif 

#endif // _LGJ_MY_KERNEL32_H_