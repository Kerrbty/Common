#include "stdafx.h"
#include "IatHook.h"
#include <ImageHlp.h>
#include <Tlhelp32.h>
#pragma comment(lib, "ImageHlp")



#pragma pack(push, 1)
// 重定位表结构
typedef struct _OffTable{
	USHORT addr:12;
	USHORT flags:4;
}OffTable, *pOffTable;

typedef struct _RELOADTABLE{
	DWORD StartVirtualAddress;
	DWORD size;
	OffTable Table[1];
}RELOADTABLE, *pRELOADTABLE;

// 导入表结构
typedef struct _HintName
{
	WORD Hint;
	CHAR Name[1];
}HintName, *PHintName;

#pragma pack(pop)

// 判断PE文件是否合法
BOOL isPEFile(LPVOID pFileMap)
{
	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)pFileMap;

	if( Header->e_magic != 'ZM' )
	{
		return FALSE;
	}

	if ( Header->e_lfanew > 0x1000 || Header->e_lfanew < 0 )
	{
		return FALSE;
	}

	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((DWORD64)Header + Header->e_lfanew);

	if (peheader->Signature != 'EP' || 
		peheader->FileHeader.Machine != 0x8664 ) // 针对AMD64
	{
		return FALSE;
	}

	return TRUE;
}

PVOID GetImportTableAddr(HMODULE hModule)
{
	__try
	{
		if( isPEFile((LPVOID)hModule) )
		{
			PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)hModule;
			PIMAGE_NT_HEADERS peheader = 
				(PIMAGE_NT_HEADERS)((DWORD64)hModule + Header->e_lfanew);
			if (peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0)
			{
				return NULL;
			}

			DWORD64  ImportTable = ( (DWORD64)hModule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) ;

			if (ImportTable> peheader->OptionalHeader.ImageBase && ImportTable < (DWORD64)hModule+peheader->OptionalHeader.SizeOfImage)
			{
				return (PVOID)ImportTable;
			}
			return NULL;
		}
		return NULL;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return NULL;
	}
	return NULL;
}

void CAPIHook::ReplaceIATEntryInOneMod(LPSTR pszExportMod,
									   PROC pfnCurrent, 
									   PROC pfnNew, 
									   HMODULE hModCaller)
{
	// 取得模块的导入表（import descriptor）首地址。ImageDirectoryEntryToData 函数可以返回导入表地址
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)GetImportTableAddr(hModCaller);

	if(pImportDesc == NULL) // 这个模块没有导入节表
		return;
	// 查找包含pszExportMod 模块中函数导入信息的导入表项
	while(pImportDesc->Name != 0)
	{ 
		LPSTR pszMod = (LPSTR)((DWORD64)hModCaller + pImportDesc->Name);
		if(lstrcmpiA(pszMod, pszExportMod) == 0) // 找到指定的dll
		{
			// 取得调用者的导入地址表（import address table, IAT）
			PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)
				(pImportDesc->FirstThunk + (DWORD64)hModCaller);
			// 查找我们要HOOK 的函数，将它的地址用新函数的地址替换掉
			while(pThunk->u1.Function)
			{ // lpAddr 指向的内存保存了函数的地址
				PDWORD64 lpAddr = (PDWORD64)&(pThunk->u1.Function);
				if( !IsBadReadPtr(lpAddr, 4) && *lpAddr == (DWORD64)pfnCurrent)
				{ // 修改页的保护属性
					// 修改内存地址 相当于“*lpAddr = (DWORD)pfnNew;”
					::WriteProcessMemory(   ::GetCurrentProcess(),
						lpAddr, 
						&pfnNew, 
						sizeof(DWORD64), 
						NULL);
					break;
				}
				pThunk++;
			}
		}
		pImportDesc++;
	}
}

void CAPIHook::ReplaceIATEntryInAllMods(LPSTR pszExportMod,
										PROC pfnCurrent, 
										PROC pfnNew, 
										BOOL bExcludeAPIHookMod)
{ // 取得当前模块的句柄
	HMODULE hModThis = (HMODULE)-1;
	if(bExcludeAPIHookMod)
	{ 
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)ReplaceIATEntryInAllMods, &hModThis);
	}
	// 取得本进程的模块列表
	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ::GetCurrentProcessId());
	// 遍历所有模块，分别对它们调用ReplaceIATEntryInOneMod 函数，修改导入地址表
	MODULEENTRY32 me = { sizeof(MODULEENTRY32) };
	BOOL bOK = ::Module32First(hSnap, &me);
	while(bOK)
	{ // 注意：我们不HOOK 当前模块的函数
		if(me.hModule != hModThis)
		{
			OutputDebugString(me.szExePath);
			OutputDebugStringA("\n");
			ReplaceIATEntryInOneMod(pszExportMod, pfnCurrent, pfnNew, me.hModule);
		}
		bOK = ::Module32Next(hSnap, &me);
	}
	::CloseHandle(hSnap);
}




void CAPIHook::ReplaceIATEntryInOneName(LPSTR pszExportMod,
										LPSTR pfnFuncName, 
										PROC pfnNew, 
										HMODULE hModCaller)
{ 
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)GetImportTableAddr(hModCaller);
	if(pImportDesc == NULL) // 这个模块没有导入节表
		return;
	// 查找包含pszExportMod 模块中函数导入信息的导入表项
	while(pImportDesc->Name != 0)
	{ 
		LPSTR pszMod = (LPSTR)((DWORD64)hModCaller + pImportDesc->Name);
		if( lstrcmpiA(pszMod, pszExportMod) == 0 ) // 找到指定的dll
		{
			// 取得 IMAGE_THUNK_DATA
			PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)
				(pImportDesc->OriginalFirstThunk + (DWORD64)hModCaller);
			DWORD64* funaddr = (DWORD64*)(pImportDesc->FirstThunk + (DWORD64)hModCaller);

			// 查找我们要HOOK 的函数，将它的地址用新函数的地址替换掉
			DWORD Orig = 0;
			__try
			{
				while(pThunk->u1.ForwarderString)
				{ 
					PHintName FuncHint = (PHintName)(pThunk->u1.ForwarderString+(DWORD64)hModCaller);
					if (FuncHint->Hint &&lstrcmpiA(FuncHint->Name, pfnFuncName) == 0)
					{
						// 修改页的保护属性
						PROC findfunaddr = (PROC)&funaddr[Orig];
						// 修改内存地址 相当于“*lpAddr = (DWORD64)pfnNew;”
						::WriteProcessMemory(   ::GetCurrentProcess(),
							findfunaddr, 
							&pfnNew, 
							sizeof(DWORD64), 
							NULL);
						break;
					}
					Orig++;
					pThunk++;
				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				pImportDesc++;
				continue;
			}

		}
		pImportDesc++;
	}
}


void CAPIHook::ReplaceIATEntryInAllNames(LPSTR pszExportMod,
										 LPSTR pfnFuncName, 
										 PROC pfnNew, 
										 BOOL bExcludeAPIHookMod)
{ // 取得当前模块的句柄
	HMODULE hModThis = (HMODULE)-1;
	if(bExcludeAPIHookMod)
	{ 
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)ReplaceIATEntryInAllMods, &hModThis);
	}
	// 取得本进程的模块列表
	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ::GetCurrentProcessId());
	// 遍历所有模块，分别对它们调用ReplaceIATEntryInOneMod 函数，修改导入地址表
	MODULEENTRY32 me = { sizeof(MODULEENTRY32) };
	BOOL bOK = ::Module32First(hSnap, &me);
	while(bOK)
	{ // 注意：我们不HOOK 当前模块的函数
		if(me.hModule != hModThis)
		{
			OutputDebugString(me.szExePath);
			OutputDebugStringA("\n");
			ReplaceIATEntryInOneName(pszExportMod, pfnFuncName, pfnNew, me.hModule);
		}
		bOK = ::Module32Next(hSnap, &me);
	}
	::CloseHandle(hSnap);
}