#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS
#ifdef _DEBUG
#include "Debug.h"
#include <stdio.h>
#include <windows.h>
#include <tchar.h>

void createcui( )
{
    if(AllocConsole())
    {
        freopen("CONOUT$","w", stdout); 
        freopen("CONOUT$","w", stderr); 
        freopen("CONIN$", "r", stdin); 
    }
}

// ASCII°æ 
void _dbgprintA(const char* lpszFormant, ...)
{
    char* szBuffer = NULL;
    size_t stSize = 0;

    va_list args;
    va_start(args, lpszFormant);

#if _MSC_VER > 1400
    stSize = _vscprintf(lpszFormant, args)+4;
#else
    stSize = 2048;
#endif
    szBuffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, stSize*sizeof(char));

    vsprintf(szBuffer, lpszFormant, args);
    va_end(args);

    OutputDebugStringA(szBuffer);
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, szBuffer);
}

// UNICODE°æ 
void _dbgprintW(const wchar_t* lpszFormant, ...)
{
    wchar_t* szBuffer = NULL;
    size_t stSize = 0;

    va_list args;
    va_start(args, lpszFormant);

#if _MSC_VER > 1400
    stSize = _vscwprintf(lpszFormant, args)+4;
#else
    stSize = 2048;
#endif
    szBuffer = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, stSize*sizeof(wchar_t));

    vswprintf(szBuffer, lpszFormant, args);
    va_end(args);

    OutputDebugStringW(szBuffer);
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, szBuffer);
}


int _dbgfprintA(FILE* stream, const char* lpszFormant, ...)
{
    char* szBuffer = NULL;
    size_t stSize = 0;

    va_list args;
    va_start(args, lpszFormant);

#if _MSC_VER > 1400
    stSize = _vscprintf(lpszFormant, args)+4;
#else
    stSize = 2048;
#endif
    szBuffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, stSize*sizeof(char));

    vsprintf(szBuffer, lpszFormant, args);
    va_end(args);

    stSize = fprintf(stream, "%s", szBuffer);
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, szBuffer);
    return stSize;
}


int _dbgfprintW(FILE* stream, const wchar_t* lpszFormant, ...)
{
    wchar_t* szBuffer = NULL;
    size_t stSize = 0;

    va_list args;
    va_start(args, lpszFormant);

#if _MSC_VER > 1400
    stSize = _vscwprintf(lpszFormant, args)+4;
#else
    stSize = 2048;
#endif
    szBuffer = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, stSize*sizeof(wchar_t));

    vswprintf(szBuffer, lpszFormant, args);
    va_end(args);

    stSize = fwprintf(stream, L"%s", szBuffer);
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, szBuffer);
    return stSize;
}













#endif  // _DEBUG