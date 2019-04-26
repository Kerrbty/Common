/* gzip.h -- common declarations for all gzip modules
 * Copyright (C) 1992-1993 Jean-loup Gailly.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License, see the file COPYING.
 */

/* I don't like nested includes, but the string and io functions are used
 * too often
 */
#include <stdio.h>
#include <string.h>

#define OF(args)  args

typedef char * voidp;

#define memzero(s, n)     memset ((voidp)(s), 0, (n))

#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif

#define local //static

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned int  ulg;

/* Return codes from gzip */

/* Compression methods (see algorithm.doc) */
#define STORED      0 /* stored */
#define COMPRESSED  1 /* compressed */
#define PACKED      2 /* packed */
#define LZHED       3 /* lzwed */
/* methods 4 to 7 reserved */
#define DEFLATED    8 /* ȱʡ */
#define MAX_METHODS 9 /* ���ѹ�� */
extern int method;         /* compression method */

/* To save memory for 16 bit systems, some arrays are overlaid between
 * the various modules:
 * deflate:  prev+head   window      d_buf  l_buf  outbuf
 * unlzw:    tab_prefix  tab_suffix  stack  inbuf  outbuf
 * inflate:              window             inbuf
 * unpack:               window             inbuf  prefix_len
 * unlzh:    left+right  window      c_table inbuf c_len
 * For compression, input is done in window[]. For decompression, output
 * is done in window except for unlzw.
 */
#define INBUFSIZ  0x2000  /* input buffer size */
#define INBUF_EXTRA  64     /* required by unlzw() */
#define OUTBUFSIZ   8192  /* output buffer size */

#define OUTBUF_EXTRA 2048   /* required by unlzw() */
#define DIST_BUFSIZE 0x2000 /* buffer for distances, see trees.c */

#define near

#define EXTERN(type, array)  extern type * near array
#define DECLARE(type, array, size)  type * near array

#define fcalloc(items,size) malloc((size_t)(items)*(size_t)(size))
#define fcfree(ptr) free(ptr)
#define ALLOC(type, array, size) { \
      array = (type*)fcalloc((size_t)(((size)+1L)/2), 2*sizeof(type));/* malloc */ \
      if (array == NULL) error("insufficient memory"); \
   }
#define FREE(array) {if (array != NULL) fcfree(array), array=NULL;}

EXTERN( uch, inbuf );          /* input buffer */
EXTERN( uch, outbuf );         /* output buffer */
EXTERN( ush, d_buf );          /* buffer for distances, see trees.c */
EXTERN( uch, window );         /* Sliding window and suffix table (unlzw) */
#define tab_suffix window
#define tab_prefix prev    /* hash link (see deflate.c) */
#define head (prev+WSIZE)  /* hash head (see deflate.c) */
EXTERN( ush, tab_prefix );  /* prefix code (see unlzw.c) */

extern unsigned insize; /* valid bytes in inbuf */
extern unsigned inptr;  /* index of next byte to be processed in inbuf */
extern unsigned outcnt; /* bytes in output buffer */

extern int bytes_in;   /* number of input bytes */
extern int bytes_out;  /* number of output bytes */
extern int header_bytes;/* number of bytes in gzip header */

#define isize bytes_in
/* for compatibility with old zip sources (to be cleaned) */

extern int time_stamp; /* original time stamp (modification time) */

#define PACK_MAGIC     "\037\036" /* Magic header for packed files */
#define GZIP_MAGIC     "\037\213" /* Magic header for gzip files, 1F 8B */
#define OLD_GZIP_MAGIC "\037\236" /* Magic header for gzip 0.5 = freeze 1.x */
#define LZH_MAGIC      "\037\240" /* Magic header for SCO LZH Compress files*/
#define PKZIP_MAGIC    "\120\113\003\004" /* Magic header for pkzip files */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

/* internal file attribute */
#define UNKNOWN 0xffff /* δ֪ */
#define BINARY  0 /* binary */
#define ASCII   1 /* ascII */

#ifndef WSIZE
#define WSIZE 0x8000     /* window size--must be a power of two, and */
#endif                     /*  at least 32K for zip's deflate method */

#define MIN_MATCH  3 /* min match */
#define MAX_MATCH  258 /* max match*/
/* The minimum and maximum match lengths */

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

extern int exit_code;      /* program exit code */
extern int quiet;          /* be quiet (-q) */
extern int level;          /* compression level */

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())
#define try_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())

/* put_byte is used for the compressed output, put_ubyte for the
 * uncompressed output. However unlzw() uses window for its
 * suffix table instead of its output buffer, so it does not use put_ubyte
 * (to be cleaned up).
 */
#define put_byte(c) {outbuf[outcnt++]=(uch)(c); if (outcnt==OUTBUFSIZ)\
   flush_outbuf();}
#define put_ubyte(c) {window[outcnt++]=(uch)(c); if (outcnt==WSIZE)\
   flush_window();}

/* Output a 16 bit value, lsb first */
#define put_short(w) \
{ if (outcnt < OUTBUFSIZ-2) { /* compare */\
    outbuf[outcnt++] = (uch) ((w) & 0xff);/* ��λ */ \
    outbuf[outcnt++] = (uch) ((ush)(w) >> 8);/* ��λ */ \
  } else { \
    put_byte((uch)((w) & 0xff)); /* ��λ */\
    put_byte((uch)((ush)(w) >> 8));/* ��λ */ \
  } \
}

/* Output a 32 bit value to the bit stream, lsb first */
#define put_long(n) { \
    put_short((n) & 0xffff);/* ��λ */ \
    put_short(((ulg)(n)) >> 16); /* ��λ */\
}

#define seekable()    0  /* force sequential output */
#define translate_eol 0  /* no option -a yet */

/* Macros for getting two-byte and four-byte header values */
#define SH(p) ((ush)(uch)((p)[0]) | ((ush)(uch)((p)[1]) << 8))
/* Macros for getting two-byte and four-byte header values */
#define LG(p) ((ulg)(SH(p)) | ((ulg)(SH((p)+2)) << 16))

/* Diagnostic functions */
#define Assert(cond,msg)
#define Trace(x)
#define Tracev(x)
#define Tracevv(x)
#define Tracec(c,x)
#define Tracecv(c,x)

#define WARN(msg) {if (!quiet) fprintf msg ; \
           if (exit_code == OK) exit_code = WARNING;}

extern unsigned char * zip_mem_inptr; /* ���뻺�� */
extern int zip_mem_insize; /* ���뻺�泤�� */
extern int zip_mem_inpos; /* �Ѿ�ʹ�õ�λ�� */

extern char * unzip_mem_inptr ;
extern int unzip_mem_insize ;
extern int unzip_mem_inpos;

extern char * zip_mem_outptr;
extern int zip_mem_outlen ;

/* in zip.c: */
extern int zip        OF(( void ));
extern int mem_read   OF(( char * buf,  unsigned size ));
/* in unzip.c */
extern int unzip      OF(( void ));

/* in deflate.c */
void lm_init OF(( int pack_level, ush * flags ));
ulg  deflate OF(( void ));

/* in trees.c */
void ct_init     OF(( ush * attr, int * method ));
int  ct_tally    OF(( int dist, int lc ));
ulg  flush_block OF(( char * buf, ulg stored_len, int eof ));

/* in bits.c */
void     bi_init    OF(());
void     send_bits  OF(( int value, int length ));
unsigned bi_reverse OF(( unsigned value, int length ));
void     bi_windup  OF(( void ));
void     copy_block OF(( char * buf, unsigned len, int header ));
extern   int (* read_buf ) OF(( char * buf, unsigned size ));

/* in util.c: */
extern ulg  updcrc        OF(( uch * s, unsigned n ));
extern void clear_bufs    OF(( void ));
extern int  fill_inbuf    OF(( void ));
extern void flush_outbuf  OF(( void ));
extern void flush_window  OF(( void ));
extern void write_buf     OF(( voidp buf, unsigned cnt ));
extern void error         OF(( char * m ));
extern void warn          OF(( char * a, char * b ));
extern void read_error    OF(( void ));
extern void write_error   OF(( void ));
extern voidp xmalloc      OF(( unsigned int size ));

/* in inflate.c */
extern int inflate OF(( void ));
