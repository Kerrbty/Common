#ifndef _CHECK_PE_FILE_SIGN_INFO_HEAGER_
#define _CHECK_PE_FILE_SIGN_INFO_HEAGER_
#ifdef __cplusplus
extern "C" {
#endif
#include <windows.h>

// У���ļ�ǩ���Ƿ���ȷ����ֹ�ļ��۸ģ�
// У��ǩ����˾�Ƿ�Ϊ "��������"
BOOL WINAPI VerifyEmbeddedSignature(LPCWSTR pwszSourceFile, BOOL bCheckComparyName);

// �õ�ǩ���ļ��Ĺ�˾��
BOOL WINAPI GetCertName(LPCWSTR szFileName, char* namebuf, int ilen);


#ifdef __cplusplus
};
#endif
#endif