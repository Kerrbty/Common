#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <Wincrypt.h>
#pragma comment(lib, "Advapi32")

BOOL CalcStringMD5(PBYTE pbuf, DWORD dwlen, DWORD MD5[4])
{
	BOOL retval = FALSE;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	DWORD error = 0;
	do
	{
		if( !CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) ) 
		{
			break;
		}
		CryptCreateHash(hProv, CALG_MD5, NULL, 0, &hHash);
		DWORD ilen = 4*sizeof(DWORD);
		if( CryptHashData(hHash, pbuf, dwlen, 0) && 
			CryptGetHashParam(hHash, HP_HASHVAL, (PBYTE)MD5, &ilen, 0) )
		{
			retval = TRUE;
		}
	}while(0);
	
	if(hHash) 
		CryptDestroyHash(hHash);
	if(hProv) 
		CryptReleaseContext(hProv, 0);
	return retval;
}

BOOL CalcFileMD5(PTCHAR szFileName, DWORD MD5[4])
{
	  BOOL retval = FALSE;
	  HCRYPTPROV hProv = 0;
	  HCRYPTHASH hHash = 0;
	  DWORD error = 0;
	  do
	  {
		  if( !CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) ) 
		  {
			  break;
		  }
		  CryptCreateHash(hProv, CALG_MD5, NULL, 0, &hHash);
		  
		  // Open the file to calc
		  HANDLE handle = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		  if (handle == INVALID_HANDLE_VALUE)
		  {
			  break;
		  }
		  
		  DWORD dwFileSize = GetFileSize(handle, NULL);
		  PBYTE pbuf = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 20*1024*1024); // Alloc the 20M memory to read file
		  DWORD dwByte = 0;
		  while ( ReadFile(handle, pbuf, 20*1024*1024, &dwByte, NULL) && dwByte != 0 )
		  {
			  CryptHashData(hHash, pbuf, dwByte, 0);
		  }
		  HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pbuf);
		  CloseHandle(handle);

		  // get the md5 hash 
		  DWORD ilen = 4*sizeof(DWORD);
		  if( CryptGetHashParam(hHash, HP_HASHVAL, (PBYTE)MD5, &ilen, 0) )
		  {
			  retval = TRUE;
		  }
	  }while(0);

	  if(hHash) 
		  CryptDestroyHash(hHash);
	  if(hProv) 
		  CryptReleaseContext(hProv, 0);
	  return retval;
}

int main()
{
	DWORD val[4];
	CalcFileMD5(TEXT("D:\\Windows764.GHO"), val);
	PBYTE pbuf = (PBYTE)val;
	for (int i=0; i<16; i++)
	{
		printf("%02X", pbuf[i]);
	}
	printf("\n");
	return 0;
}
