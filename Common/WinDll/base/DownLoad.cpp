#include <WinSock2.h>
#include <string>
#include <iostream>
#include <windows.h>
#include <winhttp.h> 
#include <Shlwapi.h>
#include <tchar.h>
#pragma comment(lib, "Shlwapi")
#pragma comment(lib,"winhttp.lib")
#pragma comment(lib, "ws2_32")


BOOL DownLoadFile(LPWSTR szHttpAddress, LPTSTR szSaveFile)
{
	BOOL retVal = FALSE;
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer = NULL;
	PWSTR szEnd = NULL;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	BOOL  bResults = FALSE;

	hSession=WinHttpOpen(L"User Agent", 
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 
		WINHTTP_NO_PROXY_NAME, 
		WINHTTP_NO_PROXY_BYPASS, 
		0);

	PWSTR szHost = new WCHAR[MAX_PATH];
	ZeroMemory(szHost, MAX_PATH*sizeof(WCHAR));
	PWSTR szStart = StrStrIW(szHttpAddress, L"//");
	if (szStart != NULL)
	{
		szEnd = StrStrIW(szStart+2, L"/");
		wcsncpy(szHost, szStart+2, szEnd-szStart-2);
	}
	LPWSTR port = StrStrIW(szHost, L":");
	if(hSession)
	{
		if (port != NULL)
		{
			port[0] = L'\0';
			DWORD dwport = _wtol(port+1);
			hConnect=WinHttpConnect(hSession, szHost, dwport,0);
		}
		else
		{
			hConnect=WinHttpConnect(hSession, szHost, INTERNET_DEFAULT_HTTP_PORT,0);
		}
		
	}
	delete []szHost;

	if(hConnect)
	{
		hRequest = WinHttpOpenRequest(hConnect, L"GET", szEnd, L"HTTP/1.1", WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,0);
	}

	if(hRequest)
	{
		bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0 );
	}

	if(bResults)
	{
		bResults = WinHttpReceiveResponse(hRequest, NULL);
	}

	// Ò»Æ¬Æ¬ÏÂÔØ
	if(bResults)
	{
		HANDLE handle = CreateFile(szSaveFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		do
		{
			if (handle == INVALID_HANDLE_VALUE)
			{
				break;
			}
			// Check for available data.
			dwSize = 0;

			if (!WinHttpQueryDataAvailable( hRequest, &dwSize))
			{
				printf( "Error %u in WinHttpQueryDataAvailable.\n",GetLastError());
				break;
			}

			if (!dwSize)
				break;

			pszOutBuffer = new char[dwSize+1];
			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				break;
			}

			ZeroMemory(pszOutBuffer, dwSize+1);
			if (!WinHttpReadData( hRequest, (LPVOID)pszOutBuffer,  dwSize, &dwDownloaded))
			{
				retVal = FALSE;
			}
			else
			{
				retVal = TRUE;
				DWORD dwBytes = 0;
				WriteFile(handle, pszOutBuffer, dwSize, &dwBytes, NULL);
			}
			delete [] pszOutBuffer;

			if (!dwDownloaded)
				break;
		} while (dwSize > 0);
		CloseHandle(handle);
	}

	if (hRequest) 
		WinHttpCloseHandle(hRequest);
	if (hConnect) 
		WinHttpCloseHandle(hConnect);
	if (hSession) 
		WinHttpCloseHandle(hSession);
	return retVal;
}


BOOL DownLoadFile2(LPTSTR szHttpAddress, LPTSTR szSaveFile)
{
	PTCHAR szEnd = NULL;
	PTSTR szHost = new TCHAR[MAX_PATH];

	ZeroMemory(szHost, MAX_PATH*sizeof(TCHAR));
	PTSTR szStart = StrStrIW(szHttpAddress, L"//");
	if (szStart != NULL)
	{
		szEnd = StrStrIW(szStart+2, L"/");
		wcsncpy(szHost, szStart+2, szEnd-szStart-2);
	}
	LPTSTR port = StrStrIW(szHost, L":");
	return FALSE;
}

void main()
{
	if ( DownLoadFile2(L"http://61.160.248.69:901/RouteFiles/LA_root.bin", TEXT("C:\\1.bin")) )
	{
		printf("Download Success!\n");
	}
}