#pragma once

#if _MSC_VER > 1300
#include <Imagehlp.h>
#else
#include <Imagehlp.h>
#include <windows.h>
#include <tchar.h>
#pragma comment(lib, "DbgHelp")

#ifdef __cplusplus
extern "C" {
#endif

#define _In_
#define _Out_
#define _In_opt_
#define _Out_opt_
typedef CONST PTSTR  PCTSTR;

typedef struct _SYMBOL_INFO {
    ULONG   SizeOfStruct;
    ULONG   TypeIndex;
    ULONG64 Reserved[2];
    ULONG   Index;
    ULONG   Size;
    ULONG64 ModBase;
    ULONG   Flags;
    ULONG64 Value;
    ULONG64 Address;
    ULONG   Register;
    ULONG   Scope;
    ULONG   Tag;
    ULONG   NameLen;
    ULONG   MaxNameLen;
    TCHAR   Name[1];
} SYMBOL_INFO, *PSYMBOL_INFO;

typedef struct _IMAGEHLP_SYMBOL64 {
    DWORD   SizeOfStruct;
    DWORD64 Address;
    DWORD   Size;
    DWORD   Flags;
    DWORD   MaxNameLength;
    TCHAR   Name[1];
} IMAGEHLP_SYMBOL64, *PIMAGEHLP_SYMBOL64;


BOOL WINAPI SymGetSymbolFile(
                              HANDLE hProcess,
                              PCTSTR SymPath,
                              PCTSTR ImageFile,
                              DWORD Type,
                              PTSTR SymbolFile,
                              size_t cSymbolFile,
                              PTSTR DbgFile,
                              size_t cDbgFile
);


DWORD64 WINAPI SymLoadModule64(
                               _In_      HANDLE hProcess,
                               _In_opt_  HANDLE hFile,
                               _In_opt_  PCSTR ImageName,
                               _In_opt_  PCSTR ModuleName,
                               _In_      DWORD64 BaseOfDll,
                               _In_      DWORD SizeOfDll
);

BOOL WINAPI SymGetSymFromAddr64(    HANDLE hProcess,             
                                    DWORD64 Address,                 
                                    PDWORD64 Displacement,       
                                    PIMAGEHLP_SYMBOL64 Symbol  
                         );


typedef BOOL (CALLBACK *PSYM_ENUMPROCESSES_CALLBACK)(
                                                        HANDLE hProcess,
                                                        PVOID UserContext
);

BOOL WINAPI SymEnumProcesses(
                             _In_  PSYM_ENUMPROCESSES_CALLBACK EnumProcessesCallback,
                             _In_  PVOID UserContext
);

BOOL WINAPI SymRefreshModuleList(
                                 _In_  HANDLE hProcess
);

BOOL WINAPI SymUnloadModule64(
                              _In_  HANDLE hProcess,
                              _In_  DWORD64 BaseOfDll
);

typedef struct _MODLOAD_DATA {
    DWORD ssize;
    DWORD ssig;
    PVOID data;
    DWORD size;
    DWORD flags;
} MODLOAD_DATA, *PMODLOAD_DATA;

DWORD64 WINAPI SymLoadModuleEx(
                               _In_  HANDLE hProcess,
                               _In_  HANDLE hFile,
                               _In_  PCTSTR ImageName,
                               _In_  PCTSTR ModuleName,
                               _In_  DWORD64 BaseOfDll,
                               _In_  DWORD DllSize,
                               _In_  PMODLOAD_DATA Data,
                               _In_  DWORD Flags
);

BOOL CALLBACK SymEnumSymbolsProc(
                                 _In_      PSYMBOL_INFO pSymInfo,
                                 _In_      ULONG SymbolSize,
                                 _In_opt_  PVOID UserContext
                                 );

// typedef struct _SYMBOL_INFO {
//     ULONG   SizeOfStruct;
//     ULONG   TypeIndex;
//     ULONG64 Reserved[2];
//     ULONG   Index;
//     ULONG   Size;
//     ULONG64 ModBase;
//     ULONG   Flags;
//     ULONG64 Value;
//     ULONG64 Address;
//     ULONG   Register;
//     ULONG   Scope;
//     ULONG   Tag;
//     ULONG   NameLen;
//     ULONG   MaxNameLen;
//     TCHAR   Name[1];
// } SYMBOL_INFO, *PSYMBOL_INFO;

typedef BOOL (CALLBACK *PSYM_ENUMERATESYMBOLS_CALLBACK)(
                                                         _In_      PSYMBOL_INFO pSymInfo,
                                                         _In_      ULONG SymbolSize,
                                                         _In_opt_  PVOID UserContext
);

BOOL WINAPI SymEnumSymbols(
                           _In_      HANDLE hProcess,
                           _In_      ULONG64 BaseOfDll,
                           _In_opt_  PCTSTR Mask,
                           _In_      PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,
                           _In_opt_  const PVOID UserContext
);

typedef BOOL (CALLBACK* PENUMDIRTREE_CALLBACK)(
                              _In_      PCTSTR FilePath,
                              _In_opt_  PVOID CallerData
);

BOOL WINAPI EnumDirTree(
                        _In_opt_   HANDLE hProcess,
                        _In_       PCTSTR RootPath,
                        _In_       PCTSTR InputPathName,
                        _Out_opt_  PTSTR OutputPathBuffer,
                        _In_opt_   PENUMDIRTREE_CALLBACK Callback,
                        _In_opt_   PVOID CallbackData
);




#ifdef __cplusplus
};
#endif

#endif