// Debug.h ������Ϣ��ӡ 
// ������ݿ���DbgView.exe��Windbg.exe��VS���������в鿴 
// 
#ifndef _ZXLY_DEBUG_HEADER_INFO_H_
#define _ZXLY_DEBUG_HEADER_INFO_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

#ifdef _DEBUG 
void createcui( );  // ����һ������printf�Ĵ���

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