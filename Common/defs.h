// CommonDef.h ͨ�õ�windows�궨�� 
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
#define CHECK_MEMORY   // У�黺����������� 
#endif // _DEBUG

#define HEAPMEMEOY      0x01
#define MALLOCMEMEOY    0x02

#define MEMOEY_FLAGS    0xA5

#pragma pack(push,1)
typedef struct _dynamic_memory 
{
    unsigned long heap_size;
    unsigned int  heap_type;
    // ��Ȼ����heap������ heap_type ֮�󣬵���˼��ʵ�����������������ĩβ�ֽ� 
#ifdef CHECK_MEMORY
    unsigned char heap[4];
#else
    unsigned char heap[2];
#endif
}dynamic_memory, *pdynamic_memory;
#pragma pack(pop)

#ifdef CHECK_MEMORY
#include <crtdbg.h>
// ����ڴ��Ƿ���� 
static __forceinline VOID ASSERT_MEMORY_OVERFLOW(LPVOID _addr, LPCTSTR message) 
{
    if (_addr != NULL)
    {
        pdynamic_memory lpBuf = (pdynamic_memory)CONTAINING_RECORD(_addr, dynamic_memory,heap);
        unsigned char* lpcheckbuf = lpBuf->heap + lpBuf->heap_size;
        if (lpcheckbuf[0] != 0x00 || lpcheckbuf[1] != 0x00 ||
            lpcheckbuf[2] != MEMOEY_FLAGS || lpcheckbuf[3] != MEMOEY_FLAGS)
        {
            // ��������� 
            _ASSERT_EXPR(NULL, message);
        }
    }
}
#else // CHECK_MEMORY
#define ASSERT_MEMORY_OVERFLOW(...)
#endif // CHECK_MEMORY
#define ASSERT_MO(addr)  ASSERT_MEMORY_OVERFLOW(addr, TEXT("Memory is Overflow"))

// �����ڴ�,��ȫ���� 
static __forceinline LPVOID AllocMemory(unsigned long _nsize) 
{
    pdynamic_memory lpBuf = (pdynamic_memory)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _nsize+sizeof(dynamic_memory)); 
    if (lpBuf == NULL)
    {
        // �ڴ�����ʧ�ܣ������������� 
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
    // ���4���ֽڣ�ǰ�����ֽ���Ϊ���ַ��Ľ�����(�Է�һЩ��������ӽ�����),�������ַ�����У����� 
    memset(lpBuf->heap+_nsize+sizeof(wchar_t), MEMOEY_FLAGS, 2);
#endif
    return lpBuf->heap;
}

// �ͷ��ڴ� 
#define FreeMemory(addr)   {FreeMemory_stub(addr); addr = NULL;} 
static __forceinline VOID FreeMemory_stub(LPVOID _addr)
{
    if (_addr != NULL)
    {
        ASSERT_MO(_addr); // ����������������ͷ��ڴ��Զ���� 
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

// ԭ������������������ڴ� ��Ҳ�����������ڴ�_addrΪNULL) 
static __forceinline LPVOID ReAllocMemory(LPVOID _addr, DWORD _nsize)
{
    if (_addr == NULL)
    {
        return AllocMemory(_nsize);
    }

    // ����ϵͳ������������ 
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

    // ϵͳ��������ʧ�ܣ������Լ����뿽�� 
    if (lpNewBuf == NULL)
    {
        lpNewBuf = (pdynamic_memory)AllocMemory(_nsize+sizeof(dynamic_memory));
        if (lpNewBuf)
        {
            // ��ԭ�������ݿ������������ͷ��ڴ� 
            memcpy(lpNewBuf->heap, _addr, lpBuf->heap_size>_nsize?_nsize:lpBuf->heap_size);
            // �ͷ�֮ǰ��buffer 
            FreeMemory(_addr);
        }
        else
        {
            return NULL;
        }
    }

    lpBuf->heap_size = _nsize;
#ifdef CHECK_MEMORY
    // ���4���ֽڣ�ǰ�����ֽ���Ϊ���ַ��Ľ�����(�Է�һЩ��������ӽ�����),�������ַ�����У����� 
//     memset(lpBuf->heap+_nsize, 0, sizeof(wchar_t));
    memset(lpBuf->heap+_nsize+sizeof(wchar_t), 0, 2);
#endif
    return lpBuf->heap;
}

#define DeleteHandle(_handle) {if(_handle != INVALID_HANDLE_VALUE) { CloseHandle(_handle); _handle = INVALID_HANDLE_VALUE;}} // �رվ�� 

// ���ļ� 
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