#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#pragma comment(lib, "OCR.lib")


int WINAPI VcodeInit(char* init_val = "3FDAD7256047B3344A0CE00685222917");
BOOL WINAPI GetVcode(int id, unsigned char* buf, size_t size, unsigned char* out_buf);



#ifdef __cplusplus
};
#endif