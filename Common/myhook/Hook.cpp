#include "Hook.h"
#define LOWBYTE 0x000000FF
#define AllocMemory(_a)  HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _a)
#define FreeMemory(_a)   { if (_a) { HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, _a); _a=NULL; } }
#define Deletehandle(_a) {if(_a != INVALID_HANDLE_VALUE) { CloseHandle(_a); _a = INVALID_HANDLE_VALUE;}}


#define ALLOC_JMP_Size  60
typedef signed __int64 int64;

FunAddress Function[MAX_HOOK_NUM] = {0};
DWORD NowFunNum = 0;
CRITICAL_SECTION  CriticalSection;


// �ַ�����
LPVOID WINAPI GetNewAddress(PVOID MyAddr)
{
    LPBYTE MyFunAddr = (LPBYTE)MyAddr;
    LPBYTE retaddr = NULL;
    if (NowFunNum == 0)
    {
        return NULL;
    }

    EnterCriticalSection(&CriticalSection);
    for (DWORD i=0; i<NowFunNum; i++)
    {
        if (Function[i].MyFunAddr == MyFunAddr)
        {
            retaddr = Function[i].NewMalloc;
        }
    }
    LeaveCriticalSection(&CriticalSection);

    return retaddr;
}

int64 make_int64(unsigned char* buf, int lenth)  
{  
    int i = 0;
    int64 ui=0;
    if (lenth > 8 || lenth < 1)
    {
        return (int64)0;
    }

    for (i=0; i<lenth; i++)
    {
        ui = (buf[i]<<8*i)|ui;
    }

    // �жϷ���λ,Ϊ�������ȡ�� 
    if (buf[lenth-1] >= 0x80)
    {
        int64 xorval = 0;
        for (i=0; i<lenth; i++)
        {
            xorval = xorval|(0xFF<<8*i);
        }
        ui = -((ui - 1)^xorval);
    }
    return ui;
}  

// ���� FF25 xxxxxxxx �����ڲ�hook���������������� E9 xxxxxxxx �����ڲ��� hook
LPVOID GetRealProcAddress(LPVOID FuncAddr)
{
    LPBYTE lpFindAddr = (LPBYTE)FuncAddr;
    LPBYTE lpMachineByte = (LPBYTE)FuncAddr;
    if ( lpMachineByte[0] == 0xFF && lpMachineByte[1] == 0x25 )
    {
        lpFindAddr = *(LPBYTE*)((LPBYTE)lpFindAddr+2);
        lpFindAddr = *(LPBYTE*)((LPBYTE)lpFindAddr);
    }
    else if (lpMachineByte[0] == 0xEB) // jmp short 
    {
        lpFindAddr = lpFindAddr + 2 + make_int64(lpFindAddr+1, 1);
    }
    else if (lpMachineByte[0] == 0xE9)  // jmp long 
    {
        lpFindAddr = lpFindAddr + 5 + make_int64(lpFindAddr+1, 4);
    }
    else if (memcmp(lpMachineByte, "\x8B\xFF\x55\x8B\xEC\x5D", 6) == 0)
    {
        lpFindAddr = lpFindAddr+6;
    }
    else
    {
        return lpFindAddr; // ���������ֱ�ӷ��� 
    }
    return GetRealProcAddress(lpFindAddr); // �����ټ��һ�� 
}


// hook����������Щ��������ת���룬��Ҫ��������� 
void ResetOffset(
                 LPVOID ProcAddress /* Դ�����ַ */,
                 PBYTE CopyCodeAddr /* �������������� */,
                 DWORD len /* �������� */ 
                 )
{
    // "\xE9\x02\x00\x00\x00\xEB\xF9" 
    // HotPatch 
    // detours ������Ȳ���������£���hook hotpatch �Ĵ��� 
    SIZE_T dwBytes;
    LPBYTE bFunc = (LPBYTE)ProcAddress;
    if (memcmp(CopyCodeAddr, "\xEB\xF9", 2) == 0 )
    {
        if (bFunc[-5] == 0xE9) // ���� 
        {
            LPBYTE relAddr = bFunc+make_int64(bFunc-4, 4);
            if (relAddr >= bFunc +2 && relAddr < bFunc+len)
            {
                // ������x64�в�һ���ܳ�������Ϊ����ǰ��2G�ĵ�ַ���޷�ֱ����ת�� 
                DWORD writevalue = (DWORD)CopyCodeAddr+(relAddr-bFunc)+3;
                writevalue = writevalue - (DWORD)bFunc;
                WriteProcessMemory(GetCurrentProcess(), bFunc-4, &writevalue, 4, &dwBytes);
            }
        }
// ����û�취�����ڴ治��, ����ȥ���� 
//         else if (bFunc[-5] == 0xEB)  
//         {
//             ;
//         }
    }

    DWORD item_len = 0; // ��ǰ����ͷָ���ƫ�Ƴ��� 
    PBYTE ProcJmp = CopyCodeAddr;
    DWORD new_address = (DWORD)CopyCodeAddr;
    DWORD changelen = len; // Ŀǰ��չ��Ĵ����ܳ��� 
    while(changelen>item_len)
    {
        // ��������jmp��call ��ַ 
        if (*ProcJmp == 0x0E8 || *ProcJmp == 0x0E9 )
        {
            DWORD* OffAddr = (DWORD*)((DWORD)ProcJmp+1);
            *OffAddr = *OffAddr + ((DWORD)ProcAddress-new_address);
        }
        // ���� jmp short 
        else if (*ProcJmp == 0xEB)
        {
            signed int reloff = (signed int)make_int64(ProcJmp+1, 1) + 2; // ��ȡ��ǰ�����ַ��ƫ�Ƶ�ַ 
            // ��ת��ַ�Ƿ񳬹��˸��Ƶ��ֽ��� 
            if ( reloff<0 || reloff>(int)(changelen-item_len) )
            {
                // ����Ĵ���ȫ������3�ֽ� 
                changelen += 3;
                for (DWORD i=changelen-1; i>=item_len+5; i--)
                {
                    ProcJmp[i] = ProcJmp[i-3];
                }

                *ProcJmp = 0xE9;
                // ԭ��ַ + ƫ�Ƶ�ַ = ��ʵ��ַ���ټӹ���ƫ�Ƶ�ǰ��һ��ָ���ַ 
                DWORD realoff = (DWORD)ProcAddress + (len-(changelen-item_len-3)) + reloff; // ��λԭ��ַ
                *(PDWORD)(ProcJmp+1) = (DWORD)realoff - (DWORD)ProcJmp - 5; 
            }
        }
        // ���� 7x Ϊ jnx��jx ָ��
        else if (((*ProcJmp)&0xF0) == 0x70 )
        {
            // ԭ����ַ+��ǰƫ��+2+��ת�ֽ� 
            // ���� jz xxx ��Ҫ��� jz 05   jmp 05/xxxxxxxx  jmp xxxxxxxx 
            signed int reloff = (signed int)make_int64(ProcJmp+1, 1) + 2; // ��ȡ��ǰ�����ַ��ƫ�Ƶ�ַ 
            // ��ת��ַ�Ƿ񳬹��˸��Ƶ��ֽ��� 
            if ( reloff<0 || reloff>(int)(changelen-item_len) )
            {
                // ����Ĵ���ȫ������2*(jmp)����5�ֽ� 
                changelen += 10;
                for (DWORD i=changelen-1; i>=item_len+10; i--)
                {
                    ProcJmp[i] = ProcJmp[i-10];
                }

                *(ProcJmp+1) = 0x5;
                // ԭ��ַ + ƫ�Ƶ�ַ = ��ʵ��ַ���ټӹ���ƫ�Ƶ�ǰ��һ��ָ���ַ 
                *(ProcJmp+2) = 0xE9; // ����һ��ָ�� 0xEB 0x05 
                *(ProcJmp+7) = 0xE9; // ��������תָ�� 
                if (item_len+2 < len) // ���������ֽ� 
                {
                    *(PDWORD)(ProcJmp+3) = 0x5;
                }
                else
                {
                    // ��ԭ�����ĵ�ַ 
                    *(PDWORD)(ProcJmp+3) = ((DWORD)ProcAddress+len) - ((DWORD)ProcJmp+2) - 5;
                }
                DWORD realoff = (DWORD)ProcAddress + (len-(changelen-item_len-10)) + reloff; // ��λԭ��ַ
                *(PDWORD)(ProcJmp+8) = (DWORD)realoff - ((DWORD)ProcJmp+7) - 5;  

                item_len += 2 + 5; // һ��������ת��һ����������ת 
                ProcJmp += 2 + 5;
            }
        }
        DWORD i = LDE((unsigned char *)ProcJmp, 0);
        item_len += i; 
        ProcJmp = (PBYTE)((DWORD)ProcJmp + i);
    }
}

// hookָ���ĵ�ַ������֪����������Ĳ����������ұ����� __stdcall ���÷�ʽ��
BOOL HookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr, LPVOID* NewStubAddr)
{
    static BOOL initval = FALSE;
    if (!initval)
    {
        initval = TRUE;
        InitializeCriticalSection(&CriticalSection);
    }
    if (ProcAddress == NULL || MyProcAddr == NULL)
    {
        return FALSE;
    }

    // ����һЩ�Ǳ�Ҫ���룬�����ڲ���Ҳ����hook��hook������ֱ������ 
    LPVOID RealProcAddress = GetRealProcAddress(ProcAddress);
    for (int i=0; i<NowFunNum; i++)
    {
        if( Function[i].HookedFunAddr == (LPVOID)RealProcAddress )  // ����jmp ����ûʲô�ã���Ϊ��������뵽��תĿ�꣬���� 
        {
            return TRUE;
        }
    }

    BYTE TMP[20] = {0};
    DWORD OldProtect;
    BYTE retbuf[] = "\x68\x00\x00\x00\x00\xC3"; // push address , retn

    memset(TMP, 0x90, 20);

#ifdef _USE_JMP_
    const int iHookNeedLen = 5;
    TMP[0]=(BYTE)0xE9;

    ////////////////////////////////////////////////////////////
    // ƫ�Ƶ�ַ = ���Ǻ����ĵ�ַ - ԭAPI�����ĵ�ַ - 5����������ָ��ĳ��ȣ�
    DWORD NewAddress = (DWORD)MyProcAddr - (DWORD)RealProcAddress - 5; 
    *(DWORD*)(TMP+1) = NewAddress;
#else
    // ��Ӧ������dll�Ŀ���˵��������jmp (ƫ�Ƶ�ַ���) 
    const int iHookNeedLen = 6;
    TMP[0]= (BYTE)0x68;
    *(DWORD*)(TMP+1) = (DWORD)MyProcAddr;
    TMP[5] = (BYTE)0xC3 ;
#endif
    
    DWORD len = 0;
    while(len < iHookNeedLen)
    {
        DWORD i = LDE((unsigned char *)RealProcAddress, 0);
        len += i;
        RealProcAddress = (PVOID)((DWORD)RealProcAddress + i);
    }
    RealProcAddress = (PVOID)((DWORD)RealProcAddress - len);

    *(DWORD*)(retbuf+1) = (DWORD)RealProcAddress + len;

    // ����ʹ�õĴ�С len + sizeof(retbuf) - 1
    BYTE* ProcJmp = (BYTE*)AllocMemory(len + ALLOC_JMP_Size + sizeof(retbuf)); 
    if (ProcJmp == NULL)
    {
        // �����ڴ�ʧ��
        return FALSE;
    }
    memset(ProcJmp, 0x90, len+ALLOC_JMP_Size+sizeof(retbuf));

    // ���滻���ײ�ָ��
    if ( VirtualProtect(ProcJmp, len+ALLOC_JMP_Size+sizeof(retbuf), PAGE_EXECUTE_READWRITE, &OldProtect) )
    {
        memcpy(ProcJmp, RealProcAddress, len);
        memcpy(ProcJmp+ALLOC_JMP_Size+len, retbuf, sizeof(retbuf));
//        VirtualProtect(ProcJmp, len+sizeof(retbuf), OldProtect, &OldProtect);
    }
    else
    {
        return FALSE;
    }

    // ����ԭ���������Լ�������
    EnterCriticalSection(&CriticalSection);
    Function[NowFunNum].HookedFunAddr = (LPBYTE)RealProcAddress;
    Function[NowFunNum].MyFunAddr = (LPBYTE)MyProcAddr;
    Function[NowFunNum].NewMalloc = (LPBYTE)ProcJmp;
    Function[NowFunNum].srclen = len;
    Function[NowFunNum].srcByte = (LPBYTE)AllocMemory(len);
    memcpy(Function[NowFunNum].srcByte, RealProcAddress, len);
    NowFunNum++ ;
    LeaveCriticalSection(&CriticalSection);

    if (NewStubAddr != NULL)
    {
        *NewStubAddr = (LPBYTE)ProcJmp;
    }

    // �޸�һЩƫ��
    ResetOffset(RealProcAddress, ProcJmp, len);

    // Inline Hook
    if( VirtualProtect(RealProcAddress, len, PAGE_EXECUTE_READWRITE, &OldProtect) )
    {
        memcpy(RealProcAddress, TMP, len); // д��jmpָ��
        VirtualProtect(RealProcAddress, len, OldProtect, &OldProtect);
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

// ע�⣬�˺��������ظ�hook 
BOOL HookProc(LPVOID addr, LPVOID MyProcAddr)
{
    LPVOID ProcAddress = *(PBYTE*)addr;
    return HookProcByAddress(ProcAddress, MyProcAddr, (LPVOID *)addr);
}


// UnHookProc(&(LPVOID&)pOldCreateFileW, HookCreateFile); 
BOOL UnHookProc(LPVOID addr, LPVOID MyProcAddr)
{
    if (addr == NULL)
    {
        return FALSE;
    }

    LPBYTE ProcAddress = *(LPBYTE*)addr;
    DWORD OldProtect;
    if (NowFunNum == 0)
    {
        return FALSE;
    }

    EnterCriticalSection(&CriticalSection);
    for (DWORD i=0; i<NowFunNum; i++)
    {
        // �ص�ԭ�����ȶ� 
        if (Function[i].NewMalloc == ProcAddress)
        {
            if ( VirtualProtect(Function[i].HookedFunAddr, Function[i].srclen, PAGE_EXECUTE_READWRITE, &OldProtect) )
            {
                memcpy(Function[i].HookedFunAddr, Function[i].srcByte, Function[i].srclen);
                VirtualProtect(Function[i].HookedFunAddr, Function[i].srclen, OldProtect, &OldProtect);
                FreeMemory(Function[i].srcByte);
                *(LPVOID*)addr = Function[i].HookedFunAddr;  // �ָ�����ָ��,�Է�ֹ��������ڴ� Function[i].NewMalloc ����bug 

//                 Sleep(0);
//                 // ɾ��jmp stub�ڴ�, �Է����̵߳��ã������ȱ����ɣ��ڴ�й© 
//                 FreeMemory(Function[i].NewMalloc);
//                 Function[i].NewMalloc = NULL; 

                // �����hook������ǰ�� 
                NowFunNum--;
                while (i<NowFunNum)
                {
                    memcpy(Function+i, Function+i+1, sizeof(*Function));
                    i++;
                }
            }


        }
    }
    LeaveCriticalSection(&CriticalSection);
    return TRUE;
}


BOOL UnHookProcByAddress(LPVOID ProcAddress, PVOID MyProcAddr)
{
    DWORD OldProtect;
    if (NowFunNum == 0)
    {
        return FALSE;
    }

    LPBYTE lpRealProcAddress = (LPBYTE)GetRealProcAddress(ProcAddress);
    EnterCriticalSection(&CriticalSection);
    for (DWORD i=0; i<NowFunNum; i++)
    {
        if (Function[i].HookedFunAddr == (LPBYTE)lpRealProcAddress)
        {
            if ( VirtualProtect(Function[i].HookedFunAddr, Function[i].srclen, PAGE_EXECUTE_READWRITE, &OldProtect) )
            {
                memcpy(Function[i].HookedFunAddr, Function[i].srcByte, Function[i].srclen);
                VirtualProtect(Function[i].HookedFunAddr, Function[i].srclen, OldProtect, &OldProtect);
                FreeMemory(Function[i].srcByte);
                NowFunNum--;
                while (i<NowFunNum)
                {
                    memcpy(Function+i, Function+i+1, sizeof(*Function));
                    i++;
                }
            }
        }
    }
    LeaveCriticalSection(&CriticalSection);

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
