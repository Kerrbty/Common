#ifndef _MY_HOOK_API_
#define _MY_HOOK_API_
#define MAX_HOOK_NUM   256

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include "xde32/xde.h"


BOOL HookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcName); // hookָ����dll�еĵ�������
BOOL HookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr); // hookָ���ĵ�ַ

DWORD WINAPI MyHookAll(LPVOID); // DllMain����HOOK�õ�����
DWORD WINAPI MyUnHookAll(LPVOID); // ���hook����

BOOL UnHookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr);
BOOL UnHookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcAddr); // ����unhook

DWORD WINAPI GetNewAddress(PVOID MyAddr); // �õ�ԭ�������ĵ�ַ������

typedef struct _FunAddress 
{
	DWORD MyFunAddr;   // ����hook������ַ
	DWORD NewMalloc;   // ���Ƴ������׵�ַ������ַ
}FunAddress, *PFunAddress;


#endif // _MY_HOOK_API_