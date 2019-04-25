#pragma once
#include <windows.h>

BOOL GetFileNameByHandle(HANDLE hFile, LPTSTR buff, DWORD size);
BOOL GetProcessKernelObjectName(HANDLE hHandleObjKernel, PTSTR namebuffer, DWORD size);