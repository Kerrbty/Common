#include "Hook.h"
#include "mykernel32.h"


typedef HANDLE (WINAPI *PCreateFileA)(LPSTR lpFileName,                         // file name
									  DWORD dwDesiredAccess,                      // access mode
									  DWORD dwShareMode,                          // share mode
									  LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
									  DWORD dwCreationDisposition,                // how to create
									  DWORD dwFlagsAndAttributes,                 // file attributes
									  HANDLE hTemplateFile                        // handle to template file
									);

typedef HANDLE (WINAPI *PCreateFileW)(LPWSTR lpFileName,                         // file name
									  DWORD dwDesiredAccess,                      // access mode
									  DWORD dwShareMode,                          // share mode
									  LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
									  DWORD dwCreationDisposition,                // how to create
									  DWORD dwFlagsAndAttributes,                 // file attributes
									  HANDLE hTemplateFile                        // handle to template file
									);

// ���ﶨ��
// hook��Ĵ�����
// ��������ԭ����������1�������ǵ�һ���������õ���Դ���������ص�ַ��
HANDLE WINAPI ComplagteCreateFileW(LPWSTR lpFileName,                         // file name
                                  DWORD dwDesiredAccess,                      // access mode
                                  DWORD dwShareMode,                          // share mode
                                  LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                                  DWORD dwCreationDisposition,                // how to create
                                  DWORD dwFlagsAndAttributes,                 // file attributes
                                  HANDLE hTemplateFile                        // handle to template file
                                  )
{
        LPDWORD addri = (LPDWORD)(&lpFileName);
		addri--;
 //       *(addri-1) = GetNewAddress(ComplagteCreateFileW); // ���ص�ַ

        if (wcsstr(lpFileName, L"Registry.db") != NULL )
        {
                // �ҵ����õط�
                HMODULE hmodule; 
                WCHAR FileName[MAX_PATH] = {0}; 
                GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (PTSTR)*addri, &hmodule); 
                GetModuleFileNameW(hmodule, FileName, MAX_PATH); 
                
                LPWSTR writestr = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH*3*sizeof(WCHAR)); 
                
                wsprintfW(        writestr, 
                        L"���� CreateFileW ������:%s\r\n"
                        L" Module address:%08X, Base Offset:%08X, Memory Address:%08X \r\n" 
                        L"���ļ�: %s\r\n\r\n", 
                        FileName, hmodule, (DWORD)*addri - (DWORD)hmodule, *addri, lpFileName); 
                
                OutputDebugStringW(writestr); 
                
                HeapFree(GetProcessHeap(), NULL, writestr); 
        }
		PCreateFileW dwCreate = (PCreateFileW)GetNewAddress(ComplagteCreateFileW);
        return dwCreate(lpFileName,
						dwDesiredAccess,
						dwShareMode,
						lpSecurityAttributes,
						dwCreationDisposition,
						dwFlagsAndAttributes,
						hTemplateFile);
}


HANDLE WINAPI ComplagteCreateFileA(LPSTR lpFileName,                         // file name
                                   DWORD dwDesiredAccess,                      // access mode
                                   DWORD dwShareMode,                          // share mode
                                   LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
                                   DWORD dwCreationDisposition,                // how to create
                                   DWORD dwFlagsAndAttributes,                 // file attributes
                                   HANDLE hTemplateFile                        // handle to template file
                                   )
{
        LPDWORD addri = (LPDWORD)(&lpFileName);
		addri--;
 //       *(addri-1) = GetNewAddress(ComplagteCreateFileW); // ���ص�ַ

        if (strstr(lpFileName, "Registry.db") != NULL )
        {
                // �ҵ����õط�
                HMODULE hmodule;
                char FileName[MAX_PATH] = {0};
                GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (PTSTR)*addri, &hmodule);
                GetModuleFileNameA(hmodule, FileName, MAX_PATH);
                
                LPSTR writestr = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH*3);
                
                wsprintfA(        writestr, 
                        "���� CreateFileW ������:%s\r\n"
                        " Module address:%08X, Base Offset:%08X, Memory Address:%08X \r\n"
                        "���ļ�: %s\r\n\r\n", 
                        FileName, hmodule, *addri - (DWORD)hmodule, *addri, lpFileName);
                OutputDebugStringA( writestr );
                
                HeapFree(GetProcessHeap(), NULL, writestr);
        }
		PCreateFileA dwCreate = (PCreateFileA)GetNewAddress(ComplagteCreateFileA);
        return dwCreate(lpFileName,
						dwDesiredAccess,
						dwShareMode,
						lpSecurityAttributes,
						dwCreationDisposition,
						dwFlagsAndAttributes,
						hTemplateFile);
}

// hook����
DWORD WINAPI MyHookAll(LPVOID)
{
//	OutputDebugStringA("Hook called!");
    HookProcByName(TEXT("kernel32.dll"), "CreateFileA", ComplagteCreateFileA);
    HookProcByName(TEXT("kernel32.dll"), "CreateFileW", ComplagteCreateFileW);
	return 0;
}



DWORD WINAPI MyUnHookAll(LPVOID)
{
//	OutputDebugStringA("UnHook called!");
	UnHookProcByName(TEXT("kernel32.dll"), "CreateFileA", ComplagteCreateFileA);
	UnHookProcByName(TEXT("kernel32.dll"), "CreateFileW", ComplagteCreateFileW);
	return 0;
}