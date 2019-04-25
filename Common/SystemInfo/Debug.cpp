#include "Debug.h"
#include <stdio.h>
#include <varargs.h>
#include <stdarg.h>

#ifdef _DEBUG
int  _dbgprint(const char *format, ...)
{
    va_list args;
    FILE* fp = fopen("C:\\1.log", "a+");  

    va_start(args, format);
    int ilen = vfprintf(fp, format, args);
    va_end(args);

    fclose(fp);
    return ilen;
}
#endif