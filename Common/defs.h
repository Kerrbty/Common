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

#define HEAPMEMEOY      0x01
#define MALLOCMEMEOY    0x02

#pragma pack(push,1)
typedef struct _dynamic_memory 
{
    unsigned long heap_size;
    unsigned int  heap_type;
    unsigned char heap[1];
}dynamic_memory, *pdynamic_memory;
#pragma pack(pop)


// #define AllocMemory(_nsize)  {int _a = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _nsize); if (_a == NULL){printf("err: %d\n", GetLastError()); }}  // 按字节申请内存，在C运行库未初始化情况下就可用 
static inline LPVOID AllocMemory(unsigned long _nsize) 
{
    pdynamic_memory lpBuf = (pdynamic_memory)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _nsize+sizeof(dynamic_memory)); 
    if (lpBuf == NULL)
    {
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
    return lpBuf->heap;
}

// #define ReAllocMemory(_addr, _nsize)  HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _addr, _nsize) // 在原基础上增加、减少否内存 
static inline LPVOID ReAllocMemory(LPVOID _addr, DWORD _nsize)
{
    pdynamic_memory lpBuf = (pdynamic_memory)CONTAINING_RECORD(_addr, dynamic_memory,heap);
    if (lpBuf == NULL)
    {
        return AllocMemory(_nsize);
    }
    else
    {
        switch(lpBuf->heap_type)
        {
        case HEAPMEMEOY:
            lpBuf = (pdynamic_memory)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lpBuf, _nsize+sizeof(dynamic_memory));
            break;
        case MALLOCMEMEOY:
            lpBuf = (pdynamic_memory)realloc(lpBuf, _nsize+sizeof(dynamic_memory));
            if (lpBuf != NULL && _nsize > lpBuf->heap_size)
            {
                memset(lpBuf->heap+lpBuf->heap_size, 0, _nsize-lpBuf->heap_size);
            }
            break;
        }
    }
    if (lpBuf == NULL)
    {
        return NULL;
    }

    lpBuf->heap_size = _nsize;
    return lpBuf->heap;
}

// #define FreeMemory(_addr)   { if (_addr) { HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, _addr); _addr=NULL; } }  // 释放内存
static inline VOID FreeMemory(LPVOID _addr)
{
    if (_addr != NULL)
    {
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
        addr = NULL;
    }
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