#pragma once

#include <windows.h>
#include <tchar.h>



#pragma pack(push,1)

typedef struct _mac_header{
    BYTE   dst_mac[6];
    BYTE   src_mac[6];
    USHORT pack_type; // type, 0x0800 表示IP协议
}mac_header, *pmac_header;

typedef struct _icmp_header{
    BYTE   icmp_type; // 消息类型
    BYTE   icmp_code; // 代码
    USHORT icmp_checksum; //

    // 下面是回显头
    USHORT icmp_id;   // 用来唯一标志此请求的dID号，通常设置为进程ID
    USHORT icmp_sequence;  // 序列号
    DWORD32 icmp_timestamp; // 时间戳
}icmp_header, *picmp_header;

typedef struct _ip_header{
    BYTE   ip_version:4; // 4 -- ipv4, 6 -- ipv6
    BYTE   ip_header_lenth;  // IHL << 2 才是真正的长度,一般为5
    BYTE   ip_tos;     // 服务类型
    USHORT ip_lenth;   // 除去mac_header,整个包的长度
    USHORT ip_id;  // 封包标识，唯一标识发送的每一个数据报
    USHORT ip_flags; // ipv4封包被分割成更小包的标识，0x01 MF 更多的分割， 0x02 DF 包不能分割
    BYTE   ip_ttl;  // 生存时间，ttl
    BYTE   ip_protocol; // 协议，6 代表TCP，17 代表UDP，ICMP
    USHORT ip_checksum; // 校验和
    DWORD  src_ip;     // 源IP地址 
    DWORD  dst_ip;     // 目的IP地址
}ip_header, *pip_header;

typedef struct _udp_header{
    USHORT  src_port;  // 源端口号 
    USHORT  dst_port;  // 目的端口号
    USHORT  len;  // 封包长度
    USHORT  checksum; // 校验和 
}udp_header, *pudp_header;


typedef struct _tcp_header{
    USHORT  src_port;
    USHORT  dst_port;
    DWORD   sequence_number; // 32位序列号
    DWORD   acknowledge_number; // ack确认号
    BYTE    data_offset;  // 正文数据偏移
    BYTE    flags;       //
    USHORT  windows;     //
    USHORT  checksum; 
    USHORT  urgentPointer;  // 16位紧急数据便宜
}tcp_header, *ptcp_header;


#pragma pack(pop)


unsigned short checksum(unsigned short *buffer, int size);


