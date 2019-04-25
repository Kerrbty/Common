#pragma once

#include <windows.h>
#include <tchar.h>



#pragma pack(push,1)

typedef struct _mac_header{
    BYTE   dst_mac[6];
    BYTE   src_mac[6];
    USHORT pack_type; // type, 0x0800 ��ʾIPЭ��
}mac_header, *pmac_header;

typedef struct _icmp_header{
    BYTE   icmp_type; // ��Ϣ����
    BYTE   icmp_code; // ����
    USHORT icmp_checksum; //

    // �����ǻ���ͷ
    USHORT icmp_id;   // ����Ψһ��־�������dID�ţ�ͨ������Ϊ����ID
    USHORT icmp_sequence;  // ���к�
    DWORD32 icmp_timestamp; // ʱ���
}icmp_header, *picmp_header;

typedef struct _ip_header{
    BYTE   ip_version:4; // 4 -- ipv4, 6 -- ipv6
    BYTE   ip_header_lenth;  // IHL << 2 ���������ĳ���,һ��Ϊ5
    BYTE   ip_tos;     // ��������
    USHORT ip_lenth;   // ��ȥmac_header,�������ĳ���
    USHORT ip_id;  // �����ʶ��Ψһ��ʶ���͵�ÿһ�����ݱ�
    USHORT ip_flags; // ipv4������ָ�ɸ�С���ı�ʶ��0x01 MF ����ķָ 0x02 DF �����ָܷ�
    BYTE   ip_ttl;  // ����ʱ�䣬ttl
    BYTE   ip_protocol; // Э�飬6 ����TCP��17 ����UDP��ICMP
    USHORT ip_checksum; // У���
    DWORD  src_ip;     // ԴIP��ַ 
    DWORD  dst_ip;     // Ŀ��IP��ַ
}ip_header, *pip_header;

typedef struct _udp_header{
    USHORT  src_port;  // Դ�˿ں� 
    USHORT  dst_port;  // Ŀ�Ķ˿ں�
    USHORT  len;  // �������
    USHORT  checksum; // У��� 
}udp_header, *pudp_header;


typedef struct _tcp_header{
    USHORT  src_port;
    USHORT  dst_port;
    DWORD   sequence_number; // 32λ���к�
    DWORD   acknowledge_number; // ackȷ�Ϻ�
    BYTE    data_offset;  // ��������ƫ��
    BYTE    flags;       //
    USHORT  windows;     //
    USHORT  checksum; 
    USHORT  urgentPointer;  // 16λ�������ݱ���
}tcp_header, *ptcp_header;


#pragma pack(pop)


unsigned short checksum(unsigned short *buffer, int size);


