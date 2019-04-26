#include "ComPort/ComPort.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc < 3 )
    {
        return EXIT_FAILURE;
    }

    ChgComPort(atoi(argv[1]), atoi(argv[2]));

    TCHAR Port[18] = {0};
    wsprintf(Port, TEXT("\\\\.\\COM%s"), argv[2]);
	HANDLE handle = CreateFile(Port, 
		GENERIC_READ, 
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		printf("Open %s error!\n", Port);
	}
	else
	{
		printf("open %s success!\n", Port);
		CloseHandle(handle);
	}

	return 0;
}