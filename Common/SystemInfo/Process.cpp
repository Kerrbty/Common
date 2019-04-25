#include "Process.h"
#include <Tlhelp32.h>
#include "Debug.h"
#include "Common.h"
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi")

BOOL WINAPI KillProcessByPid(DWORD pid)
{
    BOOL success = FALSE;

    if (pid == 0)
    {
        return TRUE;
    }

    // 直接杀死
    HANDLE hTreminateThread = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hTreminateThread != NULL)
    {
        success = TerminateProcess(hTreminateThread, 0);
        CloseHandle(hTreminateThread);
    }


    // 运行ExitProcess杀进程
    if ( !success )
    {
        HANDLE hThreadExit = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (hThreadExit != NULL)
        {
            BOOL IsWin32Process = FALSE;
            if (IsWow64Process(hThreadExit, &IsWin32Process) && IsWin32Process)
            {
                LPBYTE pexit = (LPBYTE)GetProcAddress(GetModuleHandle(TEXT("KernelBase")), "ExitProcess");
                if (pexit == NULL)
                {
                    pexit = (LPBYTE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "ExitProcess");
                }

                HANDLE hThread = CreateRemoteThread(hThreadExit, NULL, 0, (LPTHREAD_START_ROUTINE)pexit, NULL, 0, NULL);
                if (hThread != NULL)
                {
                    success = TRUE;
                    CloseHandle(hThread);
                }
            }
            CloseHandle(hThreadExit);
        }
    }

    // 枚举线程，杀线程
    if (!success)
    {
        HANDLE hSnapshot  = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
        if (INVALID_HANDLE_VALUE != hSnapshot )
        {
            success = TRUE;
            THREADENTRY32 te = {sizeof(THREADENTRY32)};
            if( Thread32First(hSnapshot,&te) )
            {
                do 
                {
                    if (te.th32OwnerProcessID == pid)
                    {
                        HANDLE hThread = OpenThread(THREAD_TERMINATE, FALSE, te.th32ThreadID);
                        if (hThread)
                        {
                            if ( !TerminateThread(hThread, 0) )
                            {
                                success = FALSE;
                            }
                            CloseHandle(hThread);
                        }

                        if ( !success )
                        {
                            CONTEXT context = {0};
                            HANDLE hThead2 = OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_SET_CONTEXT, FALSE, te.th32ThreadID);
                            if (hThead2)
                            {
                                SuspendThread(hThead2);
                                context.ContextFlags = CONTEXT_CONTROL;
                                GetThreadContext(hThead2, &context);
                                context.Eip = (DWORD)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "ExitProcess");
                                if ( !SetThreadContext(hThead2, &context) )
                                {
                                    success = FALSE;
                                }
                                ResumeThread(hThead2);

                                CloseHandle(hThead2);
                            }
                        }
                    }
                } while (Thread32Next(hSnapshot, &te));
            }
            CloseHandle(hSnapshot);
        }

    }

    return success;
}

BOOL WINAPI KillProcessByName(LPTSTR lpName)
{
    BOOL bkill = FALSE;
    if(lpName == NULL)  
    {  
        return 0;  
    }  

    HANDLE hProcessSnap = NULL;  
    PROCESSENTRY32 pe32 = {sizeof(PROCESSENTRY32)};  

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);  
    if(hProcessSnap != INVALID_HANDLE_VALUE) 
    {
        if( Process32First(hProcessSnap, &pe32) )  
        {  
            do  
            {  
                if(_tcsicmp(pe32.szExeFile, lpName) == 0)  
                {  
                    bkill = KillProcessByPid(pe32.th32ProcessID);
                }  
            }while(Process32Next(hProcessSnap,&pe32));  
        }  
        CloseHandle(hProcessSnap);  
    }
    return bkill;  
}

//////////////////////////////////////////////////////////////////////////
// 启动程序 
DWORD WINAPI GetPIDByName(LPTSTR lpName)
{
    DWORD dwpid = 0;
    if(lpName == NULL)  
    {  
        return 0;  
    }  

    HANDLE hProcessSnap = NULL;  
    PROCESSENTRY32 pe32 = {sizeof(PROCESSENTRY32)};  

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);  
    if(hProcessSnap != INVALID_HANDLE_VALUE) 
    {
        if( Process32First(hProcessSnap, &pe32) )  
        {  
            do  
            {  
                if(_tcsicmp(pe32.szExeFile, lpName) == 0)  
                {  
                    dwpid = pe32.th32ProcessID;
                    break;  
                }  
            }while(Process32Next(hProcessSnap,&pe32));  
        }  
        CloseHandle(hProcessSnap);  
    }
    return dwpid;  
}

BOOL WINAPI GetTokenByName(HANDLE &hToken, LPTSTR lpName)  
{  
    DWORD dwpid = GetPIDByName(lpName);
    if (dwpid == 0)
    {
        return FALSE;
    }

    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION,  
        FALSE, 
        dwpid);  
    if ( OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken) )
    {
        return TRUE;
    }
    return FALSE;
}  

BOOL WINAPI RunProcess(LPTSTR lpImage, LPTSTR commandLine, BOOL isUIProcess)  
{  
    BOOL bResult = FALSE;
    STARTUPINFO si = {sizeof(STARTUPINFO)};  
    PROCESS_INFORMATION pi;  

    si.wShowWindow = SW_SHOW; 
    si.dwFlags = STARTF_USESHOWWINDOW; 

    LPTSTR lpCurrentDir = (LPTSTR)AllocMemory(MAX_PATH*sizeof(TCHAR));
    GetModuleFileName(NULL, lpCurrentDir, MAX_PATH);
    PathRemoveFileSpec(lpCurrentDir);
    if (isUIProcess)
    {
        HANDLE hToken; 
        if( !GetTokenByName(hToken, TEXT("explorer.exe")) )  
        {  
            _dbgprint("GetTokenByName fail\n");
            return bResult;  
        }  
        si.lpDesktop = TEXT("winsta0\\default");  

        bResult = CreateProcessAsUser( hToken, 
            lpImage, 
            commandLine, 
            NULL, 
            NULL,  
            FALSE,
            NORMAL_PRIORITY_CLASS, 
            NULL, 
            lpCurrentDir, 
            &si, 
            &pi);  
        CloseHandle(hToken);  
    }
    else
    {
        bResult = CreateProcess(
            lpImage, 
            commandLine, 
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            lpCurrentDir,
            &si,
            &pi);
    }

    FreeMemory(lpCurrentDir);
    if(bResult)  
    {  
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        _dbgprint("CreateProcess success\n");
    }  
    else  
    {  
        _dbgprint("CreateProcess fail\n");
    }  
    return bResult;  
}  