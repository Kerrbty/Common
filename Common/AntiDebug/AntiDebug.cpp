#include "stdafx.h"
#include "AntiDebug.h"
#include "Unhook.h"
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Tlhelp32.h>
#include <shlwapi.h>
#if _MSC_VER < 1400
#include "mykernel32.h"
#endif
#pragma comment(lib, "shlwapi.lib")

#if _MSC_VER > 1300
#include <intrin.h>
#endif

#include "Base64.h"
#include "LoadModel.h"
#define LOWBYTE 0x000000FF

#define AllocMemory(_a)  HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _a)
#define FreeMemory(_a)   { if (_a) { HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, _a); _a=NULL; } }

typedef BOOL (WINAPI* pIsDebuggerPresent)(VOID);
typedef BOOL (WINAPI* pTerminateProcess)(  HANDLE hProcess, UINT uExitCode );
typedef VOID (WINAPI* pOutputDebugStringW)(  LPCWSTR lpOutputString );
typedef DWORD (WINAPI* pGetLastError)(VOID);
typedef VOID (WINAPI* pExitProcess)(  UINT uExitCode );


DWORD WINAPI __MoniterThread__(LPVOID lpparam);
PBYTE __global__addr = NULL;
BYTE __global__buf[5];
typedef void (__stdcall* pFuncvalue)(int,int,int,int,int,int,int,int,int,int);
pFuncvalue global__dfunc = (pFuncvalue)__global__addr;
LPTHREAD_START_ROUTINE  global__roundthread = __MoniterThread__;
pTerminateProcess pfTerminal = NULL;
pExitProcess pExitProc = NULL;
HMODULE ghkernel32 = NULL;


//////////////////////////////////////////////////////////////////////////
// PE entery
#pragma comment(linker, "/ENTRY:mystart")

#ifndef _USRDLL

#ifdef _WINMAIN_

#ifdef _UNICODE
EXTERNC void wWinMainCRTStartup();
#define _tmainCRTStartup    wWinMainCRTStartup
#else  /* _UNICODE */
EXTERNC void WinMainCRTStartup();
#define _tmainCRTStartup    WinMainCRTStartup
#endif  /* _UNICODE */

#else  /* _WINMAIN_ */

#ifdef _UNICODE
EXTERNC void wmainCRTStartup();
#define _tmainCRTStartup    wmainCRTStartup
#else  /* _UNICODE */
EXTERNC void mainCRTStartup();
#define _tmainCRTStartup    mainCRTStartup
#endif  /* _UNICODE */

#endif  /* _WINMAIN_ */

int __init_global_i = 0;

void __declspec( naked ) mystart()
{
#ifndef _DEBUG
    __asm {
        __emit 0xE8  // call 略过了
        __emit 0x0E
        __emit 0x00
        __emit 0x00
        __emit 0x00
        __emit 0x8B  // mov edx, [esp+0xC]
        __emit 0x54
        __emit 0x24
        __emit 0x0C
        __emit 0x83  // add [edx+0xbb], 0xD
        __emit 0x82
        __emit 0xB8
        __emit 0x00
        __emit 0x00
        __emit 0x00
        __emit 0x0D
        __emit 0x33  // xor eax,eax
        __emit 0xC0
        __emit 0xC3  // retn
        __emit 0x8D  // lea esp, [esp+4]
        __emit 0x64
        __emit 0x24
        __emit 0x04
    }
#endif
    __func_IDA4__();
    __func_IDA11__();

    for (__init_global_i =0; __init_global_i <5; __init_global_i++)
    {
        __func_IDA1__();
        __vmfunc__(0xFFDFE032, 0x98628462);
        __func_IDA8__();

        __vmfunc__(0xFFAFE072, 0x55628462);
    }

    __func_IDA2__();
    {
        __vmfunc__(0x89DFE032, 0xF5628462);
        _tmainCRTStartup();
    }
}
#else  /* _USRDLL */
EXTERNC BOOL WINAPI _DllMainCRTStartup(HANDLE, DWORD, LPVOID);

__declspec( naked ) VOID WINAPI mystart(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
#ifndef _DEBUG
    __asm {
        __emit 0xE8  // call 略过了
        __emit 0x0E
        __emit 0x00
        __emit 0x00
        __emit 0x00
        __emit 0x8B  // mov edx, [esp+0xC]
        __emit 0x54
        __emit 0x24
        __emit 0x0C
        __emit 0x83  // add [edx+0xbb], 0xD
        __emit 0x82
        __emit 0xB8
        __emit 0x00
        __emit 0x00
        __emit 0x00
        __emit 0x0D
        __emit 0x33  // xor eax,eax
        __emit 0xC0
        __emit 0xC3  // retn
        __emit 0x8D  // lea esp, [esp+4]
        __emit 0x64
        __emit 0x24
        __emit 0x04
    }

	InitAntiDebug();

    __func_IDA4__();
    __func_IDA11__();
    __func_IDA1__();
    __vmfunc__(0xFFDFE032, 0x98628462);
    __func_IDA8__();
    __func_IDA2__();

#endif

//     pAddressOfReturnAddress = (LPBYTE*)_AddressOfReturnAddress();
    __asm{
        mov eax, esp
        mov ecx, [eax+0xC]
        push ecx
        mov ecx, [eax+0x8]
        push ecx
        mov ecx, [eax+0x4]
        push ecx
        call _DllMainCRTStartup
    }
//    _DllMainCRTStartup(hDllHandle, dwReason, lpreserved);
    __asm ret 0xC
}
#endif  /* _USRDLL */

//
//////////////////////////////////////////////////////////////////////////

// 挂接调试器的时候会直接运行到这里
VOID __Hook_DbgUiRemoteBreakin__()
{
	__func_IDA3__();
	__vmfunc__(0xFF244032, 0x74582462);
	if (pfTerminal != NULL)
	{
		pfTerminal((HANDLE)-1, 0);
	}
	if (pExitProc != NULL)
	{
		pExitProc(-1);
	}
	__func_IDA1__();
	ExitProcess(0);
	global__dfunc(0,0,0,0,0,0,0,0,0,0);
}

// hook指定的地址（必须知道这个函数的参数个数，且必须是 __stdcall 调用方式）
BOOL __HookProcByAddress__(LPVOID ProcAddress, PVOID MyProcAddr)
{	
	__func_IDA3__();
	////////////////////////////////////////////////////////////
	// 偏移地址 = 我们函数的地址 - 原API函数的地址 - 5（我们这条指令的长度）
	DWORD NewAddress = (DWORD)MyProcAddr - (DWORD)ProcAddress - 5; 
	
	__global__buf[0]=(BYTE)0xE9;
	__global__buf[1]=(BYTE)(NewAddress&LOWBYTE);
	__global__buf[2]=(BYTE)((NewAddress>>8)&LOWBYTE);
	__global__buf[3]=(BYTE)((NewAddress>>16)&LOWBYTE);
	__global__buf[4]=(BYTE)((NewAddress>>24)&LOWBYTE);

	return TRUE;
}

// 不断的写入DbgUiRemoteBreakin hook的函数
DWORD WINAPI __MoniterThread__(LPVOID lpparam)
{
	__vmfunc__(0x15DFE032, 0x85469255);
	while(1)
	{
		__func_IDA1__();
		DWORD dwBytes = 0;
		WriteProcessMemory((HANDLE)-1, __global__addr, __global__buf, 5, &dwBytes);
		Sleep(800);
	}
	return 0;
}

// 监控并启动写入hook进程
DWORD WINAPI __ThreadProc__(LPVOID lpparam)
{
	__vmfunc__(0xFFD77032, 0x66628462);
	static HANDLE thread = CreateThread(NULL, 0, global__roundthread, NULL, 0, NULL);
	__func_IDA2__();
	while(1)
	{
		if (thread == NULL)
		{
			thread = CreateThread(NULL, 0, global__roundthread, NULL, 0, NULL);
		}
		else
		{
			DWORD ExitCode = 0;
			if( GetExitCodeThread(thread, &ExitCode) && STILL_ACTIVE != ExitCode)
			{
				CloseHandle(thread);
				thread= NULL;
				thread = CreateThread(NULL, 0, global__roundthread, NULL, 0, NULL);
			}
		}
		Sleep(1500);
		__vmfunc__(0x55DFE032, 0x32628462);
	}
	return 0;
}


#ifndef _DEBUG
// 初始化检测线程
BOOL InitAntiDebug()
{
	char* decodebuf = (char*)AllocMemory(MAX_PATH);
	char DebugBreakin[] = {"K^SwONuJCN(o}^OZztOqx+ul"};   // DbgUiRemoteBreakin  经过base64编码，再异或 25
	char IsDebug[] = {"JAW\\CNS(C+}uzu[`CAWu{wH$$"};      // IsDebuggerPresent   经过base64编码，再异或 25
	char kernel32[] = {"J+O`{tOjTcPlC^aj"};               // kernel32.dll        经过base64编码，再异或 25
	char Terminal[] = {"O^O`{Nul@AKuLQSo@+Oczn$$"};       // TerminateProcess    经过base64编码，再异或 25
	char OutPutString[] = {"M*O)zQO)K^Op}N}M}QSi{t}A$$"}; // OutputDebugStringW  经过base64编码，再异或 25
	char GetError[] = {"K+O)M^_c}\\O`zt `$$"};            // GetLastError        经过base64编码，再异或 25
	char ExitProc[] = {"KAqi}_[`{+Wuz*T$$"};              // ExitProcess         经过base64编码，再异或 25

	int len = strlen(DebugBreakin);
	int i = 0;
	for (i=0; i<len; i++)
	{
		DebugBreakin[i] ^= 25;
	}

	len = strlen(IsDebug);
	for (i=0; i<len; i++)
	{
		IsDebug[i] ^= 25;
	}

	len = strlen(kernel32);
	for (i=0; i<len; i++)
	{
		kernel32[i] ^= 25;
	}

	len = strlen(Terminal);
	for (i=0; i<len; i++)
	{
		Terminal[i] ^= 25;
	}

	len = strlen(ExitProc);
	for (i=0; i<len; i++)
	{
		ExitProc[i] ^= 25;
	}


	len = strlen(GetError);
	for (i=0; i<len; i++)
	{
		GetError[i] ^= 25;
	}


	len = strlen(OutPutString);
	for (i=0; i<len; i++)
	{
		OutPutString[i] ^= 25;
	}


	__func_IDA1__();

	// 先重载kernel32.dll，防止被hook
	// 判断目前是否处于调试
	__try
	{
		ZeroMemory(decodebuf, MAX_PATH);
		Base64_decode(kernel32, strlen(kernel32), (PBYTE)decodebuf);
		HMODULE hmodule = GetModuleHandleA(decodebuf);

		char* dllpath = (char*)AllocMemory(MAX_PATH);
		GetSystemDirectoryA(dllpath, MAX_PATH);
		strcat(dllpath, "\\");
		strcat(dllpath, decodebuf);
		if (ghkernel32 == NULL)
		{
			ghkernel32 = LoadMyDllsA(dllpath, (LPBYTE)hmodule);
		}
		FreeMemory(dllpath);

		ZeroMemory(decodebuf, MAX_PATH);

		__func_IDA1__();
		Base64_decode(ExitProc, strlen(ExitProc), (PBYTE)decodebuf);
		pExitProc = (pExitProcess)MyGetProcAddress(ghkernel32, decodebuf);

		Base64_decode(IsDebug, strlen(IsDebug), (PBYTE)decodebuf);
		pIsDebuggerPresent presentDebug = (pIsDebuggerPresent)MyGetProcAddress(ghkernel32, decodebuf);

		Base64_decode(Terminal, strlen(Terminal), (PBYTE)decodebuf);
		pfTerminal = (pTerminateProcess)MyGetProcAddress(ghkernel32, decodebuf);

		if (presentDebug != NULL)
		{
			if (presentDebug())
			{
				if (Terminal != NULL)
				{
					pfTerminal((HANDLE)-1, 0);
				}
			}
		}

		// OutputDebugString反调试
// 		ZeroMemory(decodebuf, MAX_PATH);
// 		Base64_decode(OutPutString, strlen(OutPutString), (PBYTE)decodebuf);
// 		pOutputDebugStringW DebugStringW = (pOutputDebugStringW)MyGetProcAddress(hkernel32, decodebuf);
// 
// 		ZeroMemory(decodebuf, MAX_PATH);
// 		Base64_decode(GetError, strlen(GetError), (PBYTE)decodebuf);
// 		pGetLastError geterror = (pGetLastError)MyGetProcAddress(hkernel32, decodebuf);
// 		if (presentDebug != NULL)
// 		{
// 			DebugStringW(L"process Init...");
// 			if( geterror() == 0 )
// 			{
// 				if (Terminal != NULL)
// 				{
// 					Terminal((HANDLE)-1, 0);
// 				}
// 			}
// 		}


		// fs:[30] --> PEB
		// PEB->flags
// #if _MSC_VER > 1300
// 		BYTE *value = (BYTE*)__readfsdword(0x30);
// 		DWORD*dwval = (DWORD*)__readfsdword(0x30);
// 		BYTE flags = value[2];
// 		// 调试符号位
// 		if (flags == 0x1)
// 		{
// 			if (Terminal != NULL)
// 			{
// 				pfTerminal((HANDLE)-1, 0);
// 			}
// 		}
// 
// 		if (dwval[26] != 0 )
// 		{
// 			if (Terminal != NULL)
// 			{
// 				pfTerminal((HANDLE)-1, 0);
// 			}
// 		}
// #endif

//		FreeMyDlls(hkernel32, (LPBYTE)hmodule);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		__func_IDA3__();
	}

	// 设置动态禁止挂接	
	Base64_decode(DebugBreakin, strlen(DebugBreakin), (PBYTE)decodebuf);
	__global__addr = (PBYTE)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), decodebuf);
	FreeMemory(decodebuf);

	if (__global__addr[0] != '\xE9')
	{
		__func_IDA2__();
		__HookProcByAddress__(__global__addr, __Hook_DbgUiRemoteBreakin__);
		HANDLE handle = CreateThread(NULL, 0, __ThreadProc__, NULL, 0, NULL);
		if (handle == NULL)
		{
			return FALSE;
		}
		CloseHandle(handle);
	}
	return TRUE;
}
#endif
