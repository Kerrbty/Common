#ifndef _IAT_HOOK_MODULE_INFO_H_
#define _IAT_HOOK_MODULE_INFO_H_
#include <Windows.h>

namespace CAPIHook
{
	// 通过查找IAT HOOK 所有模块
	void ReplaceIATEntryInAllMods(LPSTR pszExportMod,
		PROC pfnCurrent, 
		PROC pfnNew, 
		BOOL bExcludeAPIHookMod); // 是否排除自身？
	// 通过IAT HOOK 一个模块
	void ReplaceIATEntryInOneMod(LPSTR pszExportMod,
		PROC pfnCurrent, 
		PROC pfnNew, 
		HMODULE hModCaller); 

	// 通过查找IMT  Hook 一个模块
	void ReplaceIATEntryInOneName(LPSTR pszExportMod,
		LPSTR pfnFuncName, 
		PROC pfnNew, 
		HMODULE hModCaller);

	// 通过查找IMT Hook 所有模块
	void ReplaceIATEntryInAllNames(LPSTR pszExportMod,
		LPSTR pfnFuncName, 
		PROC pfnNew, 
		BOOL bExcludeAPIHookMod); // 是否排除自身？
};

#endif