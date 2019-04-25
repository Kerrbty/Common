#ifndef _RELOAD_MYSELF_DLL_MODEL_
#define _RELOAD_MYSELF_DLL_MODEL_

#include <windows.h>
#include <tchar.h>
#define PAGE_SIZE 0x1000

#ifdef _UNICODE
#define LoadMyDlls LoadMyDllsW
#else
#define LoadMyDlls LoadMyDllsA
#endif

HMODULE LoadMyDllsA( PSTR szFileName, LPBYTE BaseAddress = 0 ); // 加载DLL文件
HMODULE LoadMyDllsW( PWSTR szFileName, LPBYTE BaseAddress = 0 );

LPVOID MyGetProcAddress(HMODULE hmodule, PSTR szFuncName); // 得到导出函数地址
BOOL FreeMyDlls(HMODULE hmodule, LPBYTE BaseAddress = NULL); // 卸载dll


BOOL FixDllHook(LPTSTR DllPathName); // 修复指定dll中的hook

#endif  // _RELOAD_MYSELF_DLL_MODEL_