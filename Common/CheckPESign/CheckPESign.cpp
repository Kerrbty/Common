#include "stdafx.h"
#include "CheckPESign.h"
#include "../AntiDebug/CheckProcess.h"
#include <windows.h>
#include <tchar.h>
#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>
#ifndef _DEBUG
#include "VMProtectSDK.h"
#endif
#pragma comment (lib, "wintrust")
#pragma comment (lib, "Crypt32.lib")

#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)
#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)
#define AllocMemory(_a)  HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _a)
#define FreeMemory(_a)   { if (_a) { HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, _a); _a=NULL; } }
#define DeleteHandle(_a) {if(_a != INVALID_HANDLE_VALUE) { CloseHandle(_a); _a = INVALID_HANDLE_VALUE;}}
#define FileOpen(_name, _openf, _sharef, _createf)  CreateFileA(_name, _openf, _sharef, NULL, _createf, FILE_ATTRIBUTE_NORMAL, NULL)

// 校验签名作者
BOOL PrintCertificateInfo(PCCERT_CONTEXT pCertContext, char* namebuf, int ilen)
{
    BOOL fReturn = FALSE;
    LPSTR szName = NULL;
    do 
    {
        __func_IDA7__();
        // Get Subject name size.
        DWORD dwData = 1024*sizeof(CHAR);
        szName = (LPSTR)AllocMemory(dwData);
        if (!(CertGetNameStringA(pCertContext,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            0,
            NULL,
            szName,
            dwData)))
        {
            break;
        }
        __func_IDA5__();

#ifndef _DEBUG
        VMProtectBegin("zxly_key1");
#endif
        if (strstr(szName, "掌星") != NULL)
        {
            strncpy(namebuf, szName, ilen);
            fReturn = TRUE;
        }
#ifndef _DEBUG
        VMProtectEnd();
#endif
    } while (FALSE);
    FreeMemory(szName);
    __func_IDA8__();
    return fReturn;
}

BOOL WINAPI GetCertName(LPCWSTR szFileName, char* namebuf, int ilen)
{
    BOOL GetNameState = FALSE;
    HCERTSTORE hStore = NULL;
    HCRYPTMSG hMsg = NULL;
    PCMSG_SIGNER_INFO pSignerInfo = NULL;

    __func_IDA6__();
    memset(namebuf, 0, ilen);
    do
    {
        __func_IDA5__();
        if ( !CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                            szFileName,
                            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
                            CERT_QUERY_FORMAT_FLAG_BINARY,
                            0,
                            NULL,
                            NULL,
                            NULL,
                            &hStore,
                            &hMsg,
                            NULL)
            )
        {
            return FALSE;
        }

        __func_IDA9__();
        DWORD dwSignerInfo = 4096;
        pSignerInfo = (PCMSG_SIGNER_INFO)AllocMemory(dwSignerInfo);
        if ( !CryptMsgGetParam( hMsg,
                                CMSG_SIGNER_INFO_PARAM,
                                0,
                                (PVOID)pSignerInfo,
                                &dwSignerInfo)
           )
        {
            break;
        }

        CERT_INFO CertInfo = {0};
        CertInfo.Issuer = pSignerInfo->Issuer;
        CertInfo.SerialNumber = pSignerInfo->SerialNumber;
        PCCERT_CONTEXT pCertContext = CertFindCertificateInStore(  hStore,
                                                    ENCODING,
                                                    0,
                                                    CERT_FIND_SUBJECT_CERT,
                                                    (PVOID)&CertInfo,
                                                    NULL);
        if (pCertContext)
        {
            if ( PrintCertificateInfo(pCertContext, namebuf, ilen) )
            {
                GetNameState = TRUE;
            }
            CertFreeCertificateContext(pCertContext);
        }        
    }while(FALSE);
    FreeMemory(pSignerInfo);
    CertCloseStore(hStore, 0);
    CryptMsgClose(hMsg);

    return GetNameState;
}


// 校验文件签名是否正确
BOOL WINAPI VerifyEmbeddedSignature(LPCWSTR pwszSourceFile, BOOL bCheckComparyName)
{
    __func_IDA7__();
    BOOL CheckInfo = FALSE;
    LONG lStatus;
    WINTRUST_FILE_INFO FileData;
    memset(&FileData, 0, sizeof(FileData));
    FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
    FileData.pcwszFilePath = pwszSourceFile;
    FileData.hFile = NULL;
    FileData.pgKnownSubject = NULL;
    GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_DATA WinTrustData;
    memset(&WinTrustData, 0, sizeof(WinTrustData));
    WinTrustData.cbStruct = sizeof(WinTrustData);
    WinTrustData.pPolicyCallbackData = NULL;
    WinTrustData.pSIPClientData = NULL;
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; 
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
    WinTrustData.dwStateAction = 0;
    WinTrustData.hWVTStateData = NULL;
    WinTrustData.pwszURLReference = NULL;
    WinTrustData.dwProvFlags = WTD_SAFER_FLAG;
    WinTrustData.dwUIContext = 0;
    WinTrustData.pFile = &FileData;
    __func_IDA11__();
    lStatus = WinVerifyTrust(
        NULL,
        &WVTPolicyGUID,
        &WinTrustData);
    if (ERROR_SUCCESS == lStatus) 
    {
        if (bCheckComparyName)
        {
            char* buf = (char*)AllocMemory(1024);
            if ( GetCertName(pwszSourceFile, buf, 200) )
            {
#ifndef _DEBUG
                VMProtectBegin("zxly_key2");
#endif
                if( strstr(buf, "掌星立意") != NULL )
                {
                    CheckInfo = TRUE;
                }
#ifndef _DEBUG
                VMProtectEnd();
#endif
            }
            __func_IDA10__();
            FreeMemory(buf);
        }
    }
    __func_IDA2__();
    return CheckInfo;
}

