#ifndef _LINUX_PROCESS_HEADER_H_
#define _LINUX_PROCESS_HEADER_H_
#include <inttypes.h>
#ifdef  __cplusplus
extern "C" {
#endif // __cplusplus

const char* PathGetFileName(char* szPathFile);

void*  GetModuleHandle(const char* ModuleName);
bool IsBadReadPtr(void* addr, unsigned int len);
bool IsBadWritePtr(void* addr, unsigned int len);

char* GetModuleFileName(void* hModule, char* szFileName, unsigned int len); // readlink();

#ifdef __cplusplus
};
#endif // __cplusplus

#endif // _LINUX_PROCESS_HEADER_H_
