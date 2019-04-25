#ifndef _MY_HOOK_API_
#define _MY_HOOK_API_
#define MAX_HOOK_NUM   256

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>

namespace CHOOK{
    BOOL HookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcName); // hookָ����dll�еĵ�������
    BOOL HookProcByAddress(LPVOID ProcAddress, LPVOID MyProcAddr); // hookָ���ĵ�ַ

    DWORD WINAPI MyHookAll(LPVOID); // DllMain����HOOK�õ�����
    DWORD WINAPI MyUnHookAll(LPVOID); // ���hook����

    BOOL UnHookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr);
    BOOL UnHookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcAddr); // ����unhook

    LPVOID WINAPI GetNewAddress(PVOID MyAddr); // �õ�ԭ�������ĵ�ַ������

    typedef struct _FunAddress 
    {
        LPVOID MyFunAddr;   // ����hook������ַ
        LPVOID NewMalloc;   // ���Ƴ������׵�ַ������ַ
    }FunAddress, *PFunAddress;
    extern DWORD NowFunNum;
};


#endif // _MY_HOOK_API_