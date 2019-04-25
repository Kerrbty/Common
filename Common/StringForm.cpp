#include "stdafx.h"
#include "StringForm.h"

// UTF8 转 窄字节
PSTR UTF8ToANSI( INOUT PSTR str , IN size_t size)
{
	WCHAR*  pElementText;
	int    iTextLen;
	// wide char to multi char
	iTextLen = MultiByteToWideChar( CP_UTF8,
		0,
		str,
		-1,
		NULL,
		0 );
	
	pElementText = new WCHAR[iTextLen + 1];
	ZeroMemory( pElementText, sizeof(WCHAR)*(iTextLen+1) );

	MultiByteToWideChar( CP_UTF8,
		0,
		str,
		-1,
		pElementText,
		iTextLen );
	
	WideCharToMultiByte( CP_ACP,
		0,
		pElementText,
		-1,
		(PCHAR)str,
		size,
		NULL,
		NULL );

	delete pElementText;
	return str;
}


// 多字节转宽字节，内存需要自行释放
PWSTR MulToWide( PSTR str )
{
	PWSTR  pElementText;
	int    iTextLen;

	iTextLen = MultiByteToWideChar( CP_ACP,
		0,
		(PCHAR)str,
		-1,
		NULL,
		0 );
	
	pElementText = 
		(PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (iTextLen+1)*sizeof(WCHAR));

	MultiByteToWideChar( CP_ACP,
		0,
		(PCHAR)str,
		-1,
		pElementText,
		iTextLen );

	return pElementText;
}

PSTR WideToMul( PWSTR str )
{
	PSTR  pElementText;
	int    iTextLen;
	
	iTextLen = WideCharToMultiByte( CP_ACP,
		0,
		str,
		-1,
		NULL,
		0,
		NULL,
		NULL);
	
	pElementText = 
		(PSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, iTextLen+1 );
	
	WideCharToMultiByte( CP_ACP,
		0,
		str,
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL);
	
	return pElementText;
}

//////////////////////////////////////////////////////////////////////////
void CMultiAndWide::SetString(PSTR szStr)
{
    if (szStr == NULL)
    {
        return ;
    }

    if (m_szBuf)
    {
        DelString(m_szBuf);
    }
    if (m_wzBuf)
    {
        DelString(m_wzBuf);
    }

    m_szBuf = 
		(PSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, strlen(szStr) +1 );
    strcpy(m_szBuf, szStr);
    m_wzBuf = MulToWide(m_szBuf);
}

void CMultiAndWide::SetString(PWSTR wzStr)
{
    if (wzStr == NULL)
    {
        return ;
    }

    if (m_szBuf)
    {
        DelString(m_szBuf);
    }
    if (m_wzBuf)
    {
        DelString(m_wzBuf);
    }
    
    m_wzBuf = 
        (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (wcslen(wzStr) +1)*sizeof(WCHAR));
    wcscpy(m_wzBuf, wzStr);
    m_szBuf = WideToMul(m_wzBuf);
}


// 查找字符串中给定字符最后出现的位置
LPSTR FindLasteSymbolA(LPSTR CommandLine, CHAR FindWchar)
{
	int Len;
	for ( Len = strlen(CommandLine) ; Len>0; Len-- )
	{
		if (CommandLine[Len] == FindWchar)
		{
			Len++;
			break;
		}
	}
	return &CommandLine[Len];
}

// 查找字符串中给定字符最后出现的位置
LPWSTR FindLasteSymbolW(LPWSTR CommandLine, WCHAR FindWchar)
{
	int Len;
	for ( Len = wcslen(CommandLine) ; Len>0; Len-- )
	{
		if (CommandLine[Len] == FindWchar)
		{
			Len++;
			break;
		}
	}
	return &CommandLine[Len];
}

