#pragma once
#include "CheckProcess.h"

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif
// _ReturnAddress and _AddressOfReturnAddress should be prototyped before use
EXTERNC void* _AddressOfReturnAddress(void);
EXTERNC void* _ReturnAddress(void);
#pragma intrinsic(_AddressOfReturnAddress)
#pragma intrinsic(_ReturnAddress)
void * _AddressOfReturnAddress();//返回当前函数的返回地址。

// 调试模式下，所有反调试函数都无效
#ifdef _DEBUG

#define InitAntiDebug()

#else

BOOL InitAntiDebug();

#endif
