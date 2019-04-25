#include "stdafx.h"
#include "IatHook.h"
#include <ImageHlp.h>
#include <Tlhelp32.h>
#pragma comment(lib, "ImageHlp")



#pragma pack(push, 1)
// �ض�λ��ṹ
typedef struct _OffTable{
	USHORT addr:12;
	USHORT flags:4;
}OffTable, *pOffTable;

typedef struct _RELOADTABLE{
	DWORD StartVirtualAddress;
	DWORD size;
	OffTable Table[1];
}RELOADTABLE, *pRELOADTABLE;

// �����ṹ
typedef struct _HintName
{
	WORD Hint;
	CHAR Name[1];
}HintName, *PHintName;

#pragma pack(pop)

// �ж�PE�ļ��Ƿ�Ϸ�
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
		peheader->FileHeader.Machine != 0x8664 ) // ���AMD64
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
	// ȡ��ģ��ĵ����import descriptor���׵�ַ��ImageDirectoryEntryToData �������Է��ص�����ַ
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)GetImportTableAddr(hModCaller);

	if(pImportDesc == NULL) // ���ģ��û�е���ڱ�
		return;
	// ���Ұ���pszExportMod ģ���к���������Ϣ�ĵ������
	while(pImportDesc->Name != 0)
	{ 
		LPSTR pszMod = (LPSTR)((DWORD64)hModCaller + pImportDesc->Name);
		if(lstrcmpiA(pszMod, pszExportMod) == 0) // �ҵ�ָ����dll
		{
			// ȡ�õ����ߵĵ����ַ��import address table, IAT��
			PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)
				(pImportDesc->FirstThunk + (DWORD64)hModCaller);
			// ��������ҪHOOK �ĺ����������ĵ�ַ���º����ĵ�ַ�滻��
			while(pThunk->u1.Function)
			{ // lpAddr ָ����ڴ汣���˺����ĵ�ַ
				PDWORD64 lpAddr = (PDWORD64)&(pThunk->u1.Function);
				if( !IsBadReadPtr(lpAddr, 4) && *lpAddr == (DWORD64)pfnCurrent)
				{ // �޸�ҳ�ı�������
					// �޸��ڴ��ַ �൱�ڡ�*lpAddr = (DWORD)pfnNew;��
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
{ // ȡ�õ�ǰģ��ľ��
	HMODULE hModThis = (HMODULE)-1;
	if(bExcludeAPIHookMod)
	{ 
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)ReplaceIATEntryInAllMods, &hModThis);
	}
	// ȡ�ñ����̵�ģ���б�
	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ::GetCurrentProcessId());
	// ��������ģ�飬�ֱ�����ǵ���ReplaceIATEntryInOneMod �������޸ĵ����ַ��
	MODULEENTRY32 me = { sizeof(MODULEENTRY32) };
	BOOL bOK = ::Module32First(hSnap, &me);
	while(bOK)
	{ // ע�⣺���ǲ�HOOK ��ǰģ��ĺ���
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
	if(pImportDesc == NULL) // ���ģ��û�е���ڱ�
		return;
	// ���Ұ���pszExportMod ģ���к���������Ϣ�ĵ������
	while(pImportDesc->Name != 0)
	{ 
		LPSTR pszMod = (LPSTR)((DWORD64)hModCaller + pImportDesc->Name);
		if( lstrcmpiA(pszMod, pszExportMod) == 0 ) // �ҵ�ָ����dll
		{
			// ȡ�� IMAGE_THUNK_DATA
			PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)
				(pImportDesc->OriginalFirstThunk + (DWORD64)hModCaller);
			DWORD64* funaddr = (DWORD64*)(pImportDesc->FirstThunk + (DWORD64)hModCaller);

			// ��������ҪHOOK �ĺ����������ĵ�ַ���º����ĵ�ַ�滻��
			DWORD Orig = 0;
			__try
			{
				while(pThunk->u1.ForwarderString)
				{ 
					PHintName FuncHint = (PHintName)(pThunk->u1.ForwarderString+(DWORD64)hModCaller);
					if (FuncHint->Hint &&lstrcmpiA(FuncHint->Name, pfnFuncName) == 0)
					{
						// �޸�ҳ�ı�������
						PROC findfunaddr = (PROC)&funaddr[Orig];
						// �޸��ڴ��ַ �൱�ڡ�*lpAddr = (DWORD64)pfnNew;��
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
{ // ȡ�õ�ǰģ��ľ��
	HMODULE hModThis = (HMODULE)-1;
	if(bExcludeAPIHookMod)
	{ 
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)ReplaceIATEntryInAllMods, &hModThis);
	}
	// ȡ�ñ����̵�ģ���б�
	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ::GetCurrentProcessId());
	// ��������ģ�飬�ֱ�����ǵ���ReplaceIATEntryInOneMod �������޸ĵ����ַ��
	MODULEENTRY32 me = { sizeof(MODULEENTRY32) };
	BOOL bOK = ::Module32First(hSnap, &me);
	while(bOK)
	{ // ע�⣺���ǲ�HOOK ��ǰģ��ĺ���
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