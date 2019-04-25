#ifndef MD5_H
#define MD5_H

void* calcMD5(void* inBuf, int inlen, void* outBuf, int outlen);

char* calcMd5toLowerCase(char* szstr, char* outbuf, size_t len);
char* calcMd5toUpperCase(char* szstr, char* outbuf, size_t len);
char* FiveMd5Calc(char* szstr, char* outbuf, size_t len);


#endif
