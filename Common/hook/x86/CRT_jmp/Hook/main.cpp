#include "Hook.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			CloseHandle(CreateThread(NULL, 0, MyHookAll, NULL, 0, NULL));
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			MyUnHookAll(NULL);
			break;
    }
    return TRUE;
}
