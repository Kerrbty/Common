// Debug.h 调试信息打印 
// 输出内容可在DbgView.exe、Windbg.exe、VS的输出结果中查看 
// 
#ifndef _ZXLY_DEBUG_HEADER_INFO_H_
#define _ZXLY_DEBUG_HEADER_INFO_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

#ifdef _DEBUG 
void createcui( );  // 创建一个可以printf的窗口

void _dbgprintA(const char* lpszFormant, ...);  
void _dbgprintW(const wchar_t* lpszFormant, ...);

int _dbgfprintA(FILE* stream, const char* lpszFormant, ...);  
int _dbgfprintW(FILE* stream, const wchar_t* lpszFormant, ...);
#else /* _DEBUG */ 
#define createcui()
#define _dbgprintA(...)
#define _dbgfprintA(...)
#define _dbgprintW(...)
#define _dbgfprintW(...)
#endif /* _DEBUG */ 


//////////////////////////////////////////////////////////////////////////
#define dbgprintW _dbgprintW
#define dbgfprintW _dbgfprintW
#define dbgprintA _dbgprintA
#define dbgfprintA _dbgfprintA

#ifdef _UNICODE
#define dbgprint dbgprintW
#define dbgfprint dbgfprintW
#else
#define dbgprint dbgprintA
#define dbgfprint dbgfprintA
#endif /* _UNICODE */ 



#ifdef __cplusplus
};
#endif
#endif  // _ZXLY_DEBUG_HEADER_INFO_H_