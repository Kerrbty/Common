#ifndef _STRING_ENCODING_CHANGE_HEADER_H_
#define _STRING_ENCODING_CHANGE_HEADER_H_
#include <Windows.h>
enum EncodeType{
    ENCODE_UNKNOW = 0,
    ENCODE_UTF8 = 1,
    ENCODE_GBK  = 2,
};

// �ж��ı��Ƿ�Ϊutf-8���� 
BOOL WINAPI IsTextUTF8(const char* str, ULONG length);

class CStringEncoding
{
public:
    CStringEncoding();
    CStringEncoding(PCSTR szStr, EncodeType type = ENCODE_UNKNOW);
    CStringEncoding(PCWSTR wszStr);

    // �������ַ��� 
    void SetString(PCSTR szStr, EncodeType type = ENCODE_UNKNOW);
    void SetString(PCWSTR wszStr);

    // ��ȡָ�������ַ��� 
    PCSTR GetUTF8();
    PCSTR GetGBK();
    PCWSTR GetUTF16();

    ~CStringEncoding();
protected:
    VOID CleanString();     // ��������ַ��� 
    VOID Init();

private:
    PSTR m_utf8;        // UTF8 ���� 
    PSTR m_gbk;         // GBK ���� 
    PWSTR m_utf16;      // UTF16 ���� 
};

#endif // _STRING_ENCODING_CHANGE_HEADER_H_ 