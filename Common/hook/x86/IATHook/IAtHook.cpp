#include "IATHook.h"
#include <ImageHlp.h>
#include <Tlhelp32.h>
#include "LoadModel.h"
#include "mykernel32.h"
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


PVOID GetImportTableAddr(HMODULE hModule)
{
    __try
    {
        if( isPEFile((LPVOID)hModule) )
        {
            PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)hModule;
            PIMAGE_NT_HEADERS peheader = 
                (PIMAGE_NT_HEADERS)((DWORD)hModule + Header->e_lfanew);
            if (peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0)
            {
                return NULL;
            }
            
            DWORD  ImportTable = ( (DWORD)hModule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) ;
            
            if (ImportTable> peheader->OptionalHeader.ImageBase && ImportTable < (DWORD)hModule+peheader->OptionalHeader.SizeOfImage)
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
{ // ȡ��ģ��ĵ����import descriptor���׵�ַ��ImageDirectoryEntryToData �������Է��ص�����ַ
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)GetImportTableAddr(hModCaller);
                                    
    if(pImportDesc == NULL) // ���ģ��û�е���ڱ�
        return;
    // ���Ұ���pszExportMod ģ���к���������Ϣ�ĵ������
    while(pImportDesc->Name != 0)
    { 
        LPSTR pszMod = (LPSTR)((DWORD)hModCaller + pImportDesc->Name);
        if(lstrcmpiA(pszMod, pszExportMod) == 0) // �ҵ�ָ����dll
        {
            // ȡ�õ����ߵĵ����ַ��import address table, IAT��
            PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)
                                        (pImportDesc->FirstThunk + (DWORD)hModCaller);
            // ��������ҪHOOK �ĺ����������ĵ�ַ���º����ĵ�ַ�滻��
            while(pThunk->u1.Function)
            { // lpAddr ָ����ڴ汣���˺����ĵ�ַ
                PDWORD lpAddr = (PDWORD)&(pThunk->u1.Function);
                if( !IsBadReadPtr(lpAddr, 4) && *lpAddr == (DWORD)pfnCurrent)
                { // �޸�ҳ�ı�������
                    // �޸��ڴ��ַ �൱�ڡ�*lpAddr = (DWORD)pfnNew;��
                    ::WriteProcessMemory(   ::GetCurrentProcess(),
                                            lpAddr, 
                                            &pfnNew, 
                                            sizeof(DWORD), 
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
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (PCHAR)ReplaceIATEntryInAllMods, &hModThis);
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
            OutputDebugString("\n");
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
        LPSTR pszMod = (LPSTR)((DWORD)hModCaller + pImportDesc->Name);
        if( lstrcmpiA(pszMod, pszExportMod) == 0 ) // �ҵ�ָ����dll
        {
            // ȡ�� IMAGE_THUNK_DATA
            PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)
                                        (pImportDesc->OriginalFirstThunk + (DWORD)hModCaller);
            DWORD* funaddr = (DWORD*)(pImportDesc->FirstThunk + (DWORD)hModCaller);

            // ��������ҪHOOK �ĺ����������ĵ�ַ���º����ĵ�ַ�滻��
            DWORD Orig = 0;
            __try
            {
                while(pThunk->u1.ForwarderString)
                { 
                    PHintName FuncHint = (PHintName)(pThunk->u1.ForwarderString+(DWORD)hModCaller);
                    if (FuncHint->Hint &&lstrcmpiA(FuncHint->Name, pfnFuncName) == 0)
                    {
                        // �޸�ҳ�ı�������
                        PROC findfunaddr = (PROC)&funaddr[Orig];
                        // �޸��ڴ��ַ �൱�ڡ�*lpAddr = (DWORD)pfnNew;��
                        ::WriteProcessMemory(   ::GetCurrentProcess(),
                            findfunaddr, 
                            &pfnNew, 
                            sizeof(DWORD), 
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
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (PCHAR)ReplaceIATEntryInAllMods, &hModThis);
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
            OutputDebugString("\n");
            ReplaceIATEntryInOneName(pszExportMod, pfnFuncName, pfnNew, me.hModule);
        }
        bOK = ::Module32Next(hSnap, &me);
    }
    ::CloseHandle(hSnap);
}