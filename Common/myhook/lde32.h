#pragma once

#ifdef __cplusplus
#define EXTCFUN extern "C" 
#else
#define EXTCFUN
#endif

EXTCFUN int __stdcall LDE(void* , int); // �ڶ������� 0 

#ifndef _WIN64
#pragma comment(lib, "LDE32.lib")
#else
#pragma comment(lib, "LDE64.lib")
#endif