#include "Hook.h"
#define LOWBYTE 0x000000FF


FunAddress Function[MAX_HOOK_NUM] = {0};
DWORD NowFunNum = 0;


// 分发函数
DWORD WINAPI GetNewAddress(PVOID MyAddr)
{
	DWORD MyFunAddr = (DWORD)MyAddr;
	for (DWORD i=0; i<NowFunNum; i++)
	{
		if (Function[i].MyFunAddr == MyFunAddr)
		{
			return Function[i].NewMalloc;
		}
	}
	return NULL;
}

// hook指定的地址（必须知道这个函数的参数个数，且必须是 __stdcall 调用方式）
BOOL HookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr)
{
	BYTE TMP[5] = {0};
	DWORD OldProtect;
	struct xde_instr UnUse;
	BYTE retbuf[] = "\x68\x00\x00\x00\x00\xC3"; // push address , retn

	
	////////////////////////////////////////////////////////////
	// 偏移地址 = 我们函数的地址 - 原API函数的地址 - 5（我们这条指令的长度）
	DWORD NewAddress = (DWORD)MyProcAddr - (DWORD)ProcAddress - 5; 
	
	TMP[0]=(BYTE)0xE9;
	TMP[1]=(BYTE)(NewAddress&LOWBYTE);
	TMP[2]=(BYTE)((NewAddress>>8)&LOWBYTE);
	TMP[3]=(BYTE)((NewAddress>>16)&LOWBYTE);
	TMP[4]=(BYTE)((NewAddress>>24)&LOWBYTE);

	DWORD len = 0;
	while(len < 5)
	{
		DWORD i = xde_disasm((unsigned char *)ProcAddress, &UnUse);
		len += i;
		ProcAddress = (PVOID)((DWORD)ProcAddress + i);
	}
	ProcAddress = (PVOID)((DWORD)ProcAddress - len);

	*(DWORD *)(retbuf + 1) = (DWORD)ProcAddress + len;

	// 真正使用的大小 len + sizeof(retbuf) - 1
	BYTE* ProcJmp = new BYTE[len + sizeof(retbuf)]; 
	if (ProcJmp == NULL)
	{
		// 申请内存失败
		return FALSE;
	}

	// 被替换的首部指令
	if ( VirtualProtect(ProcJmp, len+sizeof(retbuf), PAGE_EXECUTE_READWRITE, &OldProtect) )
	{
		memcpy(ProcJmp, ProcAddress, len);
		memcpy(ProcJmp+len, retbuf, sizeof(retbuf));
//		VirtualProtect(ProcJmp, len+sizeof(retbuf), OldProtect, &OldProtect);
	}
	else
	{
		return FALSE;
	}


	// 保存原函数——自己处理函数
	Function[NowFunNum].MyFunAddr = (DWORD)MyProcAddr;
	Function[NowFunNum].NewMalloc = (DWORD)ProcJmp;
	NowFunNum++ ;

	// 修改一些偏移
	DWORD item_len = 0;
	DWORD new_address = (DWORD)ProcJmp;
	while(len>item_len)
	{
		if (*ProcJmp == 0x0E8 || *ProcJmp == 0x0E9 )
		{
			DWORD* OffAddr = (DWORD*)((DWORD)ProcJmp+1);
			*OffAddr = *OffAddr + ((DWORD)ProcAddress-new_address);
		}
		DWORD i = xde_disasm((unsigned char *)ProcJmp, &UnUse);
		item_len += i;
		ProcJmp = (PBYTE)((DWORD)ProcJmp + i);
	}

	// Inline Hook
	if( VirtualProtect(ProcAddress, 5, PAGE_EXECUTE_READWRITE, &OldProtect) )
	{
		memcpy(ProcAddress, TMP, 5); // 写入jmp指令
		VirtualProtect(ProcAddress, 5, OldProtect, &OldProtect);
	}
	
	////////////////////////////////////////////////////////////
	
	return TRUE;
}

// 进行hook
BOOL HookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcAddr)
{
	
	
	HMODULE Dll = GetModuleHandle(DllName);
	if (Dll == NULL)
	{
		Dll = LoadLibrary(DllName);
	}
	if (Dll == NULL)
	{
		return FALSE;
	}
	
	PVOID ProcAddress = (PVOID)GetProcAddress(Dll, ProcName);
	if (ProcAddress == NULL)
	{
		FreeLibrary(Dll);
		return FALSE;
	}
	return HookProcByAddress(ProcAddress, MyProcAddr);
}

BOOL UnHookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr)
{
	DWORD OldProtect;
	DWORD CopyAddress = GetNewAddress(MyProcAddr);
	if ( VirtualProtect(ProcAddress, 5, PAGE_EXECUTE_READWRITE, &OldProtect) )
	{
		memcpy(ProcAddress, (LPVOID)CopyAddress, 5);
		VirtualProtect(ProcAddress, 5, OldProtect, &OldProtect);
	}
	return TRUE;
}


// 进行unhook
BOOL UnHookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcAddr)
{
	
	
	HMODULE Dll = GetModuleHandle(DllName);
	if (Dll == NULL)
	{
		Dll = LoadLibrary(DllName);
	}
	if (Dll == NULL)
	{
		return FALSE;
	}

	PVOID ProcAddress = (PVOID)GetProcAddress(Dll, ProcName);
	if (ProcAddress == NULL)
	{
		FreeLibrary(Dll);
		return FALSE;
	}
	return UnHookProcByAddress(ProcAddress, MyProcAddr);
}