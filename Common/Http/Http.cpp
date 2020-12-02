#include "Config.h"
#include <defs.h>
#include "Http.h"
#include "AnalyzeURL.h"
#include "../StringFormat/StringFormat.h"
#include <stdlib.h>
#include <stdio.h>
#include <zlib-1.2.11/zlib.h>

#ifdef _DEBUG
#pragma comment(lib, "zlib_d.lib")
#pragma comment(lib, "StringFormat_d.lib")
#else 
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "StringFormat.lib")
#endif

#pragma warning(disable : 4996)

//////////////////////////////////////////////////////////////////////////
// ����ͷ
// /*
// Accept              ָ���ͻ����ܹ����յ���������                                    Accept: text/plain, text/html
// Accept-Charset      ��������Խ��ܵ��ַ����뼯��                                    Accept-Charset: iso-8859-5
// Accept-Encoding     ָ�����������֧�ֵ�web��������������ѹ���������͡�             Accept-Encoding: compress, gzip
// Accept-Language     ������ɽ��ܵ�����                                              Accept-Language: en,zh
// Accept-Ranges       ����������ҳʵ���һ�����߶���ӷ�Χ�ֶ�                        Accept-Ranges: bytes
// Authorization       HTTP��Ȩ����Ȩ֤��                                              Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==
// Cache-Control       ָ���������Ӧ��ѭ�Ļ������                                    Cache-Control: no-cache
// Connection          ��ʾ�Ƿ���Ҫ�־����ӡ���HTTP 1.1Ĭ�Ͻ��г־����ӣ�              Connection: close
// Cookie    HTTP      ������ʱ����ѱ����ڸ����������µ�����cookieֵһ���͸�web��������    Cookie: $Version=1; Skin=new;
// Content-Length      ��������ݳ���                                                  Content-Length: 348
// Content-Type        �������ʵ���Ӧ��MIME��Ϣ                                      Content-Type: application/x-www-form-urlencoded
// Date                �����͵����ں�ʱ��                                            Date: Tue, 15 Nov 2010 08:12:31 GMT
// Expect              ������ض��ķ�������Ϊ                                          Expect: 100-continue
// From                ����������û���Email                                           From: user@email.com
// Host                ָ������ķ������������Ͷ˿ں�                                  Host: www.zcmhi.com
// If-Match            ֻ������������ʵ����ƥ�����Ч                                  If-Match: ��737060cd8c284d8af7ad3082f209582d��
// If-Modified-Since   �������Ĳ�����ָ��ʱ��֮���޸�������ɹ���δ���޸��򷵻�304����    If-Modified-Since: Sat, 29 Oct 2010 19:43:31 GMT
// If-None-Match       �������δ�ı䷵��304���룬����Ϊ��������ǰ���͵�Etag�����������Ӧ��Etag�Ƚ��ж��Ƿ�ı�    If-None-Match: ��737060cd8c284d8af7ad3082f209582d��
// If-Range            ���ʵ��δ�ı䣬���������Ϳͻ��˶�ʧ�Ĳ��֣�����������ʵ�塣����ҲΪEtag    If-Range: ��737060cd8c284d8af7ad3082f209582d��
// If-Unmodified-Since ֻ��ʵ����ָ��ʱ��֮��δ���޸Ĳ�����ɹ�                        If-Unmodified-Since: Sat, 29 Oct 2010 19:43:31 GMT
// Max-Forwards        ������Ϣͨ����������ش��͵�ʱ��                                Max-Forwards: 10
// Pragma              ��������ʵ���ض���ָ��                                          Pragma: no-cache
// Proxy-Authorization ���ӵ��������Ȩ֤��                                            Proxy-Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==
// Range               ֻ����ʵ���һ���֣�ָ����Χ                                    Range: bytes=500-999
// Referer             ��ǰ��ҳ�ĵ�ַ����ǰ������ҳ�������,����·                     Referer: http://www.zcmhi.com/archives/71.html
// TE                  �ͻ���Ը����ܵĴ�����룬��֪ͨ���������ܽ���β��ͷ��Ϣ        TE: trailers,deflate;q=0.5
// Upgrade             �������ָ��ĳ�ִ���Э���Ա����������ת�������֧�֣�          Upgrade: HTTP/2.0, SHTTP/1.3, IRC/6.9, RTA/x11
// User-Agent          User-Agent�����ݰ�������������û���Ϣ                          User-Agent: Mozilla/5.0 (Linux; X11)
// Via                 ֪ͨ�м����ػ�����������ַ��ͨ��Э��                          Via: 1.0 fred, 1.1 nowhere.com (Apache/1.1)
// Warning             ������Ϣʵ��ľ�����Ϣ                                          Warn: 199 Miscellaneous warning
// */ 

//////////////////////////////////////////////////////////////////////////
// ���� 
// /*
// Accept-Ranges       �����������Ƿ�֧��ָ����Χ�����������͵ķֶ�����              Accept-Ranges: bytes
// Age                 ��ԭʼ���������������γɵĹ���ʱ�䣨����ƣ��Ǹ���            Age: 12
// Allow               ��ĳ������Դ����Ч��������Ϊ���������򷵻�405                   Allow: GET, HEAD
// Cache-Control       �������еĻ�������Ƿ���Ի��漰��������                        Cache-Control: no-cache
// Content-Encoding    web������֧�ֵķ�������ѹ���������͡�                           Content-Encoding: gzip
// Content-Language    ��Ӧ�������                                                    Content-Language: en,zh
// Content-Length      ��Ӧ��ĳ���                                                    Content-Length: 348
// Content-Location    ������Դ������ı��õ���һ��ַ                                  Content-Location: /index.htm
// Content-MD5         ������Դ��MD5У��ֵ                                             Content-MD5: Q2hlY2sgSW50ZWdyaXR5IQ==
// Content-Range       �������������б����ֵ��ֽ�λ��                                  Content-Range: bytes 21010-47021/47022
// Content-Type        �������ݵ�MIME����                                              Content-Type: text/html; charset=utf-8
// Date                ԭʼ��������Ϣ������ʱ��                                        Date: Tue, 15 Nov 2010 08:12:31 GMT
// ETag                ���������ʵ���ǩ�ĵ�ǰֵ                                      ETag: "737060cd8c284d8af7ad3082f209582d"
// Expires             ��Ӧ���ڵ����ں�ʱ��                                            Expires: Thu, 01 Dec 2010 16:00:00 GMT
// Last-Modified       ������Դ������޸�ʱ��                                          Last-Modified: Tue, 15 Nov 2010 12:45:26 GMT
// Location            �����ض�����շ���������URL��λ�������������ʶ�µ���Դ       Location: http://www.zcmhi.com/archives/94.html
// Pragma              ����ʵ���ض���ָ�����Ӧ�õ���Ӧ���ϵ��κν��շ�              Pragma: no-cache
// Proxy-Authenticate  ��ָ����֤�����Ϳ�Ӧ�õ�����ĸ�URL�ϵĲ���                     Proxy-Authenticate: Basic
// refresh             Ӧ�����ض����һ���µ���Դ�����죬��5��֮���ض�����������������󲿷������֧�֣�    Refresh: 5; url=http://www.zcmhi.com/archives/94.html
// Retry-After         ���ʵ����ʱ����ȡ��֪ͨ�ͻ�����ָ��ʱ��֮���ٴγ���            Retry-After: 120
// Server              web�������������                                               Server: Apache/1.3.27 (Unix) (Red-Hat/Linux)
// Set-Cookie          ����Http Cookie                                                 Set-Cookie: UserID=JohnDoe; Max-Age=3600; Version=1
// Trailer             ָ��ͷ���ڷֿ鴫������β������                                Trailer: Max-Forwards
// Transfer-Encoding   �ļ��������                                                    Transfer-Encoding:chunked
// Vary                �������δ�����ʹ�û�����Ӧ���Ǵ�ԭʼ����������                  Vary: *
// Via                 ��֪����ͻ�����Ӧ��ͨ�����﷢�͵�                              Via: 1.0 fred, 1.1 nowhere.com (Apache/1.1)
// Warning             ����ʵ����ܴ��ڵ�����                                          Warning: 199 Miscellaneous warning
// WWW-Authenticate    �����ͻ�������ʵ��Ӧ��ʹ�õ���Ȩ����                            WWW-Authenticate: Basic
// */

HINTERNET OpenSession(LPCWSTR UserAgent)
{
#ifdef USE_WINHTTP
    return WinHttpOpen( UserAgent,
                        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                        WINHTTP_NO_PROXY_NAME,
                        WINHTTP_NO_PROXY_BYPASS,
                        0);
#else
    return InternetOpenW(UserAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
#endif
}


HINTERNET Connect(HINTERNET hSession, LPCWSTR szServerAddr, int portNo)
{
#ifdef USE_WINHTTP
    return WinHttpConnect(hSession, szServerAddr, (INTERNET_PORT)portNo, 0);
#else
    return InternetConnectW(hSession, szServerAddr, (INTERNET_PORT)portNo, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
#endif
}


HINTERNET OpenRequest(HINTERNET hConnect, LPCWSTR verb, LPCWSTR objcetName, int scheme)
{
    DWORD flags = 0;
#ifdef USE_WINHTTP
    if (scheme == INTERNET_SCHEME_HTTPS)
    {
        flags |= WINHTTP_FLAG_SECURE;
    }
    return WinHttpOpenRequest(hConnect, verb, objcetName, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
#else
    if (scheme == INTERNET_SCHEME_HTTPS)
    {
        flags |= INTERNET_FLAG_SECURE;
    }
    return HttpOpenRequestW(hConnect, verb, objcetName, L"HTTP/1.1", NULL, NULL, flags, 0);
#endif
}


BOOL AddRequestHeaders(HINTERNET hRequest, LPCWSTR header)
{
    DWORD len = wcslen(header);
#ifdef USE_WINHTTP
    return WinHttpAddRequestHeaders(hRequest, header, len, WINHTTP_ADDREQ_FLAG_ADD|WINHTTP_ADDREQ_FLAG_REPLACE);
#else
    return HttpAddRequestHeadersW(hRequest, header, len, HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
#endif
}

// �����Զ�̷�������ssl֡���Ƿ���� 
BOOL SetRequestIgnoreCert(HINTERNET hRequest)
{
    DWORD dwFlags;
    DWORD dwBuffLen = sizeof(dwFlags);

#ifdef USE_WINHTTP        
    WinHttpQueryOption (hRequest, WINHTTP_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, &dwBuffLen);
    dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
    dwFlags |= SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
    dwFlags |= SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
    // dwFlags |= SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;

    return WinHttpSetOption (hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof (dwFlags) );
#else
    InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, &dwBuffLen);
    dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
    dwFlags |= SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
    dwFlags |= SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
    // dwFlags |= SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;

    return InternetSetOption (hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
#endif
}

BOOL SetDisableCertRedirects(HINTERNET hRequest)
{
    DWORD dwFlags = WINHTTP_DISABLE_REDIRECTS;
    DWORD dwBuffLen = sizeof(dwFlags);

#ifdef USE_WINHTTP        
    WinHttpQueryOption (hRequest, WINHTTP_OPTION_DISABLE_FEATURE, (LPVOID)&dwFlags, &dwBuffLen);
    dwFlags |= WINHTTP_DISABLE_REDIRECTS;

    return WinHttpSetOption(hRequest, WINHTTP_OPTION_DISABLE_FEATURE, &dwFlags, sizeof (dwFlags) );
#else
    return FALSE;
#endif  
}

BOOL SendRequest(HINTERNET hRequest, const void* body, DWORD size)
{
#ifdef USE_WINHTTP
    return WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)body, size, size, 0);
#else
    return HttpSendRequest(hRequest, NULL, NULL, (LPVOID)body, size);
#endif
}


BOOL EndRequest(HINTERNET hRequest)
{
#ifdef USE_WINHTTP
    return WinHttpReceiveResponse(hRequest, 0);
#else
    return TRUE;
#endif
}


BOOL QueryInfo(HINTERNET hRequest, DWORD queryId, LPWSTR szBuf, DWORD* cbSize)
{
#ifdef USE_WINHTTP
    return WinHttpQueryHeaders(hRequest, queryId, 0, szBuf, cbSize, 0);
#else
    return HttpQueryInfoW(hRequest, queryId, szBuf, cbSize, 0);
#endif
}

BOOL ReadData(HINTERNET hRequest, LPVOID buf, DWORD lenth, DWORD* cbRead, BOOL AutoChgUtf8 = FALSE)
{
#ifdef USE_WINHTTP
    if( WinHttpReadData(hRequest, buf, lenth, cbRead) )
    {
        if ( IsTextUTF8((LPSTR)buf, *cbRead) )
        {
            UTF8ToANSI((LPSTR)buf, *cbRead);
        }
        return TRUE;
    }

#else
    if( InternetReadFile(hRequest, buf, lenth, cbRead) )
    {
        if ( AutoChgUtf8 && IsTextUTF8((LPSTR)buf, *cbRead) )
        {
            UTF8ToANSI((LPSTR)buf, *cbRead);
        }
        return TRUE;
    }
#endif
    return FALSE;
}


VOID CloseInternetHandle(HINTERNET hInternet)
{
    if (hInternet)
    {
#ifdef USE_WINHTTP
        WinHttpCloseHandle(hInternet);
#else
        InternetCloseHandle(hInternet);
#endif
    }
}



//////////////////////////////////////////////////////////////////////////
// Class
//////////////////////////////////////////////////////////////////////////

// ���ò��� 
#define SetFuncBody(_name, _value)  BOOL CHttp::_name##A(LPCSTR szTmp) \
{\
    BOOL retval = FALSE; \
    if(szTmp != NULL) {\
        LPWSTR szwTmp = MulToWide(szTmp); \
        retval = _name##W(szwTmp); \
        FreeString(szwTmp); \
    } \
    return retval; \
} \
    BOOL CHttp::_name##W(LPCWSTR szwTmp) \
{\
    if (szwTmp == NULL) return FALSE; \
    if (_value) FreeMemory(_value); {\
        _value = (LPWSTR)AllocMemory((wcslen(szwTmp)+1)*sizeof(WCHAR)); \
        wcscpy(_value, szwTmp); \
    } \
    if (_value) return TRUE; \
    return FALSE; \
}

void CHttp::Initialize()
{
    m_IsPost = FALSE;
    m_AutoUnzipData = FALSE;
    m_IsRequest = FALSE;
    m_SetXmlHttpRequest = FALSE;

    m_PostData = NULL;
    m_dwPostSize = 0;

    m_Ignore_Cert = FALSE;
    m_Disable_Receive = FALSE;
    m_cookies = NULL;
    m_Referer = NULL;
    m_Accept = FALSE;
    m_AcceptEncoding = NULL;
    m_UserAgent = NULL;
    m_ContentType = NULL;
    m_AcceptLanguage = NULL;
    m_Tag = NULL;
    m_Range = NULL;

    m_hSession = NULL;
    m_hConnect = NULL;
    m_hRequest = NULL;

    m_func = NULL;
    m_userdata = NULL;

    m_tmp_string = NULL;
    m_szurl = NULL;
    m_tmp_header = NULL;
}

CHttp::CHttp()
{
    Initialize();
}

CHttp::CHttp(LPCSTR szUrl)
{
    Initialize();
    m_url.SetUrlA(szUrl);
    m_szurl = (char*)AllocMemory(strlen(szUrl)+1);
    strcpy(m_szurl, szUrl);
}

CHttp::CHttp(LPCWSTR szUrl)
{
    Initialize();
    m_url.SetUrlW(szUrl);
    m_szurl = WideToMul(szUrl);
}

void CHttp::CloseRequest()
{
    if (m_hRequest)
    {
        CloseInternetHandle(m_hRequest);
        m_hRequest = NULL;
    }
    if (m_hConnect)
    {
        CloseInternetHandle(m_hConnect);
        m_hConnect = NULL;
    }
    if (m_hSession)
    {
        CloseInternetHandle(m_hSession);
        m_hSession = NULL;
    }
}

// ���� URL��Ϣ 
BOOL CHttp::SetUrlA(LPCSTR szUrl)
{
    if (szUrl == NULL)
    {
        return FALSE;
    }

    // ����Ѿ��������վ�ˣ����ر���������, �����֮ǰ���� 
    if (m_IsRequest)
    {
        CloseRequest();
        m_IsRequest = FALSE;
    }
    m_url.SetUrlA(szUrl);
    return TRUE;
}


BOOL CHttp::SetUrlW(LPCWSTR szUrl)
{
    if (szUrl == NULL)
    {
        return FALSE;
    }

    if (m_IsRequest)
    {
        CloseRequest();
        m_IsRequest = FALSE;
    }
    m_url.SetUrlW(szUrl);
    return TRUE;
}

// ����Cookie 
BOOL CHttp::SetCookieA(LPCSTR szCookie)
{
    if (szCookie == NULL)
    {
        return FALSE;
    }
    CleanCookie();

    m_cookies = MulToWide(szCookie);
    if (m_cookies)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CHttp::SetCookieW(LPCWSTR szCookie)
{
    if (szCookie == NULL)
    {
        return FALSE;
    }
    CleanCookie();
    m_cookies = (LPWSTR)AllocMemory((wcslen(szCookie)+8)*sizeof(WCHAR));
    if (m_cookies == NULL)
    {
        return FALSE;
    }
    wcscpy(m_cookies, szCookie);
    return TRUE;
}

void CHttp::CleanCookie()
{
    if (m_cookies != NULL)
    {
        FreeMemory(m_cookies);
        m_cookies = NULL;
    }
}

BOOL CHttp::AddCookieA(LPCSTR szCookie)
{
    BOOL retval = FALSE;
    if (szCookie != NULL)
    {
        LPWSTR szwCookie = MulToWide(szCookie);
        retval = AddCookieW(szwCookie);
        FreeString(szwCookie);
    }
    return retval;
}

BOOL CHttp::AddCookieW(LPCWSTR szCookie)
{
    BOOL retval = FALSE;
    if (szCookie == NULL)
    {
        return retval;
    }

    // �Ƿ�Ϊ�� 
    if (m_cookies == NULL)
    {
        m_cookies = (LPWSTR)AllocMemory((wcslen(szCookie)+8)*sizeof(WCHAR));
        if (m_cookies)
        {
            wcscpy(m_cookies, szCookie);
            retval = TRUE;
        }
    }
    else
    {
        size_t ilen = wcslen(m_cookies);
        m_cookies = (LPWSTR)ReAllocMemory(m_cookies, (ilen+wcslen(szCookie)+8)*sizeof(WCHAR));
        if (m_cookies)
        {
            wcscat(m_cookies, L"; ");
            wcscat(m_cookies, szCookie);
            retval = TRUE;
        }
    }
    return retval;
}

SetFuncBody(SetETag, m_Tag);
SetFuncBody(SetRange, m_Range);
SetFuncBody(SetAccept, m_Accept);   // ���յ��ļ����� :  image/png, image/svg+xml, image/*;q=0.8, */*;q=0.5 
SetFuncBody(SetReferer, m_Referer);  // ������վ����ת֮ǰ����վ https://bbs.xxxxxx.com/ 
SetFuncBody(SetUserAgent, m_UserAgent);  // ʹ�õ�������������� : Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko 
SetFuncBody(SetContentType, m_ContentType);
SetFuncBody(SetAcceptEncoding, m_AcceptEncoding); // �Ƿ�ѹ������ : gzip, deflate 
SetFuncBody(SetAcceptLanguage, m_AcceptLanguage);


BOOL CHttp::SetIgnoreCert(BOOL bIgnore)
{
    BOOL bset = m_Ignore_Cert;
    m_Ignore_Cert = bIgnore;
    return bset;
}

BOOL CHttp::SetDisableReceive(BOOL bDisable)
{
    BOOL bset = m_Disable_Receive;
    m_Disable_Receive = bDisable;
    return bset;
}

// �ύ����ķ�ʽ GET/POST�� Ĭ��Ϊ GET 
void CHttp::SetPost(BOOL bPost) // True Ϊ Post�� False Ϊ Get 
{
    m_IsPost = bPost;
}

void CHttp::SetAutoUnzip(BOOL bAutoUnzip)
{
    m_AutoUnzipData = bAutoUnzip;
}

void CHttp::SetXmlHttpRequest(BOOL bXml)
{
    m_SetXmlHttpRequest = bXml;
}

BOOL CHttp::SetPostData(LPBYTE lpBuf, DWORD dwSize)
{
    if (lpBuf == NULL || dwSize == 0)
    {
        return FALSE;
    }
    FreeMemory(m_PostData);
    m_PostData = (LPBYTE)AllocMemory(dwSize+1);
    memcpy(m_PostData, lpBuf, dwSize);
    m_dwPostSize = dwSize;
    return TRUE;
}

void CHttp::SetGetDataFunc(PGetData func, void* userdata)
{
    m_func = func;
    m_userdata = userdata;
}

const char* CHttp::QueryCookieA()
{
    if (m_tmp_string)
    {
        FreeString(m_tmp_string);
    }
    if (m_cookies)
    {
        m_tmp_string = WideToMul(m_cookies);
    }
    return NULL;
}

const WCHAR* CHttp::QueryCookieW()
{
    return m_cookies;
}

const char* CHttp::QueryUrlA()
{
    return m_szurl;
}

const WCHAR* CHttp::QueryUrlW()
{
    if (m_szurl)
    {
        if (m_tmp_header)
        {
            FreeString(m_tmp_header);
        }
        m_tmp_header = MulToWide(m_szurl);
        return m_tmp_header;
    }
    return NULL;
}


const char* CHttp::QueryUserAgentA()
{
    if (m_tmp_string)
    {
        FreeString(m_tmp_string);
    }
    if (m_UserAgent)
    {
        m_tmp_string = WideToMul(m_UserAgent);
    }
    return NULL;
}

const WCHAR* CHttp::QueryUserAgentW()
{
    return m_UserAgent;
}

BOOL  CHttp::IsPost()
{
    return m_IsPost;
}

// ���ȫ��ͷ���������� , ������URL��Cookies 
void CHttp::CleanAllHeaderData()
{
    m_IsPost = FALSE;
    m_IsRequest = FALSE;
    m_dwPostSize = 0;
    FreeMemory(m_PostData);
    m_PostData = NULL;
    FreeMemory(m_Referer);
    m_Referer = NULL;
    FreeMemory(m_Accept);
    m_Accept = NULL;
    FreeMemory(m_AcceptEncoding);
    m_AcceptEncoding = NULL;
    FreeMemory(m_UserAgent);
    m_UserAgent = NULL;
    FreeMemory(m_ContentType);
    m_ContentType = NULL;
    FreeMemory(m_AcceptLanguage);
    m_AcceptLanguage = NULL;
    FreeMemory(m_Range);
    m_Range = NULL;

    FreeMemory(m_tmp_header);
    m_tmp_header = NULL;
    FreeMemory(m_tmp_string);
    m_tmp_string = NULL;
    CloseRequest();
}


void CHttp::Reset()
{
    CleanAllHeaderData();

    // �´����󻹿��Լ����õ����ݣ����ͬһ����վ 
    FreeMemory(m_cookies);
    m_cookies = NULL;
    FreeMemory(m_szurl);
    m_szurl = NULL;
    FreeMemory(m_Tag);
    m_Tag = NULL;
}


BOOL CHttp::AddHeaderReqStr(LPWSTR RequestName, LPWSTR RequestValue)
{
    BOOL addsuccess = TRUE;
    if (RequestName != NULL && RequestValue != NULL)
    {
        LPWSTR lpRequestString = (LPWSTR)AllocMemory((wcslen(RequestValue)+wcslen(RequestName)+MAX_PATH)*sizeof(WCHAR));
        wsprintfW(lpRequestString, L"%s: %s\r\n", RequestName, RequestValue);
        addsuccess = AddRequestHeaders(m_hRequest, lpRequestString);
        FreeMemory(lpRequestString);
    }
    else if (RequestName != NULL)
    {
        addsuccess = AddRequestHeaders(m_hRequest, RequestName);
    }
    else if (RequestValue != NULL)
    {
        addsuccess = AddRequestHeaders(m_hRequest, RequestValue);
    }
    return addsuccess;
}

BOOL CHttp::GetHttpRequestHeader() 
{
    BOOL iSuccess = FALSE;

#ifdef USE_WINHTTP
    int contextLengthId = WINHTTP_QUERY_CONTENT_LENGTH;
    int statusCodeId = WINHTTP_QUERY_STATUS_CODE;
    int statusTextId = WINHTTP_QUERY_STATUS_TEXT;
#else
    int contextLengthId = HTTP_QUERY_CONTENT_LENGTH;
    int statusCodeId = HTTP_QUERY_STATUS_CODE;
    int statusTextId = HTTP_QUERY_STATUS_TEXT;
#endif

    // Open session.
    if (m_UserAgent == NULL)
    {
        m_hSession = OpenSession(L"Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36");
    }
    else
    {
        m_hSession = OpenSession(m_UserAgent);
    }
    if (m_hSession == NULL)
    {
        return FALSE;
    }

    // test-tool.xianshuabao.com
    m_hConnect = Connect(m_hSession, m_url.GetServerName(), m_url.GetPort());
    if (m_hConnect == NULL)
    {
        return FALSE;
    }

    // �����ύ��ʽ 
    // Open request.
    // api/ad/getimages
    if (m_IsPost)
    {
        m_hRequest = OpenRequest(m_hConnect, L"POST", m_url.GetObjectName(), m_url.GetScheme() );
    }
    else
    {
        m_hRequest = OpenRequest(m_hConnect, L"GET", m_url.GetObjectName(), m_url.GetScheme() );
    }
    if (m_hRequest == NULL)
    {
        return FALSE;
    }

    // �����ļ�����  
    if (m_Accept)
    {
        AddHeaderReqStr(L"Accept", m_Accept);
    }

    // ����ҳ�� 
    if (m_Referer)
    {
        AddHeaderReqStr(L"Referer", m_Referer);
    }

    // ���ձ���  
    if (m_AcceptEncoding)
    {
        AddHeaderReqStr(L"Accept-Encoding", m_AcceptEncoding);
    }

    if (m_Range)
    {
        AddHeaderReqStr(L"Range", m_Range);
    }

//     AddRequestHeaders(m_hRequest, L"Range: bytes=5001-10000\r\n");
    if (m_AcceptLanguage)
    {
        AddHeaderReqStr(L"Accept-Language", m_AcceptLanguage);
    }
//     AddRequestHeaders(m_hRequest, L"Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\n");


    if (m_SetXmlHttpRequest)
    {
        AddRequestHeaders(m_hRequest, L"X-Requested-With: XMLHttpRequest\r\n");
    }

    if (m_ContentType)
    {
        AddHeaderReqStr(L"Content-Type", m_ContentType);
    }

    if (m_Tag)
    {
        AddHeaderReqStr(L"If-Range", m_Tag);
    }

    // Cookie ���� 
    if (m_cookies)
    {
        AddHeaderReqStr(L"Cookie", m_cookies);
    }

    // ���ú���CERT֤���־ �����������ҳΪHTTPS��վ 
    if (m_Ignore_Cert && m_url.GetScheme() == INTERNET_SCHEME_HTTPS)
    {
        SetRequestIgnoreCert(m_hRequest);
    }

    if (m_Disable_Receive)
    {
        SetDisableCertRedirects(m_hRequest);
    }

    // �������� 
    // Send get/post data. 
    if (SendRequest(m_hRequest, m_PostData, m_dwPostSize))
    {
        // End request
        if (EndRequest(m_hRequest))
        {
            m_IsRequest = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}

const WCHAR* CHttp::GetLocationW()  // "200" "404" "500"
{
    if (!m_IsRequest)
    {
        GetHttpRequestHeader();
    }
    if (m_tmp_header)
    {
        FreeMemory(m_tmp_header);
        m_tmp_header = NULL;
    }

    DWORD cbSize = MAX_PATH;
    m_tmp_header = (LPWSTR)AllocMemory(cbSize*sizeof(WCHAR));
#ifdef USE_WINHTTP
    if ( QueryInfo(m_hRequest, WINHTTP_QUERY_LOCATION, m_tmp_header, &cbSize) )
#else // USE_WINHTTP
    if ( QueryInfo(m_hRequest, HTTP_QUERY_LOCATION, m_tmp_header, &cbSize) )
#endif // USE_WINHTTP

    {
        return m_tmp_header;
    }
    return NULL;
}

const WCHAR* CHttp::GetReturnCodeIdW()  // "200" "404" "500"
{
    if (!m_IsRequest)
    {
        GetHttpRequestHeader();
    }
    if (m_tmp_header)
    {
        FreeMemory(m_tmp_header);
        m_tmp_header = NULL;
    }

    DWORD cbSize = MAX_PATH;
    m_tmp_header = (LPWSTR)AllocMemory(cbSize*sizeof(WCHAR));
#ifdef USE_WINHTTP
    if ( QueryInfo(m_hRequest, WINHTTP_QUERY_STATUS_CODE, m_tmp_header, &cbSize) )
#else // USE_WINHTTP
    if ( QueryInfo(m_hRequest, HTTP_QUERY_STATUS_CODE, m_tmp_header, &cbSize) )
#endif // USE_WINHTTP
    
    {
        return m_tmp_header;
    }
    return NULL;
}

const WCHAR* CHttp::GetReturnTextIdW()  // "OK"
{
    if (!m_IsRequest)
    {
        GetHttpRequestHeader();
    }
    if (m_tmp_header)
    {
        FreeMemory(m_tmp_header);
        m_tmp_header = NULL;
    }

    DWORD cbSize = MAX_PATH;
    m_tmp_header = (LPWSTR)AllocMemory(cbSize*sizeof(WCHAR));
#ifdef USE_WINHTTP
    if ( QueryInfo(m_hRequest, WINHTTP_QUERY_STATUS_TEXT, m_tmp_header, &cbSize) )
#else // USE_WINHTTP
    if ( QueryInfo(m_hRequest, HTTP_QUERY_STATUS_TEXT, m_tmp_header, &cbSize) )
#endif // USE_WINHTTP

    {
        return m_tmp_header;
    }
    return NULL;
}

const WCHAR* CHttp::GetDataLenthW()     // �����������ݳ���(��ȥͷ��) 
{
    if (!m_IsRequest)
    {
        GetHttpRequestHeader();
    }
    if (m_tmp_header)
    {
        FreeMemory(m_tmp_header);
        m_tmp_header = NULL;
    }

    DWORD cbSize = MAX_PATH;
    m_tmp_header = (LPWSTR)AllocMemory(cbSize*sizeof(WCHAR));
#ifdef USE_WINHTTP
    if ( QueryInfo(m_hRequest, WINHTTP_QUERY_CONTENT_LENGTH, m_tmp_header, &cbSize) )
#else
    if ( QueryInfo(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, m_tmp_header, &cbSize) )
#endif

    {
        return m_tmp_header;
    }
    return NULL;
}

DWORD CHttp::GetData(LPBYTE lpBuf, DWORD dwLenth, BOOL AutoChgUtf8)
{
    DWORD dwGetLen = 0;
    if (!m_IsRequest)
    {
        GetHttpRequestHeader();
    }

    DWORD ReadLenth = 0;
    while (dwLenth>dwGetLen && ReadData(m_hRequest, lpBuf+dwGetLen, dwLenth-dwGetLen, &ReadLenth, AutoChgUtf8) == TRUE && ReadLenth > 0 )
    {
        dwGetLen += ReadLenth;
    }

    // �����ѹ�������� 
    if (m_AutoUnzipData && memcmp(lpBuf, "\x1F\x8B\x08\x00", 4) == 0)
    {
        LPBYTE lpUnzipBuffer = (LPBYTE)AllocMemory(dwLenth+1);
        if (lpUnzipBuffer)
        {
            if( UnzipData(lpBuf, dwGetLen, lpUnzipBuffer, dwLenth, AutoChgUtf8) )
            {
                memcpy(lpBuf, lpUnzipBuffer, dwLenth);
                dwGetLen = strlen((char*)lpBuf);
            }
            FreeMemory(lpUnzipBuffer);
        }
    }
    return dwGetLen;
}

// gzip��ѹ����,������ʵ��Ҫ��buffer��С 
BOOL CHttp::UnzipData(LPBYTE lpInData, DWORD InLenth, LPBYTE lpOutData, DWORD OutLenth, BOOL AutoChgUtf8)
{
    // ѹ����ʽ 
    if (memcmp(lpInData, "\x1F\x8B\x08\x00", 4) == 0)
    {
        // zlib��ѹ 
        z_stream strm = {0};
        strm.avail_in = InLenth;
        strm.next_in = lpInData;
        strm.avail_out = OutLenth;
        strm.next_out = (unsigned char * ) lpOutData;
        if (inflateInit2(&strm, 47) != Z_OK || inflate(&strm, Z_NO_FLUSH) == 0)
        {
            return FALSE;
        }
        if (AutoChgUtf8)
        {
            UTF8ToANSI((char*)lpOutData,  OutLenth);
        }
    }
    return TRUE;
}

const WCHAR* CHttp::GetSetCookieW()     // �����µ� Cookie WINHTTP_QUERY_SET_COOKIE/HTTP_QUERY_SET_COOKIE
{
    if (!m_IsRequest)
    {
        GetHttpRequestHeader();
    }
    if (m_tmp_header)
    {
        FreeMemory(m_tmp_header);
        m_tmp_header = NULL;
    }

    DWORD cbSize = MAX_PATH*2;
    m_tmp_header = (LPWSTR)AllocMemory(cbSize*sizeof(WCHAR));
#ifdef USE_WINHTTP
    if ( QueryInfo(m_hRequest, WINHTTP_QUERY_SET_COOKIE, m_tmp_header, &cbSize) )
#else // USE_WINHTTP
    if ( QueryInfo(m_hRequest, HTTP_QUERY_SET_COOKIE, m_tmp_header, &cbSize) )
#endif // USE_WINHTTP

    {
        return m_tmp_header;
    }
    return NULL;
}

const char* CHttp::GetLocationA()  // "200" "404" "500"
{
    LPCWSTR lpData = GetLocationW();
    if (lpData != NULL)
    {
        if (m_tmp_string)
        {
            FreeMemory(m_tmp_string);
            m_tmp_string = NULL;
        }
        m_tmp_string = WideToMul(lpData);
        return m_tmp_string;
    }
    return NULL;
}

const char* CHttp::GetReturnCodeIdA()  // "200" "404" "500"
{
    LPCWSTR lpData = GetReturnCodeIdW();
    if (lpData != NULL)
    {
        if (m_tmp_string)
        {
            FreeMemory(m_tmp_string);
            m_tmp_string = NULL;
        }
        m_tmp_string = WideToMul(lpData);
        return m_tmp_string;
    }
    return NULL;
}

const char* CHttp::GetReturnTextIdA()  // "OK"
{
    LPCWSTR lpData = GetReturnTextIdW();
    if (lpData != NULL)
    {
        if (m_tmp_string)
        {
            FreeMemory(m_tmp_string);
            m_tmp_string = NULL;
        }
        m_tmp_string = WideToMul(lpData);
        return m_tmp_string;
    }
    return NULL;
}

const char* CHttp::GetDataLenthA()     // �����������ݳ���(��ȥͷ��) 
{
    LPCWSTR lpData = GetDataLenthW();
    if (lpData != NULL)
    {
        if (m_tmp_string)
        {
            FreeMemory(m_tmp_string);
            m_tmp_string = NULL;
        }
        m_tmp_string = WideToMul(lpData);
        return m_tmp_string;
    }
    return NULL;
}

const char* CHttp::GetSetCookieA()  
{
    LPCWSTR lpData = GetSetCookieW();
    if (lpData != NULL)
    {
        if (m_tmp_string)
        {
            FreeMemory(m_tmp_string);
            m_tmp_string = NULL;
        }
        m_tmp_string = WideToMul(lpData);
        return m_tmp_string;
    }
    return NULL;
}


// WINHTTP_QUERY_ETAG   ->  ETag: "lr0ZzoawyyhskvuleG6PNUPXjzKs" 
const char* CHttp::GetETagA()
{
    LPCWSTR lpData = GetETagW();
    if (lpData != NULL)
    {
        if (m_tmp_string)
        {
            FreeMemory(m_tmp_string);
            m_tmp_string = NULL;
        }
        m_tmp_string = WideToMul(lpData);
        return m_tmp_string;
    }
    return NULL;
}

const WCHAR* CHttp::GetETagW()
{
    if (!m_IsRequest)
    {
        GetHttpRequestHeader();
    }
    if (m_tmp_header)
    {
        FreeMemory(m_tmp_header);
        m_tmp_header = NULL;
    }

    DWORD cbSize = MAX_PATH*2;
    m_tmp_header = (LPWSTR)AllocMemory(cbSize*sizeof(WCHAR));
#ifdef USE_WINHTTP
    if ( QueryInfo(m_hRequest, WINHTTP_QUERY_ETAG, m_tmp_header, &cbSize) )
#else // USE_WINHTTP
    if ( QueryInfo(m_hRequest, HTTP_QUERY_ETAG, m_tmp_header, &cbSize) )
#endif // USE_WINHTTP

    {
        return m_tmp_header;
    }
    return NULL;
}

unsigned long long CHttp::LoopToGetData()
{
    unsigned long long dwTotalRecvSize = 0;
#define HTTP_BUFFER_SIZE 4*1024*1024
    DWORD cbSize = HTTP_BUFFER_SIZE;
    LPBYTE lpBuffer = (LPBYTE)AllocMemory(HTTP_BUFFER_SIZE);

    if (!m_IsRequest)
    {
        GetHttpRequestHeader();
    }
    while(ReadData(m_hRequest, lpBuffer, HTTP_BUFFER_SIZE, &cbSize, FALSE) == TRUE && cbSize > 0 )
    {
        dwTotalRecvSize += cbSize;
        if (m_func == NULL)
        {
            printf("%s\n", lpBuffer);
        }
        else
        {
            if ( m_func(lpBuffer, cbSize, m_userdata) != cbSize)
            {
                break;
            }
        }
    }
    return dwTotalRecvSize;
}

CHttp::~CHttp()
{
    Reset();
}