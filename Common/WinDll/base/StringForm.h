#ifndef _FORMAT_LANGUAGE_STRING_H_
#define _FORMAT_LANGUAGE_STRING_H_
#include <windows.h>

// 多字节转宽字节，内存需要自行释放
PWSTR MulToWide( PSTR str );
PSTR WideToMul( PWSTR str );

// UTF8 转 窄字节
PSTR UTF8ToANSI( PSTR str , size_t size);

LPSTR FindLasteSymbolA(LPSTR CommandLine, CHAR FindWchar); // 查找字符串中给定字符最后出现的位置
LPWSTR FindLasteSymbolW(LPWSTR CommandLine, WCHAR FindWchar); // 查找字符串中给定字符最后出现的位置


#ifdef _UNICODE

#define FindLasteSymbol FindLasteSymbolW

#else

#define FindLasteSymbol FindLasteSymbolA

#endif


#endif  // _FORMAT_LANGUAGE_STRING_H_