#pragma once
#ifndef _BASE64_H_
#define _BASE64_H_


#include <stdio.h>
#include <windows.h>


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
int Base64_decode(const char *lpIndata,int nInLen,unsigned char *lpOutBuf);

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
int Base64_encode(const char *data, int len,char *lpOutBuf,int nOutSize);

#endif // _BASE64_H_