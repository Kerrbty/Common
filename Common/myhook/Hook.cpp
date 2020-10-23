#define _EXPORT_COMMON_CODE_
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

#pragma pack(push, 1)
typedef struct _jmpCode{
    unsigned char operator_code;
    DWORD address;
}jmpCode, *pjmpCode;

typedef struct _pushretCode{
    unsigned char operator_code;
    DWORD address;
    unsigned char ret_code;
}pushretCode, *ppushretCode;

typedef struct _stubCode{
    jmpCode jmp_code;
    unsigned char other_code[15];
}stubCode, *pstubCode;
#pragma pack(pop)

// 分发函数
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

    // 判断符号位,为负则需减取反 
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

// 跳过 FF25 xxxxxxxx 深入内部hook，或者这里可以添加 E9 xxxxxxxx 深入内部的 hook
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
        return lpFindAddr; // 如果不符合直接返回 
    }
    return GetRealProcAddress(lpFindAddr); // 继续再检测一遍 
}


// hook拷贝出来有些代码是跳转代码，需要看情况修正 
void ResetOffset(
                 LPVOID ProcAddress /* 源代码地址 */,
                 PBYTE CopyCodeAddr /* 拷贝出来的数据 */,
                 DWORD len /* 拷贝长度 */ 
                 )
{
    // "\xE9\x02\x00\x00\x00\xEB\xF9" 
    // HotPatch 
    // detours 如果有热补丁的情况下，会hook hotpatch 的代码 
    SIZE_T dwBytes;
    LPBYTE bFunc = (LPBYTE)ProcAddress;
    if (memcmp(CopyCodeAddr, "\xEB\xF9", 2) == 0 )
    {
        if (bFunc[-5] == 0xE9) // 长跳 
        {
            LPBYTE relAddr = bFunc+make_int64(bFunc-4, 4);
            if (relAddr >= bFunc +2 && relAddr < bFunc+len)
            {
                // 这里在x64中不一定能成立，因为大于前后2G的地址是无法直接跳转的 
                DWORD writevalue = (DWORD)CopyCodeAddr+(relAddr-bFunc)+3;
                writevalue = writevalue - (DWORD)bFunc;
                WriteProcessMemory(GetCurrentProcess(), bFunc-4, &writevalue, 4, &dwBytes);
            }
        }
// 短跳没办法处理，内存不够, 等着去死吧 
//         else if (bFunc[-5] == 0xEB)  
//         {
//             ;
//         }
    }

    DWORD item_len = 0; // 当前函数头指针的偏移长度 
    PBYTE ProcJmp = CopyCodeAddr;
    DWORD new_address = (DWORD)CopyCodeAddr;
    DWORD changelen = len; // 目前扩展后的代码总长度 
    while(changelen>item_len)
    {
        // 修正长跳jmp、call 地址 
        if (*ProcJmp == 0x0E8 || *ProcJmp == 0x0E9 )
        {
            DWORD* OffAddr = (DWORD*)((DWORD)ProcJmp+1);
            *OffAddr = *OffAddr + ((DWORD)ProcAddress-new_address);
        }
        // 修正 jmp short 
        else if (*ProcJmp == 0xEB)
        {
            signed int reloff = (signed int)make_int64(ProcJmp+1, 1) + 2; // 获取当前代码地址的偏移地址 
            // 跳转地址是否超过了复制的字节数 
            if ( reloff<0 || reloff>(int)(changelen-item_len) )
            {
                // 后面的代码全部后移3字节 
                changelen += 3;
                for (DWORD i=changelen-1; i>=item_len+5; i--)
                {
                    ProcJmp[i] = ProcJmp[i-3];
                }

                *ProcJmp = 0xE9;
                // 原地址 + 偏移地址 = 正实地址，再加工成偏移当前下一条指令地址 
                DWORD realoff = (DWORD)ProcAddress + (len-(changelen-item_len-3)) + reloff; // 定位原地址
                *(PDWORD)(ProcJmp+1) = (DWORD)realoff - (DWORD)ProcJmp - 5; 
            }
        }
        // 修正 7x 为 jnx，jx 指令
        else if (((*ProcJmp)&0xF0) == 0x70 )
        {
            // 原来地址+当前偏移+2+跳转字节 
            // 例如 jz xxx 则要变成 jz 05   jmp 05/xxxxxxxx  jmp xxxxxxxx 
            signed int reloff = (signed int)make_int64(ProcJmp+1, 1) + 2; // 获取当前代码地址的偏移地址 
            // 跳转地址是否超过了复制的字节数 
            if ( reloff<0 || reloff>(int)(changelen-item_len) )
            {
                // 后面的代码全部后移2*(jmp)长跳5字节 
                changelen += 10;
                for (DWORD i=changelen-1; i>=item_len+10; i--)
                {
                    ProcJmp[i] = ProcJmp[i-10];
                }

                *(ProcJmp+1) = 0x5;
                // 原地址 + 偏移地址 = 正实地址，再加工成偏移当前下一条指令地址 
                *(ProcJmp+2) = 0xE9; // 跳下一条指令 0xEB 0x05 
                *(ProcJmp+7) = 0xE9; // 跳条件跳转指令 
                if (item_len+2 < len) // 后续还有字节 
                {
                    *(PDWORD)(ProcJmp+3) = 0x5;
                }
                else
                {
                    // 算原函数的地址 
                    *(PDWORD)(ProcJmp+3) = ((DWORD)ProcAddress+len) - ((DWORD)ProcJmp+2) - 5;
                }
                DWORD realoff = (DWORD)ProcAddress + (len-(changelen-item_len-10)) + reloff; // 定位原地址
                *(PDWORD)(ProcJmp+8) = (DWORD)realoff - ((DWORD)ProcJmp+7) - 5;  

                item_len += 2 + 5; // 一条条件跳转，一条无条件跳转 
                ProcJmp += 2 + 5;
            }
        }
        DWORD i = LDE((unsigned char *)ProcJmp, 0);
        item_len += i; 
        ProcJmp = (PBYTE)((DWORD)ProcJmp + i);
    }
}

// hook指定的地址（必须知道这个函数的参数个数，且必须是 __stdcall 调用方式）
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

    // 跳过一些非必要代码，深入内部，也可以hook已hook函数，直到核心 
    LPVOID RealProcAddress = GetRealProcAddress(ProcAddress);
    for (DWORD i=0; i<NowFunNum; i++)
    {
        if( Function[i].HookedFunAddr == (LPVOID)RealProcAddress )  // 对于jmp 好像没什么用，因为会继续深入到跳转目标，尴尬 
        {
            return TRUE;
        }
    }

    stubCode TMPCode = {0};
    DWORD OldProtect;
    pushretCode retbuf = {0};
    retbuf.operator_code = 0x68; // push 
    retbuf.ret_code = 0xC3; // retn 

    memset(&TMPCode, 0x90, sizeof(stubCode));

#ifdef _USE_JMP_
    const int iHookNeedLen = 5;
    TMPCode.jmp_code.operator_code = 0xE9;

    ////////////////////////////////////////////////////////////
    // 偏移地址 = 我们函数的地址 - 原API函数的地址 - 5（我们这条指令的长度）
    TMPCode.jmp_code.address = (DWORD)MyProcAddr - (DWORD)RealProcAddress - 5; 
#else
    // 对应会重载dll的壳来说，不能用jmp (偏移地址会变) 
    const int iHookNeedLen = 6;
    TMPCode.jmp_code.operator_code = 0x68;  // push 
    TMPCode.jmp_code.address = (DWORD)MyProcAddr;
    TMPCode.other_code[0] = 0xC3 ;  // retn 
#endif
    
    DWORD len = 0;
    while(len < iHookNeedLen)
    {
        DWORD i = LDE((unsigned char *)RealProcAddress, 0);
        len += i;
        RealProcAddress = (PVOID)((DWORD)RealProcAddress + i);
    }
    RealProcAddress = (PVOID)((DWORD)RealProcAddress - len);

    retbuf.address = (DWORD)RealProcAddress + len;

    // 真正使用的大小 len + sizeof(retbuf) - 1
    BYTE* ProcJmp = (BYTE*)AllocMemory(len + ALLOC_JMP_Size + sizeof(retbuf)); 
    if (ProcJmp == NULL)
    {
        // 申请内存失败
        return FALSE;
    }
    memset(ProcJmp, 0x90, len+ALLOC_JMP_Size+sizeof(retbuf));

    // 被替换的首部指令
    if ( VirtualProtect(ProcJmp, len+ALLOC_JMP_Size+sizeof(retbuf), PAGE_EXECUTE_READWRITE, &OldProtect) )
    {
        memcpy(ProcJmp, RealProcAddress, len);
        memcpy(ProcJmp+ALLOC_JMP_Size+len, &retbuf, sizeof(retbuf));
//        VirtualProtect(ProcJmp, len+sizeof(retbuf), OldProtect, &OldProtect);
    }
    else
    {
        return FALSE;
    }

    // 保存原函数——自己处理函数
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

    // 修改一些偏移
    ResetOffset(RealProcAddress, ProcJmp, len);

    // Inline Hook
    if( VirtualProtect(RealProcAddress, len, PAGE_EXECUTE_READWRITE, &OldProtect) )
    {
        memcpy(RealProcAddress, &TMPCode, len); // 写入jmp指令
        VirtualProtect(RealProcAddress, len, OldProtect, &OldProtect);
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

// 注意，此函数可以重复hook 
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
        // 回调原函数比对 
        if (Function[i].NewMalloc == ProcAddress)
        {
            if ( VirtualProtect(Function[i].HookedFunAddr, Function[i].srclen, PAGE_EXECUTE_READWRITE, &OldProtect) )
            {
                memcpy(Function[i].HookedFunAddr, Function[i].srcByte, Function[i].srclen);
                VirtualProtect(Function[i].HookedFunAddr, Function[i].srclen, OldProtect, &OldProtect);
                FreeMemory(Function[i].srcByte);
                *(LPVOID*)addr = Function[i].HookedFunAddr;  // 恢复函数指针,以防止后面清除内存 Function[i].NewMalloc 导致bug 

//                 Sleep(0);
//                 // 删除jmp stub内存, 以防多线程调用，还是先保留吧，内存泄漏 
//                 FreeMemory(Function[i].NewMalloc);
//                 Function[i].NewMalloc = NULL; 

                // 后面的hook数据往前移 
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


//////////////////////////////////////////////////////////////////////////
// 函数中间hook，特别小心
#pragma pack(push, 1)
typedef struct _MiddleHookStubProc{
    unsigned char save_context[2]; // pushfd; pushad; 
    unsigned char push_argv;   // push esp; 
    jmpCode call_selfproc;   // call MyProcAddr;
    unsigned char reset_context[2]; // popad; popfd; 
    unsigned char original_code[ALLOC_JMP_Size+5];
    pushretCode retbuf;
    DWORD save_code_size;   // 原始hook字节数  
    unsigned char save_code[20];
}MiddleHookStubProc, *PMiddleHookStubProc;
#pragma pack(pop)
BOOL HookMiddleProc(LPVOID lpTargetAddress, PMiddleSubProc MyProcAddr)
{
    if (lpTargetAddress == NULL || MyProcAddr == NULL)
    {
        return FALSE;
    }
    DWORD OldProtect;
    const DWORD iHookNeedLen = 5;

    DWORD dwcopylen = 0;
    LPVOID RealProcAddress = lpTargetAddress;
    while(dwcopylen < iHookNeedLen)
    {
        DWORD i = LDE((unsigned char *)RealProcAddress, 0);
        dwcopylen += i;
        RealProcAddress = (PVOID)((DWORD)RealProcAddress + i);
    }

    PMiddleHookStubProc ProcStubBuffer = (PMiddleHookStubProc)AllocMemory(sizeof(MiddleHookStubProc)); 
    if (ProcStubBuffer == NULL)
    {
        // 申请内存失败
        return FALSE;
    }
    memset(ProcStubBuffer, 0x90, sizeof(MiddleHookStubProc));
    ProcStubBuffer->save_context[0] = 0x9C; // pushfd; 
    ProcStubBuffer->save_context[1] = 0x60; // pushad; 
    ProcStubBuffer->push_argv = 0x54; // push esp; 
    ProcStubBuffer->call_selfproc.operator_code = 0xE8;
    ProcStubBuffer->call_selfproc.address = 
        (LPBYTE)MyProcAddr - &ProcStubBuffer->call_selfproc.operator_code - 5;
    ProcStubBuffer->reset_context[0] = 0x61; // popad; 
    ProcStubBuffer->reset_context[1] = 0x9D; // popfd; 
    ProcStubBuffer->save_code_size = dwcopylen;


    // 跳转代码权限设定 
    if ( VirtualProtect(ProcStubBuffer, sizeof(MiddleHookStubProc), PAGE_EXECUTE_READWRITE, &OldProtect) )
    {
        memcpy(ProcStubBuffer->original_code, lpTargetAddress, dwcopylen);
        memcpy(ProcStubBuffer->save_code, lpTargetAddress, dwcopylen);
        ProcStubBuffer->retbuf.operator_code = 0x68; // push 
        ProcStubBuffer->retbuf.address = (DWORD)lpTargetAddress + dwcopylen;;
        ProcStubBuffer->retbuf.ret_code = 0xC3; // retn 
    }
    else
    {
        return FALSE;
    }

    stubCode TMPCode = {0};
    memset(&TMPCode, 0x90, sizeof(TMPCode));  // 最后用0x90填充 
    TMPCode.jmp_code.operator_code = 0xE9;
    ////////////////////////////////////////////////////////////
    // 偏移地址 = 我们函数的地址 - 原API函数的地址 - 5（我们这条指令的长度）
    TMPCode.jmp_code.address = (DWORD)ProcStubBuffer->save_context - (DWORD)lpTargetAddress - 5; 

    // 修改一些偏移
    ResetOffset(lpTargetAddress, ProcStubBuffer->original_code, dwcopylen);

    // Inline Hook
    if( VirtualProtect(lpTargetAddress, dwcopylen, PAGE_EXECUTE_READWRITE, &OldProtect) )
    {
        memcpy(lpTargetAddress, &TMPCode, dwcopylen); // 写入jmp指令
        VirtualProtect(lpTargetAddress, dwcopylen, OldProtect, &OldProtect);
    }
    return TRUE;
}

BOOL UnHookMiddleProc(LPVOID lpTargetAddress, PMiddleSubProc MyProcAddr)
{
    if (lpTargetAddress == NULL)
    {
        return FALSE;
    }

    pstubCode TMPCode = (pstubCode)lpTargetAddress;
    if (TMPCode->jmp_code.operator_code != 0xE9)
    {
        return FALSE;
    }

    PMiddleHookStubProc ProcStubBuffer = (PMiddleHookStubProc)
        ((DWORD)lpTargetAddress + TMPCode->jmp_code.address + 5);
    if (ProcStubBuffer->save_context[0] == 0x9C)
    {
        DWORD OldProtect = 0;
        DWORD len = ProcStubBuffer->save_code_size;
        if( VirtualProtect(lpTargetAddress, len, PAGE_EXECUTE_READWRITE, &OldProtect) )
        {
            memcpy(lpTargetAddress, ProcStubBuffer->save_code, len); // 恢复 
//             Sleep(0);
//             FreeMemory(ProcStubBuffer);
            VirtualProtect(lpTargetAddress, len, OldProtect, &OldProtect);
            return TRUE;
        }
    }
    return FALSE;    
}