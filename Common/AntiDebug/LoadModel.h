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

HMODULE LoadMyDllsA( PSTR szFileName, LPBYTE BaseAddress = 0 ); // ����DLL�ļ�
HMODULE LoadMyDllsW( PWSTR szFileName, LPBYTE BaseAddress = 0 );

LPVOID MyGetProcAddress(HMODULE hmodule, PSTR szFuncName); // �õ�����������ַ
BOOL FreeMyDlls(HMODULE hmodule, LPBYTE BaseAddress = NULL); // ж��dll


BOOL FixDllHook(LPTSTR DllPathName); // �޸�ָ��dll�е�hook

#endif  // _RELOAD_MYSELF_DLL_MODEL_