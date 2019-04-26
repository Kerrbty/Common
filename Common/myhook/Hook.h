#ifndef _MY_HOOK_API_
#define _MY_HOOK_API_
#define MAX_HOOK_NUM   256
#define _USE_JMP_ 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include "lde32.h"


BOOL HookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcName); // hook指定的dll中的导出函数
BOOL HookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr, LPVOID* NewStubAddr = NULL); // hook指定的地址
BOOL HookProc(LPVOID addr, LPVOID MyProcAddr);  // 类似 DetourAttach 


BOOL UnHookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr);
BOOL UnHookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcAddr); // 进行unhook
BOOL UnHookProc(LPVOID addr, LPVOID MyProcAddr);


LPVOID WINAPI GetNewAddress(PVOID MyAddr); // 得到原来函数的地址并返回


typedef struct _FunAddress 
{
    LPBYTE HookedFunAddr; // 被hook函数地址
	LPBYTE MyFunAddr;   // 处理hook函数地址
	LPBYTE NewMalloc;   // 新的转接Stub代码地址
	LPBYTE srcByte;     // 原始数据
	DWORD srclen;      // 原始数据长度
}FunAddress, *PFunAddress;


#endif // _MY_HOOK_API_