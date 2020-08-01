#ifndef _HTTP_CONFIG_HEADER_201904141545_H_
#define _HTTP_CONFIG_HEADER_201904141545_H_
#include <Windows.h>
#define USE_WINHTTP

#ifdef USE_WINHTTP 
    // Windows XP��Windows 2000 Professional SP3 ���Ժ�汾 
    // win8 ��ʼ����websocketЭ�飬���磺WinHttpWebSocketSend��WinHttpWebSocketReceive 
    #include <winhttp.h>
    #pragma comment(lib, "winhttp.lib")
#else // USE_WINHTTP 
    // Windows XP��Windows Server 2003 R2������汾 
    // WinINet��֧�ַ�����ʵ��,���ڷ�����ʵ�ֻ����ʹ��WinHTTP 
    #include <WinInet.h>
    #pragma comment(lib, "wininet.lib")
#endif // USE_WINHTTP 

#endif // _HTTP_CONFIG_HEADER_201904141545_H_