#ifndef _MY_HOOK_API_
#define _MY_HOOK_API_
#define MAX_HOOK_NUM   256
#define _USE_JMP_ 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include "lde32.h"

#ifndef _EXPORT_COMMON_CODE_
    #ifdef _DEBUG
        #pragma comment(lib, "myhook_d")
    #else
        #pragma comment(lib, "myhook")
    #endif
#endif

// ����ͷ�� Inline Hook 
BOOL HookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcName); // hookָ����dll�еĵ�������
BOOL HookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr, LPVOID* NewStubAddr = NULL); // hookָ���ĵ�ַ
BOOL HookProc(LPVOID addr, LPVOID MyProcAddr);  // ���� DetourAttach 

// �����ַInline Hook 
typedef struct _context{
    DWORD Edi;
    DWORD Esi;
    DWORD Ebp;
    DWORD Esp;
    DWORD Ebx;
    DWORD Edx;
    DWORD Ecx;
    DWORD Eax;
    DWORD dwflags;
    DWORD others[1]; // hookͷ���Ļ�����һ�������Ƿ��ص�ַ������ȥ�ǲ��� 
}context, *pcontext;
typedef VOID (WINAPI* PMiddleSubProc)(pcontext pctx);
BOOL HookMiddleProc(LPVOID lpTargetAddress, PMiddleSubProc MyProcAddr);
BOOL UnHookMiddleProc(LPVOID lpTargetAddress, PMiddleSubProc MyProcAddr);


BOOL UnHookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr);
BOOL UnHookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcAddr); // ����unhook
BOOL UnHookProc(LPVOID addr, LPVOID MyProcAddr);


LPVOID WINAPI GetNewAddress(PVOID MyAddr); // �õ�ԭ�������ĵ�ַ������


typedef struct _FunAddress 
{
    LPBYTE HookedFunAddr; // ��hook������ַ
    LPBYTE MyFunAddr;   // ����hook������ַ
    LPBYTE NewMalloc;   // �µ�ת��Stub�����ַ
    LPBYTE srcByte;     // ԭʼ����
    DWORD srclen;      // ԭʼ���ݳ���
}FunAddress, *PFunAddress;


#endif // _MY_HOOK_API_