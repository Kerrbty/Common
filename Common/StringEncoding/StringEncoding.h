#ifndef _STRING_ENCODING_CHANGE_HEADER_H_
#define _STRING_ENCODING_CHANGE_HEADER_H_
#include <Windows.h>
enum EncodeType{
    ENCODE_UNKNOW = 0,
    ENCODE_UTF8 = 1,
    ENCODE_GBK  = 2,
};

// 判断文本是否为utf-8编码 
BOOL WINAPI IsTextUTF8(const char* str, ULONG length);

class CStringEncoding
{
public:
    CStringEncoding();
    CStringEncoding(PCSTR szStr, EncodeType type = ENCODE_UNKNOW);
    CStringEncoding(PCWSTR wszStr);

    // 设置新字符串 
    void SetString(PCSTR szStr, EncodeType type = ENCODE_UNKNOW);
    void SetString(PCWSTR wszStr);

    // 获取指定编码字符串 
    PCSTR GetUTF8();
    PCSTR GetGBK();
    PCWSTR GetUTF16();

    ~CStringEncoding();
protected:
    VOID CleanString();     // 清除现有字符串 
    VOID Init();

private:
    PSTR m_utf8;        // UTF8 编码 
    PSTR m_gbk;         // GBK 编码 
    PWSTR m_utf16;      // UTF16 编码 
};

#endif // _STRING_ENCODING_CHANGE_HEADER_H_ 