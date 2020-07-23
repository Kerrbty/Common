#ifndef _HTTP_CONFIG_HEADER_201904141545_H_
#define _HTTP_CONFIG_HEADER_201904141545_H_
#include <Windows.h>
// #define USE_WINHTTP



#ifdef USE_WINHTTP 
// xp sp3
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else // USE_WINHTTP 
// xp
#include <WinInet.h>
#pragma comment(lib, "wininet.lib")
#endif // USE_WINHTTP 



#endif // _HTTP_CONFIG_HEADER_201904141545_H_