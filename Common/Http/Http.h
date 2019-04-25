#ifndef _CHTTP_ZXLY_2019_HEADER_HH_H
#define _CHTTP_ZXLY_2019_HEADER_HH_H
#include "../defs.h"
#include "AnalyzeURL.h"

#define USE_WINHTTP

#ifdef USE_WINHTTP 
// xp
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else // USE_WINHTTP 
// xp sp3
#include <WinInet.h>
#pragma comment(lib, "wininet.lib")
#endif // USE_WINHTTP 


// ���ش��͵�dwSize��С������������˳����ջ��� 
typedef unsigned long (WINAPI* PGetData)(unsigned char* lpBuf, unsigned long dwSize, void* userdata);


class CHttp
{
public:
    // ��ʼ�� 
    CHttp();
    CHttp(LPCSTR szUrl);
    CHttp(LPCWSTR szUrl);
    ~CHttp();

    // ���� URL��Ϣ 
    BOOL SetUrlA(LPCSTR szUrl);
    BOOL SetUrlW(LPCWSTR szUrl);

    // ����Cookie 
    BOOL SetCookieA(LPCSTR szCookie);
    BOOL SetCookieW(LPCWSTR szCookie);
    BOOL AddCookieA(LPCSTR szCookie);
    BOOL AddCookieW(LPCWSTR szCookie);

    void CleanCookie();

    // ������վ����ת֮ǰ����վ https://bbs.xxxxxx.com/ 
    BOOL SetRefererA(LPCSTR szRefererName);
    BOOL SetRefererW(LPCWSTR szRefererName);

    BOOL SetContentTypeA(LPCSTR szContentType);
    BOOL SetContentTypeW(LPCWSTR szContentType);

    // ���յ��ļ����� :  image/png, image/svg+xml, image/*;q=0.8, */*;q=0.5 
    BOOL SetAcceptA(LPCSTR szAcceptFile);
    BOOL SetAcceptW(LPCWSTR szAcceptFile);

    BOOL SetAcceptLanguageA(LPCSTR szLanguage);
    BOOL SetAcceptLanguageW(LPCWSTR szLanguage);

    // �Ƿ�ѹ������ : gzip, deflate 
    BOOL SetAcceptEncodingA(LPCSTR szEncoding);
    BOOL SetAcceptEncodingW(LPCWSTR szEncoding);

    // ʹ�õ�������������� : Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko 
    BOOL SetUserAgentA(LPCSTR szAgent);
    BOOL SetUserAgentW(LPCWSTR szAgent);

    BOOL SetRangeA(LPCSTR szRange);
    BOOL SetRangeW(LPCWSTR szRange);

    // ���ڷֶ�����ʱ�����ļ� 
    BOOL SetETagA(LPCSTR szTag);
    BOOL SetETagW(LPCWSTR szTag);

    // �ύ����ķ�ʽ GET/POST�� Ĭ��Ϊ GET 
    void SetPost(BOOL bPost = TRUE); // True Ϊ Post�� False Ϊ Get 

    void SetXmlHttpRequest(BOOL bXml = TRUE);

    BOOL SetPostData(LPBYTE lpBuf, DWORD dwSize);

    void SetGetDataFunc(PGetData func, void* userdata);
public:
    // ��ѯ������Ϣ 
    const char* QueryCookieA();
    const WCHAR* QueryCookieW();

    const char* QueryUrlA();
    const WCHAR* QueryUrlW();

    const char* QueryUserAgentA();
    const WCHAR* QueryUserAgentW();

    BOOL  IsPost();
public:
    unsigned long long LoopToGetData(); // ѭ����ȡ���ݣ�֪���û����ز�һ�»��߶�ȡ����������� 

    // �������ͷ������ 
    void CleanAllHeaderData();  // ���ȫ��ͷ����������, ������URL��Cookies 

    // ����������Ϣ,��������� 
    void Reset();   // ��������������ݣ�����Cookies 

protected:
    void Initialize(); // ��ʼ�����󣬹��캯���Ƚ϶࣬�Է��޸��Ժ�©��ĳ����ʼ�� 
    BOOL GetHttpRequestHeader();  // ����������Ϣ,����ȡͷ����Ϣ 
    void CloseRequest();  // ����ͷ��Ϣ 
    BOOL AddHeaderReqStr(LPWSTR RequestName, LPWSTR RequestValue);

public:
    const WCHAR* GetReturnCodeIdW();  // "200" "404" "500"
    const WCHAR* GetReturnTextIdW();  // "OK"
    const WCHAR* GetDataLenthW();     // �����������ݳ���(��ȥͷ��) 
    const WCHAR* GetSetCookieW();     // �����µ� Cookie WINHTTP_QUERY_SET_COOKIE/HTTP_QUERY_SET_COOKIE

    const char* GetReturnCodeIdA();  // "200" "404" "500"
    const char* GetReturnTextIdA();  // "OK"
    const char* GetDataLenthA();     // �����������ݳ���(��ȥͷ��) 
    const char* GetSetCookieA();  

    // WINHTTP_QUERY_ETAG   ->  ETag: "lr0ZzoawyyhskvuleG6PNUPXjzKs" 
    const char* GetETagA(); 
    const WCHAR* GetETagW();

private:
    CURL   m_url;
    BOOL   m_IsPost;   // ��POST��ʽ�������� 
    BOOL   m_SetXmlHttpRequest;
    BOOL   m_IsRequest; // �Ƿ��Ѿ���������� ? 

    LPBYTE m_PostData;
    DWORD  m_dwPostSize;

    LPWSTR m_AcceptLanguage;
    LPWSTR m_ContentType;
    LPWSTR m_cookies;
    LPWSTR m_Referer;
    LPWSTR m_Accept;
    LPWSTR m_AcceptEncoding;
    LPWSTR m_UserAgent;
    LPWSTR m_Tag;   // ͷ�� If-Range:  
    LPWSTR m_Range; // bytes=5001-10000 , ���ر߽� 
    
    HINTERNET m_hSession;  // ��Ҫ m_UserAgent ��ʼ�� 
    HINTERNET m_hConnect;
    HINTERNET m_hRequest;
    PGetData  m_func;
    void*  m_userdata;

    char*  m_szurl;
    char*  m_tmp_string;  // ���û����ص���ʱ���� 
    LPWSTR m_tmp_header;
    
};








#endif // _CHTTP_ZXLY_2019_HEADER_HH_H