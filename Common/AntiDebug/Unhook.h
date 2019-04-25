#ifndef _UN_INLINE_HOOK_2013_11_14_18_05_
#define _UN_INLINE_HOOK_2013_11_14_18_05_
#include <windows.h>
#include <tchar.h>

DWORD  DelInlineHook(HMODULE hmodule, PTSTR DllName = NULL);
DWORD  DelIATHook(HMODULE hModule);

#endif  // _UN_INLINE_HOOK_2013_11_14_18_05_