#ifndef _LGJ_HANGZHOU_MY_USER32_
#define _LGJ_HANGZHOU_MY_USER32_
#include <windows.h>
#define	LWA_COLORKEY	0x00000001
#define	LWA_ALPHA		0x00000002
#define WS_EX_LAYERED 	0x00080000

#pragma comment(lib,"myuser32.lib")

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if _MSC_VER < 1300
LONG WINAPI SetLayeredWindowAttributes(   HWND  hWnd,
                                          int   cr,
                                          UCHAR bAlpha,
                                          DWORD dwFlags);
	
BOOL WINAPI AnimateWindow(  HWND hwnd,     // handle to window
							DWORD dwTime,  // duration of animation  
							DWORD dwFlags  // animation type
							);

#define AW_HOR_POSITIVE 0x00000001
#define AW_SLIDE  0x00040000
	
#endif
//  SystemParametersInfoA(0x14, 4, "F:\\LGJ\\GRMWDK_EN_7600_1\\splash.bmp", 1);
// DWORD WINAPI SystemParametersInfoA(DWORD, DWORD, LPSTR, DWORD); // 设置壁纸
// DWORD WINAPI SystemParametersInfoW(DWORD, DWORD, LPWSTR, DWORD); // 设置壁纸
	
// 在宽字节与窄字节中定义统一接口
#ifdef _UNICODE

#define SystemParametersInfo SystemParametersInfoW

#else

#define SystemParametersInfo SystemParametersInfoA

#endif
	
	
	
	
	
	
	
	
#ifdef __cplusplus
};
#endif // __cplusplus

#endif // _LGJ_HANGZHOU_MY_USER32_