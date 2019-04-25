#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <fstream>
#include <iostream>
#include <string>
#pragma comment(lib, "wininet")

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")


// index.htm
char szHomePage[] = "/index.htm";

// get the path
PCH GetThePath(PCH szbuf)
{
	if (memicmp(szbuf, "get", 3) == 0)
	{
		szbuf += 3;
		while(*szbuf == 0x20)
		{
			szbuf++;
		}
		if (memicmp(szbuf, "\\ ", 2) == 0 
			|| memicmp(szbuf, "/ ", 2) == 0 )
		{
			return szHomePage;
		}
		return szbuf;
	}
	return szHomePage;
}

// Change the buf string / to \  
PCH ChageSignbal(PCH szstr, char SrcCh, char DstCh)
{
	PCH retval = szstr;
	for (; *szstr != NULL; szstr++)
	{
		if (*szstr == SrcCh)
		{
			*szstr = DstCh;
		}
	}
	return retval;
}

PCH strmycat(PCH szdst, PCH szsrc, char ispace)
{
	PCH retval = szdst;
	while(*szdst != NULL)
	{
		szdst++;
	}
	while(*szsrc != ispace &&
		  *szsrc != NULL)
	{
		 *szdst++ = *szsrc++;
	}
	return retval ;
}


unsigned char ToByte(const char* str)
{
	unsigned char value = 0;
	for (int i=4; i>=0; i -= 4, str++)
	{
		if (*str <= '9')
		{
			value |= ((unsigned char)(*str - '0')<<i);
		}
		else
		{
			value |= ((unsigned char)((*str|0x20) - 'a' + 10)<<i);
		}
	}
	return value;
}

char* DecryptURL(char* DesURL, const char* SorURL, size_t size)
{
	size_t i=0;
	ZeroMemory(DesURL, size);
	for (;*SorURL!=NULL && i<size ;i++)
	{
		if (*SorURL == '%')
		{
			DesURL[i] = ToByte(SorURL+1);
			SorURL += 3;
		}
		else
		{
			DesURL[i] = *SorURL;
			SorURL++;
		}
	}
	return DesURL;
}


PCHAR UTF8ToANSI( PCHAR str , size_t size)
{
	WCHAR*  pElementText;
	int    iTextLen;
	// wide char to multi char
	iTextLen = MultiByteToWideChar( CP_UTF8,
		0,
		str,
		-1,
		NULL,
		0 );
	
	pElementText = new WCHAR[iTextLen + 4];
	memset( ( void* )pElementText, 0, sizeof( WCHAR ) * ( iTextLen + 1 ) );
	MultiByteToWideChar( CP_UTF8,
		0,
		str,
		-1,
		pElementText,
		iTextLen );
	
	WideCharToMultiByte( CP_ACP,
		0,
		pElementText,
		-1,
		str,
		size,
		NULL,
		NULL );
	delete pElementText;
	return (PCHAR)str;
}

// LPTHREAD_START_ROUTINE
DWORD CALLBACK ReportHtml(LPVOID lpThreadParameter)
{
	try
	{

		SOCKET se = (SOCKET)lpThreadParameter;
		
		char FilePath[MAX_PATH+8] = {0};
		GetCurrentDirectoryA(MAX_PATH, FilePath);
		strcat(FilePath, "\\www");
		char* buf=new char[2080];
		ZeroMemory(buf, 2080);
		recv(se, buf, 2048, 0);
//		printf("%s\n", buf);
		
		strmycat(FilePath, GetThePath(buf), 0x20);
		ChageSignbal(FilePath, '/', '\\');
		DecryptURL(buf, FilePath, 2048);
		UTF8ToANSI(buf, 2048);
		
		HANDLE pfhand = CreateFileA(buf,   
			GENERIC_READ|GENERIC_WRITE,  
			FILE_SHARE_READ|FILE_SHARE_WRITE,  
			NULL,  
			OPEN_EXISTING,  
			0,  
			NULL); 
		if (INVALID_HANDLE_VALUE != pfhand)
		{
			DWORD dwBytes = 0;
			while(ReadFile(pfhand, buf, 2048, &dwBytes, NULL) && dwBytes>0)
			{
				send(se, buf, dwBytes, 0);
			}
			CloseHandle(pfhand);
		}
		DWORD err = GetLastError();
		
		closesocket(se);
		delete []buf;
	}
	catch (...)
	{
		exit(0);
	}
	return 0;
}


int main(int argc, char* argv[])
{
	try
	{
		WSADATA WsData; 
		WSAStartup(0x0101, &WsData);
		SOCKET ss = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if ( INVALID_SOCKET == ss )
		{
			printf("socket call error: %d\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		
		struct sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_port = ntohs(80);
		addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		if (
			SOCKET_ERROR == 
			bind(ss, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) 
			)
		{
			printf("bind call error: %d\n", WSAGetLastError());
			closesocket(ss);
			WSACleanup();
			return 1;
		}
		
		if (SOCKET_ERROR == listen(ss, 5))
		{
			printf("listen call error: %d\n", WSAGetLastError());
			closesocket(ss);
			WSACleanup();
			return 1;
		}
		
		while(1)
		{
			struct sockaddr so = {0};
			int lenth = sizeof(SOCKADDR);
			SOCKET se = accept(ss, (struct sockaddr FAR *)&so, &lenth);
			HANDLE Thread = CreateThread(NULL, 0, ReportHtml, (LPVOID)(se), 0, NULL);
			CloseHandle(Thread);
		}
		
		WSACleanup();
	}
	catch (...)
	{
		exit(0);
	}
	return 0;
}