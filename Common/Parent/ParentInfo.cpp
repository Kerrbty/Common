#include "ParentInfo.h"
#include <Tlhelp32.h>
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

// 查询父进程ID 
DWORD WINAPI GetParentID(DWORD dwChildPID)
{
    PROCESSENTRY32 lppe = {sizeof(PROCESSENTRY32)};
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (dwChildPID == 0)
    {
        dwChildPID = GetCurrentProcessId();
    }
    if (hSnapshot != INVALID_HANDLE_VALUE && Process32First(hSnapshot, &lppe))
    {
        do 
        {
            if (lppe.th32ProcessID == dwChildPID)
            {
                CloseHandle(hSnapshot);
                return lppe.th32ParentProcessID;
            }
        } while (Process32Next(hSnapshot, &lppe));
        CloseHandle(hSnapshot);
    }
    return 0;
}

// 查询父进程exe模块完整路径 
// QueryFullProcessImageName
// GetModuleFileNameEx
BOOL WINAPI GetParentExeModule(LPTSTR lpBuf, DWORD dwSize, DWORD dwChildPID)
{
    DWORD dwParentPid = GetParentID(dwChildPID);
    if (dwParentPid != 0)
    {
        HANDLE ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, dwParentPid);
        if (ProcessHandle != NULL)
        {
            DWORD dwReadLenth = GetModuleFileNameEx(ProcessHandle, NULL, lpBuf, dwSize);  // Psapi.dll 
            if (dwReadLenth != 0)
            {
                return TRUE;
            }
            dwReadLenth = dwSize;
            if (QueryFullProcessImageName(ProcessHandle, 0, lpBuf, &dwReadLenth) && dwReadLenth > 0)
            {
                return TRUE;
            }
        }
        
    }
    return FALSE;
}


// 查询父进程是否是GUI程序，CUI返回否 
BOOL WINAPI IsParentGUI(BOOL* bGui, DWORD dwChildPID)
{
    BOOL isSuccess = FALSE;
    LPTSTR lpBufExe = (LPTSTR)AllocMemory(MAX_PATH*2*sizeof(TCHAR));
    if (lpBufExe)
    {
        if( GetParentExeModule(lpBufExe, MAX_PATH*2) )
        {
            HANDLE hExeFile = FileOpen(lpBufExe, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
            if (hExeFile != INVALID_HANDLE_VALUE)
            {
                DWORD dwBytes = 0;
                LPBYTE lpPEBuf = (LPBYTE)AllocMemory(4096);
                SetFilePointer(hExeFile, 0, NULL, FILE_BEGIN);
                ReadFile(hExeFile, lpPEBuf, 4096, &dwBytes, NULL);
                CloseHandle(hExeFile);

                if (dwBytes > 0x400)
                {
                    PIMAGE_DOS_HEADER hImageDos = (PIMAGE_DOS_HEADER)lpPEBuf;
                    if( hImageDos->e_magic == IMAGE_DOS_SIGNATURE && hImageDos->e_lfanew < dwBytes)
                    {
                        PIMAGE_NT_HEADERS hImageNt = (PIMAGE_NT_HEADERS)((PBYTE)hImageDos + hImageDos->e_lfanew);
                        if ( hImageNt->Signature == IMAGE_NT_SIGNATURE )
                        {
                            isSuccess = TRUE;
                            if (bGui)
                            {
//                                 if (hImageNt->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
                                // 没必要判断，因为64位结构体少了 BaseOfData ，扩展了 ImageBase 为 64位，刚好 Subsystem 偏移不变 
                                if (hImageNt->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI)
                                {
                                    *bGui = FALSE;
                                }
                                else if (hImageNt->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
                                {
                                    *bGui = TRUE;
                                }
                            }
                        }
                    }
                }
                FreeMemory(lpPEBuf);
            }
        }
        FreeMemory(lpBufExe);
    }
    return isSuccess;
}