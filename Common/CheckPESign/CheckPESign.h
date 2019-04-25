#ifndef _CHECK_PE_FILE_SIGN_INFO_HEAGER_
#define _CHECK_PE_FILE_SIGN_INFO_HEAGER_
#ifdef __cplusplus
extern "C" {
#endif
#include <windows.h>

// 校验文件签名是否正确（防止文件篡改）
// 校验签名公司是否为 "掌星立意"
BOOL WINAPI VerifyEmbeddedSignature(LPCWSTR pwszSourceFile, BOOL bCheckComparyName);

// 得到签名文件的公司名
BOOL WINAPI GetCertName(LPCWSTR szFileName, char* namebuf, int ilen);


#ifdef __cplusplus
};
#endif
#endif