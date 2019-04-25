// RunService.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <Tlhelp32.h>
#include <tchar.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
//#pragma comment(lib, "")


DWORD processNameToId(LPCTSTR lpszProcessName)
{
	DWORD RetValue = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe;

	pe.dwSize = sizeof(PROCESSENTRY32);
	
	if (!Process32First(hSnapshot, &pe)) 
	{
		return -1;
	}
	
	while (Process32Next(hSnapshot, &pe)) {
		if (!_tcsicmp(lpszProcessName, pe.szExeFile)) {
			RetValue = pe.th32ProcessID;
			break;
		}
	}
	CloseHandle(hSnapshot);
	return RetValue;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	while(TRUE)
	{
		if ( processNameToId(TEXT("WebService.exe")) == 0)
		{
			WinExec(TEXT("WebService.exe"), SW_HIDE);
		}
		Sleep(1000);
	}
	return 0;
}



