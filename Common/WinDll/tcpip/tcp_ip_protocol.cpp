#include "tcp_ip_protocol.h"


// clac the header's check sum  
unsigned short checksum(unsigned short *buffer, int size)     
{     
    unsigned long cksum=0;     
    while(size > 1)     
    {     
        cksum += *buffer++;  
        size -= sizeof(unsigned short);     
    }     
    if(size)  
    {     
        cksum += *(unsigned char*)buffer;    
    }     
    cksum = (cksum>>16) + (cksum & 0xffff);     
    cksum += (cksum>>16);     
    return  (unsigned short)(~cksum);     
}