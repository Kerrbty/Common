// OperateProcess.cpp : Defines the entry point for the console application.
//

#include "OperateProcess.h"
#include <Psapi.h>
#include <Tlhelp32.h>

#pragma comment(lib, "Psapi")
#pragma comment(lib, "ws2_32")


//=====================================================================================//
//Name: DWORD GetProcessIdByPort()                                                     //
//                                                                                     //
//Descripion: ���ݶ˿ں�����򿪸ö˿ںŵĽ��� ID(֧�� XP��Server 2003��Vista��Win7)   //
//                                                                                     //
//=====================================================================================//



#define ANY_SIZE 1

typedef struct{
    DWORD dwState;      //����״̬
    DWORD dwLocalAddr;  //���ص�ַ
    DWORD dwLocalPort;  //���ض˿�
    DWORD dwRemoteAddr; //Զ�̵�ַ
    DWORD dwRemotePort; //Զ�̶˿�
    DWORD dwProcessId;  //���̱�ʶ
    DWORD Unknown;      //������ʶ
}MIB_TCPEXROW_VISTA,*PMIB_TCPEXROW_VISTA;

typedef struct{
    DWORD dwNumEntries;
    MIB_TCPEXROW_VISTA table[ANY_SIZE];
}MIB_TCPEXTABLE_VISTA,*PMIB_TCPEXTABLE_VISTA;

typedef DWORD (WINAPI *PFNInternalGetTcpTable2)(
    PMIB_TCPEXTABLE_VISTA *pTcpTable_Vista,
    HANDLE heap,
    DWORD flags
    );

//=====================================================================================//
//Name: DWORD InternalGetUdpTableWithOwnerPid()                                        //
//                                                                                     //
//Descripion: �ú����� Windows Vista �Լ� Windows 7 ����Ч                                    //
//                                                                                     //
//=====================================================================================//

typedef struct{
    DWORD dwLocalAddr;  //���ص�ַ
    DWORD dwLocalPort;  //���ض˿�
    DWORD dwProcessId;  //���̱�ʶ
}MIB_UDPEXROW,*PMIB_UDPEXROW;

typedef struct{
    DWORD dwNumEntries;
    MIB_UDPEXROW table[ANY_SIZE];
}MIB_UDPEXTABLE,*PMIB_UDPEXTABLE;


typedef DWORD (WINAPI *PFNInternalGetUdpTableWithOwnerPid)(
    PMIB_UDPEXTABLE *pUdpTable,
    HANDLE heap,
    DWORD flags
    );

typedef struct{
    DWORD dwState;      //����״̬
    DWORD dwLocalAddr;  //���ص�ַ
    DWORD dwLocalPort;  //���ض˿�
    DWORD dwRemoteAddr; //Զ�̵�ַ
    DWORD dwRemotePort; //Զ�̶˿�
    DWORD dwProcessId;  //���̱�ʶ
}MIB_TCPEXROW,*PMIB_TCPEXROW;

typedef struct{
    DWORD dwNumEntries;
    MIB_TCPEXROW table[ANY_SIZE];
}MIB_TCPEXTABLE,*PMIB_TCPEXTABLE;

typedef DWORD (WINAPI* PFNAllocateAndGetTcpExTableFromStack)(
    _Out_ PVOID  *ppTcpTable,
    _In_  BOOL   bOrder,
    _In_  HANDLE hHeap,
    _In_  DWORD  dwFlags,
    _In_  DWORD  dwFamily
    );

typedef DWORD (WINAPI *PFNAllocateAndGetUdpExTableFromStack)(
    PMIB_UDPEXTABLE *pUdpTable,
    bool bOrder,
    HANDLE heap,
    DWORD zero,
    DWORD flags
    );

DWORD CProcOperate::GetProcessIdByPort(TcpOrUdp type, DWORD dwPort)
{
    HMODULE hModule = LoadLibraryW(L"iphlpapi.dll");
    if (hModule == NULL)
    {
        return 0;
    }

    if(type == TcpType)
    {
        // ������ѯ���� TCP ��Ϣ
        PFNAllocateAndGetTcpExTableFromStack pAllocateAndGetTcpExTableFromStack;
        pAllocateAndGetTcpExTableFromStack = 
            (PFNAllocateAndGetTcpExTableFromStack)GetProcAddress(hModule, "AllocateAndGetTcpExTableFromStack");
        if (pAllocateAndGetTcpExTableFromStack != NULL)
        {
            // ����Ϊ XP ���� Server 2003 ����ϵͳ
            PMIB_TCPEXTABLE pTcpExTable ;
            if (pAllocateAndGetTcpExTableFromStack((PVOID *)&pTcpExTable, TRUE, GetProcessHeap(), 0, AF_INET) != 0)
            {
                if (pTcpExTable)
                {
                    HeapFree(GetProcessHeap(), 0, pTcpExTable);
                }

                FreeLibrary(hModule);
                hModule = NULL;

                return 0;
            }

            for (UINT i = 0; i < pTcpExTable->dwNumEntries; i++)
            {
                // ���˵����ݣ�ֻ��ѯ������Ҫ�Ľ�������
                if(dwPort == ntohs(0x0000FFFF & pTcpExTable->table[i].dwLocalPort))
                {
                    DWORD dwProcessId = pTcpExTable->table[i].dwProcessId;
                    if (pTcpExTable)
                    {
                        HeapFree(GetProcessHeap(), 0, pTcpExTable);
                    }

                    FreeLibrary(hModule);
                    hModule = NULL;

                    return dwProcessId;
                }
            }

            if (pTcpExTable)
            {
                HeapFree(GetProcessHeap(), 0, pTcpExTable);
            }

            FreeLibrary(hModule);
            hModule = NULL;

            return 0;
        }
        else
        {
            // ����Ϊ Vista ���� 7 ����ϵͳ
            PMIB_TCPEXTABLE_VISTA pTcpExTable = NULL;
            PFNInternalGetTcpTable2 pInternalGetTcpTable2 = 
                (PFNInternalGetTcpTable2)GetProcAddress(hModule, "InternalGetTcpTable2");
            if (pInternalGetTcpTable2 == NULL)
            {
                if (pTcpExTable)
                {
                    HeapFree(GetProcessHeap(), 0, pTcpExTable);
                }

                FreeLibrary(hModule);
                hModule = NULL;

                return 0;
            }

            if (pInternalGetTcpTable2(&pTcpExTable, GetProcessHeap(), 1))
            {
                if (pTcpExTable)
                {
                    HeapFree(GetProcessHeap(), 0, pTcpExTable);
                }

                FreeLibrary(hModule);
                hModule = NULL;

                return 0;
            }

            for (UINT i = 0;i < pTcpExTable->dwNumEntries; i++)
            {
                // ���˵����ݣ�ֻ��ѯ������Ҫ�Ľ�������
                if(dwPort == ntohs(0x0000FFFF & pTcpExTable->table[i].dwLocalPort))
                {
                    DWORD dwProcessId = pTcpExTable->table[i].dwProcessId;
                    if (pTcpExTable)
                    {
                        HeapFree(GetProcessHeap(), 0, pTcpExTable);
                    }

                    FreeLibrary(hModule);
                    hModule = NULL;

                    return dwProcessId;
                }
            }

            if (pTcpExTable)
            {
                HeapFree(GetProcessHeap(), 0, pTcpExTable);
            }

            FreeLibrary(hModule);
            hModule = NULL;

            return 0;
        }
    }
    else if(type == UdpType)
    {
        // ������ѯ���� UDP ��Ϣ
        PMIB_UDPEXTABLE pUdpExTable = NULL;
        PFNAllocateAndGetUdpExTableFromStack pAllocateAndGetUdpExTableFromStack;
        pAllocateAndGetUdpExTableFromStack = 
            (PFNAllocateAndGetUdpExTableFromStack)GetProcAddress(hModule,"AllocateAndGetUdpExTableFromStack");
        if (pAllocateAndGetUdpExTableFromStack != NULL)
        {
            // ����Ϊ XP ���� Server 2003 ����ϵͳ
            if (pAllocateAndGetUdpExTableFromStack(&pUdpExTable, TRUE, GetProcessHeap(), 0, AF_INET) != 0)
            {
                if (pUdpExTable)
                {
                    HeapFree(GetProcessHeap(), 0, pUdpExTable);
                }

                FreeLibrary(hModule);
                hModule = NULL;

                return 0;
            }

            for (UINT i = 0; i < pUdpExTable->dwNumEntries; i++)
            {
                // ���˵����ݣ�ֻ��ѯ������Ҫ�Ľ�������
                if (dwPort == ntohs(0x0000FFFF & pUdpExTable->table[i].dwLocalPort))
                {
                    DWORD dwProcessId = pUdpExTable->table[i].dwProcessId;
                    if (pUdpExTable)
                    {
                        HeapFree(GetProcessHeap(), 0, pUdpExTable);
                    }

                    FreeLibrary(hModule);
                    hModule = NULL;

                    return dwProcessId;
                }
            }

            if (pUdpExTable)
            {
                HeapFree(GetProcessHeap(), 0, pUdpExTable);
            }

            FreeLibrary(hModule);
            hModule = NULL;

            return 0;
        }
        else
        {
            // ����Ϊ Vista ���� 7 ����ϵͳ
            PFNInternalGetUdpTableWithOwnerPid pInternalGetUdpTableWithOwnerPid;
            pInternalGetUdpTableWithOwnerPid = 
                (PFNInternalGetUdpTableWithOwnerPid)GetProcAddress(hModule, "InternalGetUdpTableWithOwnerPid");
            if (pInternalGetUdpTableWithOwnerPid != NULL)
            {
                if (pInternalGetUdpTableWithOwnerPid(&pUdpExTable, GetProcessHeap(), 1))
                {
                    if (pUdpExTable)
                    {
                        HeapFree(GetProcessHeap(), 0, pUdpExTable);
                    }

                    FreeLibrary(hModule);
                    hModule = NULL;

                    return 0;
                }

                for (UINT i = 0; i < pUdpExTable->dwNumEntries; i++)
                {
                    // ���˵����ݣ�ֻ��ѯ������Ҫ�Ľ�������
                    if (dwPort == ntohs(0x0000FFFF & pUdpExTable->table[i].dwLocalPort))
                    {
                        DWORD dwProcessId = pUdpExTable->table[i].dwProcessId;
                        if (pUdpExTable)
                        {
                            HeapFree(GetProcessHeap(), 0, pUdpExTable);
                        }

                        FreeLibrary(hModule);
                        hModule = NULL;

                        return dwProcessId;
                    }
                }
            }

            if (pUdpExTable)
            {
                HeapFree(GetProcessHeap(), 0, pUdpExTable);
            }

            FreeLibrary(hModule);
            hModule = NULL;

            return 0;
        }
    }
    else
    {
        FreeLibrary(hModule);
        hModule = NULL;

        return -1;
    }
}



bool CProcOperate::EnableDebugPrivilege()  
{  
    HANDLE hToken;  
    LUID sedebugnameValue;  
    TOKEN_PRIVILEGES tkp;  
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {  
        return   FALSE;  
    }  
    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue)) 
    {  
        CloseHandle(hToken);  
        return false;  
    }  
    tkp.PrivilegeCount = 1;  
    tkp.Privileges[0].Luid = sedebugnameValue;  
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
    {  
        CloseHandle(hToken);  
        return false;  
    }  
    return true;  
}



LPTSTR CProcOperate::GetFileNameByPid(DWORD pid, LPTSTR szProcName, DWORD dwNameLenth)
{
    LPTSTR retval = NULL;

    if (szProcName != NULL)
    {
        memset(szProcName, 0, dwNameLenth*sizeof(TCHAR));
    }

    if (pid == 0)
    {
        return NULL;
    }
    
    HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, pid); 

    if (handle == NULL)
    {
        return NULL;
    }

    do 
    {
        if ( GetModuleFileNameEx(handle, NULL, szProcName, dwNameLenth) != 0 )
        {
            retval = szProcName;
            break;
        }

        if (GetProcessImageFileName(handle, szProcName, dwNameLenth) != 0)
        {
            retval = szProcName;
            break;
        }

        char* cmdname = "GetCommandLine"
#ifdef _UNICODE
            "W";
#else
            "A";
#endif

        BOOL IsWin32Process = FALSE;
        if (IsWow64Process(handle, &IsWin32Process) && IsWin32Process)
        {
            LPBYTE cmdline = (LPBYTE)GetProcAddress(GetModuleHandle(TEXT("KernelBase")), cmdname);
            if (cmdline == NULL)
            {
                cmdline = (LPBYTE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), cmdname);
            }

            if (cmdline != NULL)
            {
                DWORD dwBytes;
                if( ReadProcessMemory(handle, cmdline+1, &cmdline, 4, &dwBytes) )
                {
                    if (ReadProcessMemory(handle, cmdline, &cmdline, 4, &dwBytes) && ReadProcessMemory(handle, cmdline, szProcName, dwNameLenth, &dwBytes))
                    {
                        retval = szProcName;
                    }
                }
            }
        }
    } while (0);


    CloseHandle(handle);
    return retval;
}





BOOL CProcOperate::KillProcess(DWORD pid)
{
    BOOL success = FALSE;

    if (pid == 0)
    {
        return TRUE;
    }

    // ֱ��ɱ��
    HANDLE hTreminateThread = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hTreminateThread != NULL)
    {
        success = TerminateProcess(hTreminateThread, 0);
        CloseHandle(hTreminateThread);
    }
   

    // ����ExitProcessɱ����
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

    // ö���̣߳�ɱ�߳�
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


CProcOperate::CProcOperate()
{
    WORD wVersionRequested  = MAKEWORD(1, 1);
    WSADATA wsaData = {0};
    WSAStartup(wVersionRequested, &wsaData);

    EnableDebugPrivilege();

}


CProcOperate::~CProcOperate()
{
    WSACleanup();
}



