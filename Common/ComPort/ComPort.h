#ifndef _H_HH_COMPORT_H_
#define _H_HH_COMPORT_H_
#include <windows.h>

/////////////////////////////////////////////////////////////////////////
//   ChgComPort  修改已有串口号 
//              dwSrc  原串口号 
//              dwDst  目标串口号 
//  return 
//        TRUE  成功， FALSE 失败  

BOOL ChgComPort(DWORD dwSrc, DWORD dwDst);


#endif  // _H_HH_COMPORT_H_
