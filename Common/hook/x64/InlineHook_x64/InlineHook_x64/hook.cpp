#include "hook.h"
#include "lde64.h"
#define COPY_CODE_LEN  12

CHOOK::FunAddress Function[MAX_HOOK_NUM] = {0};
DWORD CHOOK::NowFunNum = 0;

// �ַ�����
LPVOID WINAPI CHOOK::GetNewAddress(PVOID MyAddr)
{
    LPBYTE MyFunAddr = (LPBYTE)MyAddr;
    for (DWORD i=0; i<NowFunNum; i++)
    {
        if ( (LPBYTE)Function[i].MyFunAddr == MyFunAddr)
        {
            return Function[i].NewMalloc;
        }
    }
    return NULL;
}

// hookָ���ĵ�ַ������֪����������Ĳ����������ұ����� __stdcall ���÷�ʽ��
BOOL CHOOK::HookProcByAddress(LPVOID ProcAddress, LPVOID MyProcAddr)
{
    BYTE TMP[COPY_CODE_LEN] = { 
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov rax, 0
        0xFF, 0xE0 // jmp rax
    }; 
    BYTE retbuf[] = "\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00\xFF\xE0"; // mov rax,xxxx  jmp rax

    DWORD len = 0;
    while(len < COPY_CODE_LEN)
    {
        if ( (*(unsigned char *)ProcAddress&0xF0) == 0x70 || 
             *(unsigned char *)ProcAddress == 0xE8  || 
             *(unsigned char *)ProcAddress == 0xE9 
            )
        {
            // ������������ת����hook
            return FALSE;
        }
        DWORD i = LDE((unsigned char *)ProcAddress, 64);
        len += i;
        ProcAddress = (PVOID)((DWORD)ProcAddress + i);
    }
    ProcAddress = (PVOID)((DWORD)ProcAddress - len);

    *(LPBYTE *)(retbuf + 2) = (LPBYTE)ProcAddress + len;

    // ����ʹ�õĴ�С len + sizeof(retbuf) - 1
    BYTE* ProcJmp = new BYTE[len + sizeof(retbuf)]; 
    if (ProcJmp == NULL)
    {
        // �����ڴ�ʧ��
        return FALSE;
    }

    // ���滻���ײ�ָ��
    DWORD OldProtect;
    if ( VirtualProtect(ProcJmp, len+sizeof(retbuf), PAGE_EXECUTE_READWRITE, &OldProtect) )
    {
        memcpy(ProcJmp, ProcAddress, len);
        memcpy(ProcJmp+len, retbuf, sizeof(retbuf));
    }
    else
    {
        return FALSE;
    }

    // ����ԭ���������Լ�������
    Function[NowFunNum].MyFunAddr = (LPBYTE)MyProcAddr;
    Function[NowFunNum].NewMalloc = (LPBYTE)ProcJmp;
    NowFunNum++ ;

    // Inline Hook
    *(LPBYTE *)((LPBYTE)TMP + 2) = (LPBYTE)MyProcAddr;
    WriteProcessMemory(::GetCurrentProcess(), ProcAddress, TMP, COPY_CODE_LEN, NULL);
    
    return TRUE;
}

// ����hook
BOOL CHOOK::HookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcAddr)
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

BOOL CHOOK::UnHookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr)
{
    LPBYTE CopyAddress = (LPBYTE)GetNewAddress(MyProcAddr);
    WriteProcessMemory(::GetCurrentProcess(), ProcAddress, MyProcAddr, COPY_CODE_LEN, NULL);
    return TRUE;
}

// ����unhook
BOOL CHOOK::UnHookProcByName(LPCTSTR DllName, LPCSTR ProcName, PVOID MyProcAddr)
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