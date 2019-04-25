#ifndef _CHECK_MODULE_IN_PROCESS_HH_
#define _CHECK_MODULE_IN_PROCESS_HH_
#include <Windows.h>
#include <tchar.h>
#if _MSC_VER > 1300 
#include <intrin.h>
#endif


#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif
// _ReturnAddress and _AddressOfReturnAddress should be prototyped before use
EXTERNC void* _AddressOfReturnAddress(void);
EXTERNC void* _ReturnAddress(void);
#pragma intrinsic(_AddressOfReturnAddress)
#pragma intrinsic(_ReturnAddress)

// 调试模式下，所有反调试函数都无效
#ifdef _DEBUG

#define __func_IDA1__()
#define __func_IDA2__()
#define __func_IDA3__()
#define __func_IDA4__()
#define __func_IDA5__()
#define __func_IDA6__()
#define __func_IDA7__()
#define __func_IDA8__()
#define __func_IDA9__()
#define __func_IDA10__()
#define __func_IDA11__()


#define __vmfunc__(__Key, __value1__)

#else
// release模式开启反调试

void __forceinline __func_IDA1__()
{
#ifndef _WIN64
    __asm{
        __emit 0x66
        __emit 0x90
        __emit 0x70
        __emit 0x03
        __emit 0x71
        __emit 0x01
        __emit 0xE9
    }
#else
    __halt();
    __nop();
    __halt();
    __nop();
    __halt();
#endif
}

void __forceinline __func_IDA2__()
{
#ifndef _WIN64
    __asm{
        __emit 0x66
        __emit 0x90
        __emit 0x74
        __emit 0x03
        __emit 0x75
        __emit 0x01
        __emit 0xE9
    }
#else
    __halt();
    __nop();
    __halt();
    __nop();
    __halt();
#endif
}

void __forceinline __func_IDA3__()
{
#ifndef _WIN64
    __asm{
        __emit 0x66
        __emit 0x90
        __emit 0x72
        __emit 0x03
        __emit 0x73
        __emit 0x01
        __emit 0x0F
    }
#else
    __halt();
    __nop();
    __halt();
    __nop();
    __halt();
#endif
}

void __forceinline __func_IDA4__()
{
#ifndef _WIN64
    __asm{
        // push eax  // 还是保存一下原始值吧
        __emit 0x50  // __emit 60  // pushad
        // call  00000005
        __emit 0xE8
        __emit 0x00
        __emit 0x00
        __emit 0x00
        __emit 0x00
        // pop eax
        __emit 0x58
        // add eax, 0x6
        __emit 0x83
        __emit 0xC0
        __emit 0x06
        // push eax
        __emit 0x50
        // ret
        __emit 0xC3
        // pop eax
        __emit 0x58   // __emit 0x61  // popad
    }
#else
    __halt();
    __nop();
    __halt();
    __nop();
    __halt();
#endif
}

void __forceinline __func_IDA5__()
{
#ifndef _WIN64
    __asm{
        __emit 0x66
        __emit 0x90
        __emit 0x78
        __emit 0x03
        __emit 0x79
        __emit 0x01
        __emit 0xC2   // pop eax
    }
#else
    __halt();
    __nop();
    __halt();
    __nop();
    __halt();
#endif
}

void __forceinline __func_IDA6__()
{
#ifndef _WIN64
    __asm{
        __emit 0x0F   // NOP DWORD ptr [EAX] 
        __emit 0x1F
        __emit 0x00
        __emit 0x0F   // NOP DWORD ptr [EAX + 00H]
        __emit 0x1F
        __emit 0x40
        __emit 0x00
    }
#else
    __halt();
    __nop();
    __halt();
    __nop();
    __halt();
#endif
}


void __forceinline __func_IDA7__()
{
#ifndef _WIN64
    __asm{
        __emit 0x0F  // NOP DWORD ptr [EAX + EAX*1 + 00H] 
        __emit 0x1F
        __emit 0x44
        __emit 0x00
        __emit 0x00 
    }
#else
    __halt();
    __nop();
    __halt();
    __nop();
    __halt();
#endif
}


void __forceinline __func_IDA8__()
{
#ifndef _WIN64
    __asm{
        __emit 0x66    //  NOP DWORD ptr [EAX + EAX*1 + 00H]
        __emit 0x0F
        __emit 0x1F
        __emit 0x44
        __emit 0x00 
        __emit 0x00
    }
#else
    __halt();
    __nop();
    __halt();
    __nop();
    __halt();
#endif
}

void __forceinline __func_IDA9__()
{
#ifndef _WIN64
    __asm{
        __emit 0x0F   // NOP DWORD ptr [EAX + 00000000H]
        __emit 0x1F
        __emit 0x80
        __emit 0x00
        __emit 0x00  
        __emit 0x00
        __emit 0x00
    }
#else
    __halt();
    __nop();
    __halt();
    __nop();
    __halt();
#endif
}

void __forceinline __func_IDA10__()
{
#ifndef _WIN64
    __asm{
        __emit 0x0F  // NOP DWORD ptr [EAX + EAX*1 + 00000000H] 
        __emit 0x1F
        __emit 0x84
        __emit 0x00
        __emit 0x00 
        __emit 0x00
        __emit 0x00
        __emit 0x00
    }
#else
    __halt();
    __nop();
    __halt();
    __nop();
    __halt();
#endif
}


void __forceinline __func_IDA11__()
{
#ifndef _WIN64
    __asm{
        __emit 0x66  // NOP DWORD ptr [EAX + EAX*1 + 00000000H]
        __emit 0x0F
        __emit 0x1F
        __emit 0x84
        __emit 0x00
        __emit 0x00 
        __emit 0x00
        __emit 0x00
        __emit 0x00
    }
#else
    __halt();
    __nop();
    __halt();
    __nop();
    __halt();
#endif
}

#define __vmfunc__(__Key, __value1__)   \
{  \
    __asm pushfd                \
    __asm push  __LINE__        \
    __asm xchg  [esp], eax      \
    __asm not   eax             \
    __asm and   eax,eax         \
    __asm cmp   eax,eax         \
    __asm lea   esp, dword ptr [esp+04h]   \
    __asm mov   eax, edi        \
    __asm xor   eax, ebx        \
    __asm not   eax             \
    __asm and   eax, ecx        \
    __asm cmp   eax, eax        \
    __asm xchg  [esp-4], eax    \
    __asm push  __value1__        \
    __asm lea   esp, dword ptr [esp+04h]   \
    __asm xchg  [esp], eax      \
    __asm lea   esp, dword ptr [esp-04h]   \
    __asm xchg  [esp], eax      \
    __asm push  __Key    \
    __asm lea   esp, dword ptr [esp+04h]   \
    __asm push  eax             \
    __asm push  __LINE__        \
    __asm mov   eax, dword ptr [esp+08h]   \
    __asm xchg  [esp], eax      \
    __asm popfd                 \
    __asm pushfd                \
    __asm add   esp, 4          \
    __asm lea   esp, dword ptr [esp-04h]   \
    __asm pop   eax             \
    __asm not   eax             \
    __asm xor   eax, ecx        \
    __asm cmp   eax, eax        \
    __asm mov   eax, GetCommandLine    \
    __asm call  eax             \
    __asm push   _MSC_VER       \
    __asm xchg  [esp], eax      \
    __asm lea   esp, dword ptr [esp+04h]   \
    __asm pop   eax             \
    __asm sub   esp, 4          \
    __asm xchg  dword ptr [esp], eax   \
    __asm not   eax             \
    __asm and   eax, eax        \
    __asm cmp   eax, eax        \
    __asm push  0FF5Fh          \
    __asm pop   eax             \
    __asm lea   esp, dword ptr [esp+04h]   \
    __asm push  __LINE__    \
    __asm add   esp,4           \
    __asm push __LINE__         \
    __asm lea esp, dword ptr [esp+04h]  \
    __asm mov   eax, GetEnvironmentStrings  \
    __asm call  eax             \
    __asm push  eax             \
    __asm mov   eax, FreeEnvironmentStrings \
    __asm call  eax             \
    __asm xchg  eax,[esp]       \
    __asm push  0fbb5f5h        \
    __asm add   esp, 04h        \
    __asm xchg  [esp],eax       \
    __asm dec   eax             \
    __asm dec   eax             \
    __asm push  ecx             \
    __asm xchg  eax,[esp]       \
    __asm add   esp, 4          \
    __asm push  0fbb5f5h        \
    __asm add   esp, 04h        \
    __asm mov   eax, GetLastError   \
    __asm call  eax             \
    __asm pushfd                \
    __asm pop   eax             \
    __asm popfd                 \
    __asm xchg   [esp], eax     \
    __asm lea   esp,[esp+04h]   \
}

#endif  // _DEBUG

BOOL CheckModule(LPTSTR lpFileName);

BOOL FreeAllNotSystemModule();

#endif   // _CHECK_MODULE_IN_PROCESS_HH_