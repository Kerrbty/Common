#ifndef _IAT_HOOK_MODULE_INFO_H_
#define _IAT_HOOK_MODULE_INFO_H_
#include <Windows.h>

namespace CAPIHook
{
	// ͨ������IAT HOOK ����ģ��
	void ReplaceIATEntryInAllMods(LPSTR pszExportMod,
		PROC pfnCurrent, 
		PROC pfnNew, 
		BOOL bExcludeAPIHookMod); // �Ƿ��ų�����
	// ͨ��IAT HOOK һ��ģ��
	void ReplaceIATEntryInOneMod(LPSTR pszExportMod,
		PROC pfnCurrent, 
		PROC pfnNew, 
		HMODULE hModCaller); 

	// ͨ������IMT  Hook һ��ģ��
	void ReplaceIATEntryInOneName(LPSTR pszExportMod,
		LPSTR pfnFuncName, 
		PROC pfnNew, 
		HMODULE hModCaller);

	// ͨ������IMT Hook ����ģ��
	void ReplaceIATEntryInAllNames(LPSTR pszExportMod,
		LPSTR pfnFuncName, 
		PROC pfnNew, 
		BOOL bExcludeAPIHookMod); // �Ƿ��ų�����
};

#endif