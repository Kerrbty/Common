#ifndef _FORMAT_LANGUAGE_STRING_H_
#define _FORMAT_LANGUAGE_STRING_H_
#include "../defs.h"

#define DelString(_a)  FreeMemory(_a)


// ���ֽ�ת���ֽڣ��ڴ���Ҫ�����ͷ�
PWSTR MulToWide( IN LPCSTR str );
PSTR WideToMul( IN LPCWSTR str );


class CMultiAndWide
{
public:
    CMultiAndWide()
    {
        m_szBuf = NULL;
        m_wzBuf = NULL;
    }

    CMultiAndWide(PCSTR szStr)
    {
        m_szBuf = NULL;
        m_wzBuf = NULL;
        SetString(szStr);
    }

    CMultiAndWide(PCWSTR wzStr)
    {
        m_szBuf = NULL;
        m_wzBuf = NULL;
        SetString(wzStr);
    }

    void SetString(PCSTR szStr);
    void SetString(PCWSTR wzStr);

    PCSTR GetMultiByte()
    {
        return m_szBuf;
    }

    PCWSTR GetWideChar()
    {
        return m_wzBuf;
    }

    ~CMultiAndWide()
    {
        if (m_szBuf)
        {
            DelString(m_szBuf);
        }
        if (m_wzBuf)
        {
            DelString(m_wzBuf);
        }
    }

private:
    PSTR m_szBuf;
    PWSTR m_wzBuf;
};


// UTF8 ת խ�ֽ�
PSTR UTF8ToANSI( INOUT PSTR str , IN size_t size);
PSTR ANSIToUTF8( INOUT PSTR str , IN size_t size);

LPSTR FindLasteSymbolA(LPSTR CommandLine, CHAR FindWchar); // �����ַ����и����ַ������ֵ�λ��
LPWSTR FindLasteSymbolW(LPWSTR CommandLine, WCHAR FindWchar); // �����ַ����и����ַ������ֵ�λ��


BOOL IsTextUTF8(const char* str, ULONG length);  // �ж��ı��Ƿ�Ϊutf-8���� 

VOID FreeString(LPVOID lpBuffer);

#ifdef _UNICODE
#define FindLasteSymbol FindLasteSymbolW
#else
#define FindLasteSymbol FindLasteSymbolA
#endif

#ifndef _DEBUG
#pragma comment(lib, "StringFormat")
#else
#pragma comment(lib, "StringFormat_d")
#endif

#endif  // _FORMAT_LANGUAGE_STRING_H_