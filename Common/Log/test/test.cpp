// test.cpp : Defines the entry point for the console application.
//

#include "stdio.h"
#include "Log.h"

DWORD WINAPI ThreadProc(LPVOID lparam)
{
    int id = (int)lparam;
    int i=0;
    do 
    {
        LogPrint(TEXT("[%d]:%s\r\n"), id, "thread log");
        Sleep(500);
        i++;
    } while (i<10);
    return 0;
}

int main(int argc, char* argv[])
{
    SetLogFile(TEXT("C:\\log1.txt"));
    LogOn();
    HANDLE handle[2];

    handle[0] = CreateThread(NULL, 0, ThreadProc, (LPVOID)0, 0, NULL);
    handle[1] = CreateThread(NULL, 0, ThreadProc, (LPVOID)1, 0, NULL);

    WaitForMultipleObjects(2, handle, FALSE, INFINITE);

    for (int i=0; i<2; i++)
    {
        CloseHandle(handle[i]);
    }
    LogClose();

	return 0;
}
