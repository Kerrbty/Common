#ifndef _RELOAD_MYSELF_DLL_MODEL_
#define _RELOAD_MYSELF_DLL_MODEL_

#include <windows.h>
#include <tchar.h>


HMODULE LoadMyDlls( PTSTR szFileName, DWORD BaseAddress = 0 ); // ����DLL�ļ�
LPVOID MyGetProcAddress(HMODULE hmodule, PSTR szFuncName); // �õ�����������ַ
BOOL FreeMyDlls(HMODULE hmodule); // ж��dll



#endif  // _RELOAD_MYSELF_DLL_MODEL_