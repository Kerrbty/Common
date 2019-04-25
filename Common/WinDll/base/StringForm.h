#ifndef _FORMAT_LANGUAGE_STRING_H_
#define _FORMAT_LANGUAGE_STRING_H_
#include <windows.h>

// ���ֽ�ת���ֽڣ��ڴ���Ҫ�����ͷ�
PWSTR MulToWide( PSTR str );
PSTR WideToMul( PWSTR str );

// UTF8 ת խ�ֽ�
PSTR UTF8ToANSI( PSTR str , size_t size);

LPSTR FindLasteSymbolA(LPSTR CommandLine, CHAR FindWchar); // �����ַ����и����ַ������ֵ�λ��
LPWSTR FindLasteSymbolW(LPWSTR CommandLine, WCHAR FindWchar); // �����ַ����и����ַ������ֵ�λ��


#ifdef _UNICODE

#define FindLasteSymbol FindLasteSymbolW

#else

#define FindLasteSymbol FindLasteSymbolA

#endif


#endif  // _FORMAT_LANGUAGE_STRING_H_