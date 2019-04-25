#ifndef _RELOAD_MYSELF_DLL_MODEL_
#define _RELOAD_MYSELF_DLL_MODEL_

#include <windows.h>
#include <tchar.h>


HMODULE LoadMyDlls( PTSTR szFileName, DWORD BaseAddress = 0 ); // 加载DLL文件
LPVOID MyGetProcAddress(HMODULE hmodule, PSTR szFuncName); // 得到导出函数地址
BOOL FreeMyDlls(HMODULE hmodule); // 卸载dll



#endif  // _RELOAD_MYSELF_DLL_MODEL_