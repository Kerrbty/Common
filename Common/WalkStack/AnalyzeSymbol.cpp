#include "AnalyzeSymbol.h"
#include "dbghelp.h"
#include "StringForm.h"

#pragma comment(lib, "Imagehlp.lib")
LPTSTR GetFuncNameByDllName(  LPSTR DllName, 
                            DWORD Address, 
                            LPTSTR FuncName, 
                            DWORD lenth, 
                            DWORD* offset, 
                            HANDLE hprocess )
{
    SymSetOptions(SYMOPT_CASE_INSENSITIVE|SYMOPT_DEFERRED_LOADS|SYMOPT_UNDNAME);
    
    SymInitialize(	hprocess, 
        TEXT("srv*http://msdl.microsoft.com/download/symbols;D:\\symbols"), 
				    FALSE);
    PLOADED_IMAGE pLoadedImage = ImageLoad(DllName, NULL);
    
    DWORD SymModuleBase = SymLoadModule( hprocess, 
        pLoadedImage->hFile, 
        pLoadedImage->ModuleName, 
        DllName, 
        (DWORD)GetModuleHandle(DllName), 
        pLoadedImage->SizeOfImage);
    
    PIMAGEHLP_SYMBOL pImageHelpSymbol = (PIMAGEHLP_SYMBOL)HeapAlloc(GetProcessHeap(), 
        HEAP_ZERO_MEMORY, 
        sizeof(pImageHelpSymbol)+sizeof(TCHAR)*MAX_PATH);
    
    pImageHelpSymbol->SizeOfStruct = sizeof(pImageHelpSymbol)+sizeof(TCHAR)*MAX_PATH;
    pImageHelpSymbol->MaxNameLength = MAX_PATH;
    if( SymGetSymFromAddr(hprocess, Address, offset, pImageHelpSymbol) == TRUE)
    {
        _tcsncpy(FuncName, pImageHelpSymbol->Name, lenth);
    }
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pImageHelpSymbol);
    
    SymUnloadModule(hprocess, SymModuleBase);
    ImageUnload(pLoadedImage);
    
    return FuncName;
}

LPTSTR GetFuncName(  DWORD Address, 
                   LPTSTR FuncName, 
                   DWORD lenth, 
                   DWORD* offset)
{
    HMODULE hmodule;
    
    ZeroMemory(FuncName, sizeof(TCHAR)*lenth);
    PTSTR FileName = (PTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TCHAR)*MAX_PATH);
    if ( GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (PCHAR)Address, &hmodule) == TRUE && 
        hmodule != NULL)
    {
        GetModuleFileName(hmodule, FileName, MAX_PATH);
        GetFuncNameByDllName(FindLasteSymbol(FileName, TEXT('\\')), Address, FuncName, lenth, offset);
    }
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, FileName);
    return FuncName;
}

DWORD GetFuncAddress(LPTSTR DllName, LPTSTR FuncName, HANDLE hprocess )
{
    DWORD FuncAddr = 0;
    
    SymSetOptions(SYMOPT_CASE_INSENSITIVE|SYMOPT_DEFERRED_LOADS|SYMOPT_UNDNAME);
    //   HANDLE hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
    SymInitialize(	hprocess, 
        TEXT("srv*http://msdl.microsoft.com/download/symbols;D:\\symbols"), 
				    FALSE);
    PLOADED_IMAGE pLoadedImage = ImageLoad(DllName, NULL);
    
    DWORD SymModuleBase = SymLoadModule( hprocess, 
        pLoadedImage->hFile, 
        pLoadedImage->ModuleName, 
        DllName, 
        (DWORD)GetModuleHandle(DllName), 
        pLoadedImage->SizeOfImage);
    
    PIMAGEHLP_SYMBOL pImageHelpSymbol = (PIMAGEHLP_SYMBOL)HeapAlloc(GetProcessHeap(), 
        HEAP_ZERO_MEMORY, 
        sizeof(pImageHelpSymbol)+sizeof(TCHAR)*MAX_PATH);
    pImageHelpSymbol->SizeOfStruct = sizeof(pImageHelpSymbol)+sizeof(TCHAR)*MAX_PATH;
    if (SymGetSymFromName(hprocess, FuncName, pImageHelpSymbol) == TRUE)
    {
        FuncAddr = pImageHelpSymbol->Address;
    }
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pImageHelpSymbol);
    
    SymUnloadModule(hprocess, SymModuleBase);
    ImageUnload(pLoadedImage);
    
    return FuncAddr;
}




/*

#include <windows.h>
#include "DbgHelp.h"
#include <Imagehlp.h>
#pragma comment(lib, "Imagehlp.lib")
#include "dbghelp.h"
#include <stdio.h>


// 枚举所有导出函数
BOOL CALLBACK CallBackProc( PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext )
{
    printf( "函数名: %s\r\n地址: %08X \r\n\r\n", pSymInfo->Name, pSymInfo->Address );
    return TRUE;
}

char* UnicodeToAnsi( const wchar_t* szStr, char* szDest )
{
    int nLen = WideCharToMultiByte( CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL );
    if ( nLen == 0 )
    {
        return NULL;
    }
    char* pResult = new char[nLen];
    WideCharToMultiByte( CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL );
    strcpy( szDest, pResult );
    delete pResult;
    return szDest;
}

BOOL GetSymbol( LPTSTR FileName )
{
    HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId() );
    CloseHandle( hProcess );
    
    if ( !SymInitialize( hProcess, NULL, FALSE ) )
    {
        return FALSE;
    }
    
    DWORD dwOpt = SymGetOptions();
    SymSetOptions( dwOpt | SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_CASE_INSENSITIVE );
    
//    char sFileName[MAX_PATH] = {0};
//    UnicodeToAnsi( FileName, sFileName );
    DWORD64 dwSymModule = SymLoadModuleEx( hProcess, NULL, FileName, NULL, 0, 0, NULL, 0 );
    
    if ( 0 == dwSymModule )
    {
        SymCleanup( hProcess );
        return -1;
    }
    
    if ( !SymEnumSymbols( hProcess, dwSymModule, 0, ( PSYM_ENUMERATESYMBOLS_CALLBACK )CallBackProc, NULL ) )
    {
        SymCleanup( hProcess );
        return -1;
    }
    
    return SymCleanup( hProcess );
}

int _tmain( int argc, _TCHAR* argv[] )
{
    TCHAR* sDllPath = _T( "C:\\Windows\\System32\\WS2_32.DLL" );
    if ( !GetSymbol( sDllPath ) )
    {
        return -1;
    }
    getchar();
    return 0;
}

*/