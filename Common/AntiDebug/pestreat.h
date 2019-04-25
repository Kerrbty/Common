#ifndef _PE_STRUCT_2013_11_04_17_55_
#define _PE_STRUCT_2013_11_04_17_55_
#include <windows.h>
#include <tchar.h>
#define PAGE_SIZE 0x1000

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

typedef struct _DllMainCall 
{
	HMODULE hmodule;
	DWORD reasons;
}DllMainCall, *pDllMainCall;

#pragma pack(pop)


#endif  // _PE_STRUCT_2013_11_04_17_55_