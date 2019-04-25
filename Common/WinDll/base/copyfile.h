#ifndef _ZW_HZ_COPY_FILE_2015_07_30_
#define _ZW_HZ_COPY_FILE_2015_07_30_
#include <Windows.h>
#include <tchar.h>



BOOL WINAPI CopyAllFileA(LPCSTR SrcDir, LPCSTR DstDir);
BOOL WINAPI CopyAllFileW(LPCWSTR SrcDir, LPCWSTR DstDir);




#ifdef _UNICODE
#define CopyAllFile CopyAllFileW
#else
#define CopyAllFile CopyAllFileA
#endif


#endif // _ZW_HZ_COPY_FILE_2015_07_30_