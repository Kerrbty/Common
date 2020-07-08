#ifndef _HHCHINA_MNAME_COMMONDEF_
#define _HHCHINA_MNAME_COMMONDEF_
#include <windows.h>

// DeviceName 转化为 DosPathName
LPTSTR DevicePathNameToDosPathName(const LPTSTR DevicePathName, LPTSTR DosPathName, DWORD dwLen);

// 查找最后一个'\'符号位置
LPWSTR FindFileNameFromFullPathW(LPWSTR CommandLine, WCHAR FindWchar = L'\\');

// 查找最后一个'\'符号位置
LPSTR FindFileNameFromFullPathA(LPSTR CommandLine, CHAR FindChar = '\\');


#ifdef UNICODE
#define FindFileNameFromFullPath FindFileNameFromFullPathW
#define tstring wstring
#else

#define FindFileNameFromFullPath FindFileNameFromFullPathA
#define tstring string
#endif // UNICODE

#endif // _HHCHINA_MNAME_COMMONDEF_