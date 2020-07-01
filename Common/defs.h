// CommonDef.h 通用的windows宏定义 
// 
#ifndef _ZXLY_COMMAND_DEFINES_HEADER_H_
#define _ZXLY_COMMAND_DEFINES_HEADER_H_
#ifdef __cplusplus
#define EXTERN_C   extern "C"
#else  // __cplusplus 
#define EXTERN_C
#endif // __cplusplus 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif

#ifdef _DEBUG
#define CHECK_MEMORY   // 校验缓冲区溢出开关 
#endif // _DEBUG

#define HEAPMEMEOY      0x01
#define MALLOCMEMEOY    0x02

#define MEMOEY_FLAGS    0xA5

#pragma pack(push,1)
typedef struct _dynamic_memory 
{
    unsigned long heap_size;
    unsigned int  heap_type;
    // 虽然下面heap定义在 heap_type 之后，但意思其实是在整个柔性数组的末尾字节 
#ifdef CHECK_MEMORY
    unsigned char heap[4];
#else
    unsigned char heap[2];
#endif
}dynamic_memory, *pdynamic_memory;
#pragma pack(pop)

#ifdef CHECK_MEMORY
#include <crtdbg.h>
// 检测内存是否溢出 
static __forceinline VOID ASSERT_MEMORY_OVERFLOW(LPVOID _addr, LPCTSTR message) 
{
    if (_addr != NULL)
    {
        pdynamic_memory lpBuf = (pdynamic_memory)CONTAINING_RECORD(_addr, dynamic_memory,heap);
        unsigned char* lpcheckbuf = lpBuf->heap + lpBuf->heap_size;
        if (lpcheckbuf[0] != 0x00 || lpcheckbuf[1] != 0x00 ||
            lpcheckbuf[2] != MEMOEY_FLAGS || lpcheckbuf[3] != MEMOEY_FLAGS)
        {
            // 发生了溢出 
            _ASSERT_EXPR(NULL, message);
        }
    }
}
#else // CHECK_MEMORY
#define ASSERT_MEMORY_OVERFLOW(...)
#endif // CHECK_MEMORY
#define ASSERT_MO(addr)  ASSERT_MEMORY_OVERFLOW(addr, TEXT("Memory is Overflow"))

// 申请内存,并全置零 
static __forceinline LPVOID AllocMemory(unsigned long _nsize) 
{
    pdynamic_memory lpBuf = (pdynamic_memory)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _nsize+sizeof(dynamic_memory)); 
    if (lpBuf == NULL)
    {
        // 内存申请失败，换个函数申请 
        lpBuf = (pdynamic_memory)malloc(_nsize+sizeof(dynamic_memory));
        if (lpBuf != NULL)
        {
            lpBuf->heap_type = MALLOCMEMEOY;
            memset(lpBuf->heap, 0, _nsize+sizeof(dynamic_memory));
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        lpBuf->heap_type = HEAPMEMEOY; 
    }
    lpBuf->heap_size = _nsize;
#ifdef CHECK_MEMORY
    // 最后4个字节，前两个字节作为宽字符的结束符(以防一些函数不会加结束符),后两个字符用于校验溢出 
    memset(lpBuf->heap+_nsize+sizeof(wchar_t), MEMOEY_FLAGS, 2);
#endif
    return lpBuf->heap;
}

// 释放内存 
#define FreeMemory(addr)   {FreeMemory_stub(addr); addr = NULL;} 
static __forceinline VOID FreeMemory_stub(LPVOID _addr)
{
    if (_addr != NULL)
    {
        ASSERT_MO(_addr); // 如果开启检测溢出，释放内存自动检测 
        pdynamic_memory lpBuf = (pdynamic_memory)CONTAINING_RECORD(_addr, dynamic_memory,heap);
        if (lpBuf != NULL)
        {
            switch(lpBuf->heap_type)
            {
            case HEAPMEMEOY:
                HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, lpBuf);
                break;
            case MALLOCMEMEOY:
                free(lpBuf);
                break;
            }
        }
        _addr = NULL;
    }
}

// 原基础上扩充或者缩减内存 （也可用于申请内存_addr为NULL) 
static __forceinline LPVOID ReAllocMemory(LPVOID _addr, DWORD _nsize)
{
    if (_addr == NULL)
    {
        return AllocMemory(_nsize);
    }

    // 利用系统函数重新申请 
    pdynamic_memory lpNewBuf = NULL;
    pdynamic_memory lpBuf = (pdynamic_memory)CONTAINING_RECORD(_addr, dynamic_memory,heap);
    switch(lpBuf->heap_type)
    {
    case HEAPMEMEOY:
        lpNewBuf = (pdynamic_memory)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lpBuf, _nsize+sizeof(dynamic_memory));
        break;
    case MALLOCMEMEOY:
        lpNewBuf = (pdynamic_memory)realloc(lpBuf, _nsize+sizeof(dynamic_memory));
        if (lpNewBuf != NULL && _nsize > lpNewBuf->heap_size)
        {
            memset(lpNewBuf->heap+lpNewBuf->heap_size, 0, _nsize-lpNewBuf->heap_size);
        }
        break;
    }

    // 系统重新申请失败，试试自己申请拷贝 
    if (lpNewBuf == NULL)
    {
        lpNewBuf = (pdynamic_memory)AllocMemory(_nsize+sizeof(dynamic_memory));
        if (lpNewBuf)
        {
            // 把原来的数据拷贝过来，再释放内存 
            memcpy(lpNewBuf->heap, _addr, lpBuf->heap_size>_nsize?_nsize:lpBuf->heap_size);
            // 释放之前的buffer 
            FreeMemory(_addr);
        }
        else
        {
            return NULL;
        }
    }

    lpBuf->heap_size = _nsize;
#ifdef CHECK_MEMORY
    // 最后4个字节，前两个字节作为宽字符的结束符(以防一些函数不会加结束符),后两个字符用于校验溢出 
//     memset(lpBuf->heap+_nsize, 0, sizeof(wchar_t));
    memset(lpBuf->heap+_nsize+sizeof(wchar_t), 0, 2);
#endif
    return lpBuf->heap;
}

#define DeleteHandle(_handle) {if(_handle != INVALID_HANDLE_VALUE) { CloseHandle(_handle); _handle = INVALID_HANDLE_VALUE;}} // 关闭句柄 

// 打开文件 
#define FileOpenA(_name, _openf, _sharef, _createf)  CreateFileA(_name, _openf, _sharef, NULL, _createf, FILE_ATTRIBUTE_NORMAL, NULL)  
#define FileOpenW(_name, _openf, _sharef, _createf)  CreateFileW(_name, _openf, _sharef, NULL, _createf, FILE_ATTRIBUTE_NORMAL, NULL)

#ifdef _UNICODE
#define FileOpen FileOpenW 
#else /* _UNICODE */
#define FileOpen FileOpenA 
#endif /* _UNICODE */


#if _MSC_VER < 1400 
EXTERN_C BOOL WINAPI GetFileSizeEx(HANDLE, PLARGE_INTEGER);
EXTERN_C BOOL WINAPI SetFilePointerEx(HANDLE, LARGE_INTEGER, PLARGE_INTEGER,DWORD);
#endif /* _MSC_VER */ 


#endif /* _ZXLY_COMMAND_DEFINES_HEADER_H_ */ 