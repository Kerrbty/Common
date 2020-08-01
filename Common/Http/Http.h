#ifndef _CHTTP_ZXLY_2019_HEADER_HH_H
#define _CHTTP_ZXLY_2019_HEADER_HH_H
#include <Windows.h>
#include "AnalyzeURL.h"

typedef void *HINTERNET;
// 返回传送的dwSize大小，如果不等则退出接收环节 
typedef unsigned long (WINAPI* PGetData)(unsigned char* lpBuf, unsigned long dwSize, void* userdata);

class CHttp
{
public:
    // 初始化 
    CHttp();
    CHttp(LPCSTR szUrl);
    CHttp(LPCWSTR szUrl);
    ~CHttp();

    // 设置 URL信息 
    BOOL SetUrlA(LPCSTR szUrl);
    BOOL SetUrlW(LPCWSTR szUrl);

    // 设置Cookie 
    BOOL SetCookieA(LPCSTR szCookie);
    BOOL SetCookieW(LPCWSTR szCookie);
    BOOL AddCookieA(LPCSTR szCookie);
    BOOL AddCookieW(LPCWSTR szCookie);

    void CleanCookie();  // 清除之前设置的Cookies 

    // 引用页，跳转之前的网站 Referer：https://bbs.xxxxxx.com/ 
    BOOL SetRefererA(LPCSTR szRefererName);
    BOOL SetRefererW(LPCWSTR szRefererName);

    BOOL SetContentTypeA(LPCSTR szContentType);
    BOOL SetContentTypeW(LPCWSTR szContentType);

    // 接收的文件类型 :  image/png, image/svg+xml, image/*;q=0.8, */*;q=0.5 
    BOOL SetAcceptA(LPCSTR szAcceptFile);
    BOOL SetAcceptW(LPCWSTR szAcceptFile);

    // 接收语言 Accept-Language: fr-CH, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5 
    BOOL SetAcceptLanguageA(LPCSTR szLanguage);
    BOOL SetAcceptLanguageW(LPCWSTR szLanguage);

    // 是否压缩传输 : Accept-Encoding: deflate, gzip;q=1.0, *;q=0.5 
    BOOL SetAcceptEncodingA(LPCSTR szEncoding);
    BOOL SetAcceptEncodingW(LPCWSTR szEncoding);

    // 使用的浏览器代理类型 : Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko 
    BOOL SetUserAgentA(LPCSTR szAgent);
    BOOL SetUserAgentW(LPCWSTR szAgent);

    // Range: bytes=5001-10000
    BOOL SetRangeA(LPCSTR szRange);
    BOOL SetRangeW(LPCWSTR szRange);

    // 用于分段下载时候标记文件 
    BOOL SetETagA(LPCSTR szTag);
    BOOL SetETagW(LPCWSTR szTag);

    BOOL SetIgnoreCert(BOOL bIgnore);  // 忽略HTTPS证书合法性 TRUE 表示忽略,返回原来设置 

    // 提交请求的方式 GET/POST， 默认为 GET 
    void SetPost(BOOL bPost = TRUE); // True 为 Post， False 为 Get 

    void SetAutoUnzip(BOOL bAutoUnzip = TRUE); // gzip数据自动解压,只有使用GetData一次性获取所有数据才有效 

    void SetXmlHttpRequest(BOOL bXml = TRUE);

    BOOL SetPostData(LPBYTE lpBuf, DWORD dwSize);

    void SetGetDataFunc(PGetData func, void* userdata);

public:
    // 查询设置信息 
    const char* QueryCookieA();
    const WCHAR* QueryCookieW();

    const char* QueryUrlA();
    const WCHAR* QueryUrlW();

    const char* QueryUserAgentA();
    const WCHAR* QueryUserAgentW();

    BOOL  IsPost();

public:
    unsigned long long LoopToGetData(); // 循环获取数据，知道用户返回不一致或者读取网络数据完毕 

    // 清空所有头部请求 
    void CleanAllHeaderData();  // 清除全部头部传输数据, 不包含URL、Cookies 

    // 重置所有信息,复用类对象 
    void Reset();   // 清除本来所有数据，包括Cookies 

protected:
    void Initialize(); // 初始化对象，构造函数比较多，以防修改以后漏掉某个初始化 
    BOOL GetHttpRequestHeader();  // 发送请求信息,并获取头部信息 
    void CloseRequest();  // 请求头信息 
    BOOL AddHeaderReqStr(LPWSTR RequestName, LPWSTR RequestValue);

public:
    const WCHAR* GetReturnCodeIdW();  // "200" "404" "500"
    const WCHAR* GetReturnTextIdW();  // "OK"
    const WCHAR* GetDataLenthW();     // 返回正文数据长度(除去头部) 
    const WCHAR* GetSetCookieW();     // 返回新的 Cookie WINHTTP_QUERY_SET_COOKIE/HTTP_QUERY_SET_COOKIE

    const char* GetReturnCodeIdA();  // "200" "404" "500"
    const char* GetReturnTextIdA();  // "OK"
    const char* GetDataLenthA();     // 返回正文数据长度(出去头部) 
    DWORD       GetData(LPBYTE lpBuf, DWORD dwLenth, BOOL AutoChgUtf8 = TRUE);  // 获取http服务器返回正文  
    const char* GetSetCookieA();  
    static BOOL UnzipData(LPBYTE lpInData, DWORD InLenth, LPBYTE lpOutData, DWORD OutLenth, BOOL AutoChgUtf8);  // gzip解压数据 

    // WINHTTP_QUERY_ETAG   ->  ETag: "lr0ZzoawyyhskvuleG6PNUPXjzKs" 
    const char* GetETagA(); 
    const WCHAR* GetETagW();

private:
    CURL   m_url;
    BOOL   m_IsPost;   // 是POST方式发送数据 
    BOOL   m_AutoUnzipData;  // 是否自动解压gzip数据 
    BOOL   m_SetXmlHttpRequest;
    BOOL   m_IsRequest; // 是否已经请求过数据 ?  

    LPBYTE m_PostData;
    DWORD  m_dwPostSize;

    BOOL   m_Ignore_Cert;
    LPWSTR m_AcceptLanguage;
    LPWSTR m_ContentType;
    LPWSTR m_cookies;
    LPWSTR m_Referer;
    LPWSTR m_Accept;
    LPWSTR m_AcceptEncoding;
    LPWSTR m_UserAgent;
    LPWSTR m_Tag;   // 头部 If-Range:  
    LPWSTR m_Range; // bytes=5001-10000 , 下载边界 
    
    HINTERNET m_hSession;  // 需要 m_UserAgent 初始化 
    HINTERNET m_hConnect;
    HINTERNET m_hRequest;
    PGetData  m_func;
    void*  m_userdata;

    char*  m_szurl;
    char*  m_tmp_string;  // 给用户返回的临时数据 
    LPWSTR m_tmp_header;
};

#endif // _CHTTP_ZXLY_2019_HEADER_HH_H