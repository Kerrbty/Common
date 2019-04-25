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
void * _AddressOfReturnAddress();//���ص�ǰ�����ķ��ص�ַ��

// ����ģʽ�£����з����Ժ�������Ч
#ifdef _DEBUG

#define InitAntiDebug()

#else

BOOL InitAntiDebug();

#endif
