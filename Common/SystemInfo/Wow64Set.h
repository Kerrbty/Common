#ifndef _WOW64_SET_INFO_HEADER_
#define _WOW64_SET_INFO_HEADER_
#include <Windows.h>

EXTERN_C BOOL WINAPI EnableWow64FsRedirection(BOOL enable);  // ���������ļ��ض���
EXTERN_C BOOL WINAPI IsWow64();


#endif  // _WOW64_SET_INFO_HEADER_