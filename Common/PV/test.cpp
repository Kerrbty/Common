#include <stdio.h>
#include "semaphore.h"
#include "lock.h"

CSemaphore* empty;
CSemaphore* full;
DWORD idcount = 0;
CLock datalock; 

DWORD WINAPI Customer(LPVOID lparam)
{
    DWORD id = (DWORD)lparam;
    while (true)
    {
        empty->P();
        datalock.lock();
        idcount--;
        printf("sub 消费者%d 消费了一个，-目前还有%d个\n", id, idcount);
        datalock.unlock();
        full->V();
        Sleep(1000);
    }
}


DWORD WINAPI Producer(LPVOID lparam)
{
    DWORD id = (DWORD)lparam;
    while (true)
    {
        full->P();
        datalock.lock();
        idcount++;
        printf("add 生成者%d 生产了一个，+目前有%d个\n", id, idcount);
        datalock.unlock();
        empty->V();
        Sleep(1000);
    }
}


int _tmain(int argc, _TCHAR* argv[])
{
    empty = new CSemaphore(5, 5);
    full = new CSemaphore(5, 0);

    HANDLE handle[10];
    for (int i=0; i<5; i++)
    {
        handle[i] = CreateThread(NULL, 0, Customer, (LPVOID)i, 0, NULL);
        handle[i+5] = CreateThread(NULL, 0, Producer, (LPVOID)i, 0, NULL);
    }
    
    WaitForMultipleObjects(10, handle, TRUE, INFINITE);
	return 0;
}

