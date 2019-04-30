#ifndef MD5_H
#define MD5_H

void* calcMD5(void* inBuf, int inlen, void* outBuf, int outlen);

char* calcMd5toLowerCase(char* szstr, unsigned long inlen, char* outbuf, size_t outlen);
char* calcMd5toUpperCase(char* szstr, unsigned long inlen, char* outbuf, size_t outlen);
char* FiveMd5Calc(char* szstr, unsigned long inlen, char* outbuf, size_t outlen);


#endif
