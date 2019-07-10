#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>

std::map<int, std::string> ssdt;

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

void EnumSSDT() 
{
    int i=0;
    ssdt.clear();
    HMODULE hModule = LoadLibrary(TEXT("ntdll.dll"));

    PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS peheader = 
        (PIMAGE_NT_HEADERS)((LPBYTE)Header + Header->e_lfanew);

    // 导出表地址
    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY) // (pRELOADTABLE)
        ( (LPBYTE)Header + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    PDWORD NameRVA = (PDWORD)((LPBYTE)Header + pExportDir->AddressOfNames );
    PDWORD FuncAddr = (PDWORD)((LPBYTE)Header + pExportDir->AddressOfFunctions);
    PWORD Ordinal = (PWORD)((LPBYTE)Header + pExportDir->AddressOfNameOrdinals);

    // 遍历以名称导出的函数
    for (i=0; i<pExportDir->NumberOfNames; i++)
    {
        char* ProcName = (PSTR)(NameRVA[i]+(LPBYTE)Header);
        WORD Hint = Ordinal[i];
        LPBYTE funaddr = (LPBYTE)Header+FuncAddr[Hint];
#ifdef _WIN64
        if (ProcName[0] == 'N' && ProcName[1] == 't' && memcmp(funaddr, "\x4C\x8B\xD1\xB8", 4) == 0)
        {
            ssdt[*(DWORD*)(funaddr+4)] = ProcName;
        }
#else
        if (ProcName[0] == 'N' && ProcName[1] == 't' && funaddr[0] == 0xB8)
        {
            ssdt[*(DWORD*)(funaddr+1)] = ProcName;
        }
#endif
    }

    for (i=0; i<0x1000; i++)
    {
        if (ssdt.find(i) != ssdt.end())
        {
            printf("%d\t%s\n", i, ssdt[i].c_str());
        }
    }

}

int main()
{
    EnumSSDT();
    return 0;
}