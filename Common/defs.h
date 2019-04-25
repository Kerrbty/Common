// CommonDef.h 通用的windows宏定义 
// 
#ifndef _ZXLY_COMMAND_DEFINES_HEADER_H_
#define _ZXLY_COMMAND_DEFINES_HEADER_H_

#include <windows.h>
#include <tchar.h>

#define AllocMemory(_nsize)  HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _nsize)  // 按字节申请内存，在C运行库未初始化情况下就可用 
#define ReAllocMemory(_addr, _nsize)  HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _addr, _nsize) // 在原基础上增加、减少否内存 
#define FreeMemory(_addr)   { if (_addr) { HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, _addr); _addr=NULL; } }  // 释放内存
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
extern "C" BOOL WINAPI GetFileSizeEx(HANDLE, PLARGE_INTEGER);
extern "C" BOOL WINAPI SetFilePointerEx(HANDLE, LARGE_INTEGER, PLARGE_INTEGER,DWORD);
#endif /* _MSC_VER */ 



#endif /* _ZXLY_COMMAND_DEFINES_HEADER_H_ */