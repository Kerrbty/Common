#ifndef _FORMAT_LANGUAGE_STRING_H_
#define _FORMAT_LANGUAGE_STRING_H_
#include <windows.h>
#include <tchar.h>
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif

// ���ֽ�ת���ֽڣ��ڴ���Ҫ�����ͷ�
PWSTR MulToWide( IN PSTR str );
PSTR WideToMul( IN PWSTR str );
#define DelString(a)  { HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, a); a=NULL; }

class CMultiAndWide
{
public:
    CMultiAndWide()
    {
        m_szBuf = NULL;
        m_wzBuf = NULL;
    }

    CMultiAndWide(PSTR szStr)
    {
        SetString(szStr);
    }

    CMultiAndWide(PWSTR wzStr)
    {
        SetString(wzStr);
    }

    void SetString(PSTR szStr);
    void SetString(PWSTR wzStr);

    PSTR GetMultiByte()
    {
        return m_szBuf;
    }

    PWSTR GetWideChar()
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

LPSTR FindLasteSymbolA(LPSTR CommandLine, CHAR FindWchar); // �����ַ����и����ַ������ֵ�λ��
LPWSTR FindLasteSymbolW(LPWSTR CommandLine, WCHAR FindWchar); // �����ַ����и����ַ������ֵ�λ��

#ifdef _UNICODE

#define FindLasteSymbol FindLasteSymbolW

#else

#define FindLasteSymbol FindLasteSymbolA

#endif


#endif  // _FORMAT_LANGUAGE_STRING_H_