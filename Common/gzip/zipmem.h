/**************************************************************
** Copyright (C) 2002-2007 基础技术开发部
** All right reserved
***************************************************************
**
** 项目名称：内存压缩与解压缩
** 功能简介：内存压缩与解压缩
**
** 原始作者：段太钢  Email：duantaigang@xjgc.com
** 组织部门：许继集团 许继电气技术中心嵌入式软件开发部
** 备    注：
** 建立时间：2007-10-22 17:11
** 完成时间：2007-10-22 17:11
** 版    本：1.0
***************************************************************
** 单元标识：$Id: zipmem.h,v 1.2 2007/11/19 06:16:06 dtg Exp $
** 版本历史：
** 修 改 者：
** 最近修改：
**************************************************************/
#ifndef _ZIP_MEM_H_
#define _ZIP_MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
	函数功能: 压缩内存,压缩后的数据存放在mem_outptr所指向的空间
	参数说明:
		mem_inptr: 待压缩的内存指针，由用户提供缓存空间
		mem_insize: 待压缩的内存长度
		mem_outptr: 压缩后的内存指针, 由用户提供缓存空间
	返回值:
	    压缩后的内存长度
*/
int zipmem( char * mem_inptr, int mem_insize, char * mem_outptr );

/*
	函数功能: 解压缩, 解压缩后的数据存放在mem_outptr所指向的空间
	参数说明:
		mem_inptr: 压缩后的内存指针，由用户提供缓存空间
		mem_insize: 压缩后的内存长度
		mem_outptr: 解压缩后的内存指针, 由用户提供缓存空间
	返回值:
	    解压缩的内存长度
*/
int unzipmem( char * mem_inptr, int mem_insize, char * mem_outptr );

#ifdef __cplusplus
};
#endif

#endif
