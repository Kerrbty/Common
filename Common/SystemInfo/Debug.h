#ifndef _DEBUG_INFO_HEADER_HH_H_
#define _DEBUG_INFO_HEADER_HH_H_

#ifdef _DEBUG
int  _dbgprint(const char *format, ...);
#else 
#define _dbgprint(...) 
#endif


#endif // _DEBUG_INFO_HEADER_HH_H_