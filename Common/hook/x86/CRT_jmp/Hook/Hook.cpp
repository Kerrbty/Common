#include "Hook.h"
#define LOWBYTE 0x000000FF


FunAddress Function[MAX_HOOK_NUM] = {0};
DWORD NowFunNum = 0;


// �ַ�����
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

// hookָ���ĵ�ַ������֪����������Ĳ����������ұ����� __stdcall ���÷�ʽ��
BOOL HookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr)
{
	BYTE TMP[5] = {0};
	DWORD OldProtect;
	struct xde_instr UnUse;
	BYTE retbuf[] = "\x68\x00\x00\x00\x00\xC3"; // push address , retn

	
	////////////////////////////////////////////////////////////
	// ƫ�Ƶ�ַ = ���Ǻ����ĵ�ַ - ԭAPI�����ĵ�ַ - 5����������ָ��ĳ��ȣ�
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

	// ����ʹ�õĴ�С len + sizeof(retbuf) - 1
	BYTE* ProcJmp = new BYTE[len + sizeof(retbuf)]; 
	if (ProcJmp == NULL)
	{
		// �����ڴ�ʧ��
		return FALSE;
	}

	// ���滻���ײ�ָ��
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


	// ����ԭ���������Լ�������
	Function[NowFunNum].MyFunAddr = (DWORD)MyProcAddr;
	Function[NowFunNum].NewMalloc = (DWORD)ProcJmp;
	NowFunNum++ ;

	// �޸�һЩƫ��
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
		memcpy(ProcAddress, TMP, 5); // д��jmpָ��
		VirtualProtect(ProcAddress, 5, OldProtect, &OldProtect);
	}
	
	////////////////////////////////////////////////////////////
	
	return TRUE;
}

// ����hook
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


// ����unhook
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