#ifndef _HHCHINA_MNAME_COMMONDEF_
#define _HHCHINA_MNAME_COMMONDEF_
#include <windows.h>

// DeviceName ת��Ϊ DosPathName
LPTSTR DevicePathNameToDosPathName(const LPTSTR DevicePathName, LPTSTR DosPathName, DWORD dwLen);

// �������һ��'\'����λ��
LPWSTR FindFileNameFromFullPathW(LPWSTR CommandLine, WCHAR FindWchar = L'\\');

// �������һ��'\'����λ��
LPSTR FindFileNameFromFullPathA(LPSTR CommandLine, CHAR FindChar = '\\');


#ifdef UNICODE
#define FindFileNameFromFullPath FindFileNameFromFullPathW
#define tstring wstring
#else

#define FindFileNameFromFullPath FindFileNameFromFullPathA
#define tstring string
#endif // UNICODE

#endif // _HHCHINA_MNAME_COMMONDEF_