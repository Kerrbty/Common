#include "Base64.h"
#include "stdlib.h"
///Base64 编码表
const char base64_code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**************************************************
//
// 函数名称:		Base64_decode
// 函数功能:		Base64算法的解码函数
// 函数参数:		lpIndata:	密文数据	
// 函数参数:		nlen:		密文数据的长度
// 函数参数:		lpOutBuf:	解密数据缓冲区
// 函数返回值:		解码后的数据长度,如果返回零就表示解密失败
//
************************************************/
int Base64_decode(const char *lpIndata,int nInLen,unsigned char *lpOutBuf)
{
    static char base64_value[256] = {0};
	char *result = (char*)malloc(nInLen + 10);
	int j,i;
    int c;
    long val;
	int nret = 0;
	int sum = 0;
	if(!base64_value[0])
	{
		memset(base64_value,0xFF,256);
		for(i = 0;i < 64;i++)
		{
			base64_value[(int) base64_code[i]] = i;
		}
		base64_value['='] = 0;
	}
    
    if (!lpIndata)
		return 0;
    val = c = 0;
    for (j = 0,i = 0;i < nInLen;i++) {
		unsigned int k = ((unsigned char) *(lpIndata + i)) % 256;
		if (base64_value[k] < 0)
			continue;
		//记录末尾有几个'='号
		if( k == 0x3D)
		{
			sum++;
		}
		val <<= 6;
		val += base64_value[k];
		if (++c < 4)
			continue;
		/* One quantum of four encoding characters/24 bit */
		result[j++] = (val >> 16) & 0xff;	/* High 8 bits */
		result[j++] = (val >> 8) & 0xff;	/* Mid 8 bits */
		result[j++] = val & 0xff;	/* Low 8 bits */
		val = c = 0;
    }
	if (lpOutBuf != NULL)
	{
		lpOutBuf[j] = 0;
		memcpy(lpOutBuf,result,j);
	}
	free(result);
	nret = j - sum + 1;
    return nret;
}

/**************************************************
//
// 函数名称:		Base64_encode
// 函数功能:		Base64算法的编码函数
// 函数参数:		data:		明文数据	
// 函数参数:		nlen:		明文数据的长度
// 函数参数:		lpOutBuf:	编码数据缓冲区,如果缓冲区为NULL,那么就返回编码所需要的空间大小
// 函数参数:		nOutSize:	缓冲区大小
// 函数返回值:		编码后的数据长度,如果返回零就表示编码失败
//
************************************************/
int Base64_encode(const char *data, int len,char *lpOutBuf,int nOutSize)
{
//    static char result[BASE64_RESULT_SZ];

	if (!data)
		return 0;
	int malSize = ((len - 1) / 3 + 1) * 4 + 1;
	char *result = (char*)malloc(malSize);
	memset(result,0,malSize);
    int bits = 0;
    int char_count = 0;
    int out_cnt = 0;
	

    while (len--) {
		int c = (unsigned char) *data++;
		bits += c;
		char_count++;
		if (char_count == 3) {
			result[out_cnt++] = base64_code[bits >> 18];
			result[out_cnt++] = base64_code[(bits >> 12) & 0x3f];
			result[out_cnt++] = base64_code[(bits >> 6) & 0x3f];
			result[out_cnt++] = base64_code[bits & 0x3f];
			bits = 0;
			char_count = 0;
		} else {
			bits <<= 8;
		}
    }
    if (char_count != 0) {
		bits <<= 16 - (8 * char_count);
		result[out_cnt++] = base64_code[bits >> 18];
		result[out_cnt++] = base64_code[(bits >> 12) & 0x3f];
		if (char_count == 1) {
			result[out_cnt++] = '=';
			result[out_cnt++] = '=';
		} else {
			result[out_cnt++] = base64_code[(bits >> 6) & 0x3f];
			result[out_cnt++] = '=';
		}
    }
    result[out_cnt] = '\0';	/* terminate */
	if(!lpOutBuf)
	{
		free(result);
		return out_cnt;
	}
	if(nOutSize < out_cnt)
	{
		free(result);
		return 0;
	}
	memcpy(lpOutBuf,result,out_cnt);
	free(result);
    return out_cnt;
}