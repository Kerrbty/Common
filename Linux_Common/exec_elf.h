#ifndef _SYS_EXEC_ELF_H_
#define _SYS_EXEC_ELF_H_




//////////////////////////////////////////////////////////////////////////
// 
//        Elf file format
//  
//   ┌──────────────────────┐ 
//   │                      ∣ 
//   │     ELF header       ∣   Elf32_Ehdr or Elf64_Ehdr
//   │                      ∣ 
//   ├──────────────────────┤ 
//   │                      ∣ 
//   │ Program header table ∣   
//   │                      ∣ 
//   ├──────────────────────┤ 
//   │                      ∣ 
//   │      Section 1       ∣ 
//   │                      ∣ 
//   ├──────────────────────┤ 
//   │                      ∣ 
//   │      Section...      ∣ 
//   │                      ∣ 
//   ├──────────────────────┤ 
//   │                      ∣ 
//   │      Section N       ∣ 
//   │                      ∣ 
//   ├──────────────────────┤ 
//   │                      ∣ 
//   │ Section header table ∣ 
//   │                      ∣ 
//   └──────────────────────┘ 
//   
//////////////////////////////////////////////////////////////////////////



#pragma pack(push, 1)

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;

typedef uint8_t        Elf_Byte;

typedef uint32_t    Elf32_Addr;
#define ELF32_FSZ_ADDR    4
typedef uint32_t    Elf32_Off;
typedef int32_t     Elf32_SOff;
#define ELF32_FSZ_OFF     4
typedef int32_t     Elf32_Sword;
#define ELF32_FSZ_SWORD   4
typedef uint32_t    Elf32_Word;
#define ELF32_FSZ_WORD    4
typedef uint16_t    Elf32_Half;
#define ELF32_FSZ_HALF    2
typedef uint64_t    Elf32_Lword;
#define ELF32_FSZ_LWORD 8

typedef uint64_t    Elf64_Addr;
#define ELF64_FSZ_ADDR    8
typedef uint64_t    Elf64_Off;
typedef int64_t     Elf64_SOff;
#define ELF64_FSZ_OFF    8
typedef int32_t     Elf64_Shalf;
#define ELF64_FSZ_SHALF 4

typedef int32_t     Elf64_Sword;
#define ELF64_FSZ_SWORD 4
typedef uint32_t    Elf64_Word;
#define ELF64_FSZ_WORD    4

typedef int64_t     Elf64_Sxword;
#define ELF64_FSZ_SXWORD 8
typedef uint64_t    Elf64_Xword;
#define ELF64_FSZ_XWORD 8
typedef uint64_t    Elf64_Lword;
#define ELF64_FSZ_LWORD 8
typedef uint32_t    Elf64_Half;
#define ELF64_FSZ_HALF 2

typedef uint16_t	Elf64_Quarter;

/*
 * ELF Header
 */
#define ELF_NIDENT    16
#define ELF_MAGIC     'FLE\x7F'

/* ei_magic */
#define ELFMAG0        0x7f
#define ELFMAG1        'E'
#define ELFMAG2        'L'
#define ELFMAG3        'F'
#define ELFMAG        "\177ELF"
#define SELFMAG        4


/* ei_class */
#define ELFCLASSNONE    0    /* Invalid class */
#define ELFCLASS32      1    /* 32-bit objects */
#define ELFCLASS64      2    /* 64-bit objects */
#define ELFCLASSNUM     3

/* ei_data */
#define ELFDATANONE     0    /* Invalid data encoding */
#define ELFDATA2LSB     1    /* 2's complement values, LSB first */
#define ELFDATA2MSB     2    /* 2's complement values, MSB first */

/* ei_version */
#define EV_NONE        0    /* Invalid version */
#define EV_CURRENT     1    /* Current version */
#define EV_NUM         2

/* ei_osabi */
#define ELFOSABI_SYSV          0    /* UNIX System V ABI */
#define ELFOSABI_HPUX          1    /* HP-UX operating system */
#define ELFOSABI_NETBSD        2    /* NetBSD */
#define ELFOSABI_LINUX         3    /* GNU/Linux */
#define ELFOSABI_HURD          4    /* GNU/Hurd */
#define ELFOSABI_86OPEN        5    /* 86Open */
#define ELFOSABI_SOLARIS       6    /* Solaris */
#define ELFOSABI_MONTEREY      7    /* Monterey */
#define ELFOSABI_IRIX          8    /* IRIX */
#define ELFOSABI_FREEBSD       9    /* FreeBSD */
#define ELFOSABI_TRU64        10    /* TRU64 UNIX */
#define ELFOSABI_MODESTO      11    /* Novell Modesto */
#define ELFOSABI_OPENBSD      12    /* OpenBSD */
#define ELFOSABI_OPENVMS      13    /* OpenVMS */
#define ELFOSABI_NSK          14    /* HP Non-Stop Kernel */
#define ELFOSABI_AROS         15    /* Amiga Research OS */
/* Unofficial OSABIs follow */
#define ELFOSABI_ARM          97    /* ARM */
#define ELFOSABI_STANDALONE  255    /* Standalone (embedded) application */

#define ELFOSABI_NONE        ELFOSABI_SYSV
#define ELFOSABI_AIX        ELFOSABI_MONTEREY


typedef struct _Elf_Magic
{
/* +0x00 */
    union {
        struct {
            Elf_Byte e_mag0;
            Elf_Byte e_mag1;
            Elf_Byte e_mag2;
            Elf_Byte e_mag3;
        };
        Elf32_Word ei_magic;
    };
/* +0x04 */    Elf_Byte   ei_class;      /* File class: elf file bits，1-> ELFCLASS32 is 32bits, 2 -> ELFCLASS64 is 64bits */ 
/* +0x05 */    Elf_Byte   ei_data;       /* Data encoding: 1 -> ELFDATA2LSB Little-Endian or 2 -> ELFDATA2MSB Big-Endian */ 
/* +0x06 */    Elf_Byte   ei_version;    /* File version */
/* +0x07 */    Elf_Byte   ei_osabi;      /* Operating system/ABI identification: ELFOSABI_SYSV / ELFOSABI_LINUX / ELFOSABI_FREEBSD / ELFOSABI_ARM */
/* +0x08 */    Elf_Byte   ei_abiversion; /* ABI version */
/* +0x09 */    Elf_Byte   ei_pad[7];     /* Start of padding bytes up to EI_NIDENT */
}Elf_Magic, *PElf_Magic;



//////////////////////////////////////////////////////////////////////////
// ELF header 
//////////////////////////////////////////////////////////////////////////
typedef struct _Elf32_Ehdr
{
/* +0x00 */    Elf_Magic   e_ident;        /* magic header */ 
/* +0x10 */    Elf32_Half  e_type;         /* file type: ET_NONE 0x0000 no file type, ET_REL 0x001 Relocatable file, ET_EXEC 0002 exe file, ET_DYN 0003 so */ 
/* +0x12 */    Elf32_Half  e_machine;      /* machine type: 0x003e x86-64, 0x0003 x86, 0x28 armeabi, 0xB7 arm64-v8a */ 
/* +0x14 */    Elf32_Word  e_version;      /* version number: 0x00000001 current version */ 
/* +0x18 */    Elf32_Addr  e_entry;        /* entry point, important */    /* 程序入口点 */ 
/* +0x1C */    Elf32_Off   e_phoff;        /* Program hdr table offset */  /* 程序头文件中的偏移 */ 
/* +0x20 */    Elf32_Off   e_shoff;        /* Section hdr table offset */  /* 段表在文件中的偏移 */ 
/* +0x24 */    Elf32_Word  e_flags;        /* Processor flags */           /* 处理器相关的标志 */ 
/* +0x28 */    Elf32_Half  e_ehsize;       /* sizeof ehdr */               /* Elf32_Ehdr 此结构体的大小 */ 
/* +0x2A */    Elf32_Half  e_phentsize;    /* Program header entry size */ /* 程序头的大小 */ 
/* +0x2C */    Elf32_Half  e_phnum;        /* Number of program headers */ /* 程序头的数量 */ 
/* +0x2E */    Elf32_Half  e_shentsize;    /* Section header entry size */ /* 段中每个结构的大小 */ 
/* +0x30 */    Elf32_Half  e_shnum;        /* Number of section headers */ /* 段表的数量 */ 
/* +0x32 */    Elf32_Half  e_shstrndx;     /* String table index */        /* 字符串表在段表中的索引 */ 
} Elf32_Ehdr, *PElf32_Ehdr;

typedef struct _Elf64_Ehdr
{
/* +0x00 */    Elf_Magic       e_ident;          /* magic header */ 
/* +0x10 */    Elf64_Quarter   e_type;           /* file type: 02 exe file, 03 so */ 
/* +0x12 */    Elf64_Quarter   e_machine;        /* machine type */ 
/* +0x14 */    Elf64_Word      e_version;        /* version number */ 
/* +0x18 */    Elf64_Addr      e_entry;          /* entry point */               /* 程序入口点 */  
/* +0x20 */    Elf64_Off       e_phoff;          /* Program hdr offset */        /* 代码段在文件中的偏移 */ 
/* +0x28 */    Elf64_Off       e_shoff;          /* Section hdr offset */        /* 段表在文件中的偏移 */  
/* +0x30 */    Elf64_Word      e_flags;          /* Processor flags */           /* 处理器相关的标志 */  
/* +0x34 */    Elf64_Quarter   e_ehsize;         /* sizeof ehdr */               /* Elf32_Ehdr 此结构体的大小 */ 
/* +0x36 */    Elf64_Quarter   e_phentsize;      /* Program header entry size */ /* 代码段头的大小 */ 
/* +0x38 */    Elf64_Quarter   e_phnum;          /* Number of program headers */ /* 代码段头的数量 */ 
/* +0x3A */    Elf64_Quarter   e_shentsize;      /* Section header entry size */ /* 段中每个结构的大小 */ 
/* +0x3C */    Elf64_Quarter   e_shnum;          /* Number of section headers */ /* 段表的数量 */ 
/* +0x3E */    Elf64_Quarter   e_shstrndx;       /* String table index */        /* 字符串表在段表中的索引 */ 
} Elf64_Ehdr, *PElf64_Ehdr;


/* e_type */
#define ET_NONE       0    /* No file type */
#define ET_REL        1    /* Relocatable file */
#define ET_EXEC       2    /* Executable file */
#define ET_DYN        3    /* Shared object file */
#define ET_CORE       4    /* Core file */
#define ET_NUM        5

#define ET_LOOS        0xfe00    /* Operating system specific range */
#define ET_HIOS        0xfeff
#define ET_LOPROC      0xff00    /* Processor-specific range */
#define ET_HIPROC      0xffff

/* e_machine */
#define EM_NONE        0    /* No machine */
#define EM_M32         1    /* AT&T WE 32100 */
#define EM_SPARC       2    /* SPARC */
#define EM_386         3    /* Intel 80386 */
#define EM_68K         4    /* Motorola 68000 */
#define EM_88K         5    /* Motorola 88000 */
#define EM_486         6    /* Intel 80486 */
#define EM_860         7    /* Intel 80860 */
#define EM_MIPS        8    /* MIPS I Architecture */
#define EM_S370        9    /* Amdahl UTS on System/370 */
#define EM_MIPS_RS3_LE    10    /* MIPS RS3000 Little-endian */
/* 11-14 - Reserved */
#define EM_RS6000     11    /* IBM RS/6000 XXX reserved */
#define EM_PARISC     15    /* Hewlett-Packard PA-RISC */
#define EM_NCUBE      16    /* NCube XXX reserved */
#define EM_VPP500     17    /* Fujitsu VPP500 */
#define EM_SPARC32PLUS    18    /* Enhanced instruction set SPARC */
#define EM_960        19    /* Intel 80960 */
#define EM_PPC        20    /* PowerPC */
#define EM_PPC64      21    /* 64-bit PowerPC */

/* 22-35 - Reserved */
#define EM_S390        22    /* System/390 XXX reserved */
#define EM_V800        36    /* NEC V800 */
#define EM_FR20        37    /* Fujitsu FR20 */
#define EM_RH32        38    /* TRW RH-32 */
#define EM_RCE         39    /* Motorola RCE */
#define EM_ARM         40    /* Advanced RISC Machines ARM */
#define EM_ALPHA       41    /* DIGITAL Alpha */
#define EM_SH          42    /* Hitachi Super-H */
#define EM_SPARCV9     43    /* SPARC Version 9 */
#define EM_TRICORE     44    /* Siemens Tricore */
#define EM_ARC         45    /* Argonaut RISC Core */
#define EM_H8_300      46    /* Hitachi H8/300 */
#define EM_H8_300H     47    /* Hitachi H8/300H */
#define EM_H8S         48    /* Hitachi H8S */
#define EM_H8_500      49    /* Hitachi H8/500 */
#define EM_IA_64       50    /* Intel Merced Processor */
#define EM_MIPS_X      51    /* Stanford MIPS-X */
#define EM_COLDFIRE    52    /* Motorola Coldfire */
#define EM_68HC12      53    /* Motorola MC68HC12 */
#define EM_MMA         54    /* Fujitsu MMA Multimedia Accelerator */
#define EM_PCP         55    /* Siemens PCP */
#define EM_NCPU        56    /* Sony nCPU embedded RISC processor */
#define EM_NDR1        57    /* Denso NDR1 microprocessor */
#define EM_STARCORE    58    /* Motorola Star*Core processor */
#define EM_ME16        59    /* Toyota ME16 processor */
#define EM_ST100       60    /* STMicroelectronics ST100 processor */
#define EM_TINYJ       61    /* Advanced Logic Corp. TinyJ embedded family processor */
#define EM_X86_64      62    /* AMD x86-64 architecture */
#define EM_PDSP        63    /* Sony DSP Processor */
#define EM_PDP10       64    /* Digital Equipment Corp. PDP-10 */
#define EM_PDP11       65    /* Digital Equipment Corp. PDP-11 */
#define EM_FX66        66    /* Siemens FX66 microcontroller */
#define EM_ST9PLUS     67    /* STMicroelectronics ST9+ 8/16 bit microcontroller */
#define EM_ST7         68    /* STMicroelectronics ST7 8-bit microcontroller */
#define EM_68HC16      69    /* Motorola MC68HC16 Microcontroller */
#define EM_68HC11      70    /* Motorola MC68HC11 Microcontroller */
#define EM_68HC08      71    /* Motorola MC68HC08 Microcontroller */
#define EM_68HC05      72    /* Motorola MC68HC05 Microcontroller */
#define EM_SVX         73    /* Silicon Graphics SVx */
#define EM_ST19        74    /* STMicroelectronics ST19 8-bit CPU */
#define EM_VAX         75    /* Digital VAX */
#define EM_CRIS        76    /* Axis Communications 32-bit embedded processor */
#define EM_JAVELIN     77    /* Infineon Technologies 32-bit embedded CPU */
#define EM_FIREPATH    78    /* Element 14 64-bit DSP processor */
#define EM_ZSP         79    /* LSI Logic's 16-bit DSP processor */
#define EM_MMIX        80    /* Donald Knuth's educational 64-bit processor */
#define EM_HUANY       81    /* Harvard's machine-independent format */
#define EM_PRISM       82    /* SiTera Prism */
#define EM_AVR         83    /* Atmel AVR 8-bit microcontroller */
#define EM_FR30        84    /* Fujitsu FR30 */
#define EM_D10V        85    /* Mitsubishi D10V */
#define EM_D30V        86    /* Mitsubishi D30V */
#define EM_V850        87    /* NEC v850 */
#define EM_M32R        88    /* Mitsubishi M32R */
#define EM_MN10300     89    /* Matsushita MN10300 */
#define EM_MN10200     90    /* Matsushita MN10200 */
#define EM_PJ          91    /* picoJava */
#define EM_OPENRISC    92    /* OpenRISC 32-bit embedded processor */
#define EM_ARC_A5      93    /* ARC Cores Tangent-A5 */
#define EM_XTENSA      94    /* Tensilica Xtensa Architecture */
#define EM_VIDEOCORE   95    /* Alphamosaic VideoCore processor */
#define EM_TMM_GPP     96    /* Thompson Multimedia General Purpose Processor */
#define EM_NS32K       97    /* National Semiconductor 32000 series */
#define EM_TPC         98    /* Tenor Network TPC processor */
#define EM_SNP1K       99    /* Trebia SNP 1000 processor */
#define EM_ST200      100    /* STMicroelectronics ST200 microcontroller */
#define EM_IP2K       101    /* Ubicom IP2xxx microcontroller family */
#define EM_MAX        102    /* MAX processor */
#define EM_CR         103    /* National Semiconductor CompactRISC micorprocessor */
#define EM_F2MC16     104    /* Fujitsu F2MC16 */
#define EM_MSP430     105    /* Texas Instruments MSP430 */
#define EM_BLACKFIN   106    /* Analog Devices Blackfin DSP */
#define EM_SE_C33     107    /* Seiko Epson S1C33 family */
#define EM_SEP        108    /* Sharp embedded microprocessor */
#define EM_ARCA       109    /* Arca RISC microprocessor */
#define EM_UNICORE    110    /* UNICORE from PKU-Unity Ltd. and MPRC Peking University */

/* Unofficial machine types follow */
#define EM_AVR32         6317    /* used by NetBSD/avr32 */
#define EM_ALPHA_EXP    36902    /* used by NetBSD/alpha; obsolete */
#define EM_NUM          36903


/* e_version */
#define EV_NONE        0        /* Invalid */
#define EV_CURRENT    1        /* Current */
#define EV_NUM        2        /* number of versions */





//////////////////////////////////////////////////////////////////////////
///* Section Header */
// PElf32_Ehdr->e_shoff 就指向 Section Table 
//////////////////////////////////////////////////////////////////////////
typedef struct _Elf32_Shdr{
/* +0x00 */    Elf32_Word    sh_name;      /* name - index into section header string table section */  /* 段名在字符串表中的索引(字符串表索引为 PElf32_Ehdr->e_shstrndx ) */
/* +0x04 */    Elf32_Word    sh_type;      /* type */
/* +0x08 */    Elf32_Word    sh_flags;     /* flags */        /* 该section在进程执行时的特性 */
/* +0x0C */    Elf32_Addr    sh_addr;      /* address */      /* 若此section在进程的内存映像中出现，则给出开始的虚地址 */
/* +0x10 */    Elf32_Off     sh_offset;    /* file offset */  /* 此section在文件中的偏移 */
/* +0x14 */    Elf32_Word    sh_size;      /* section size */ /* 此section的数据大小 */
/* +0x18 */    Elf32_Word    sh_link;      /* section header table index link */
/* +0x1C */    Elf32_Word    sh_info;      /* extra information */
/* +0x20 */    Elf32_Word    sh_addralign; /* address alignment */
/* +0x24 */    Elf32_Word    sh_entsize;   /* section entry size */
} Elf32_Shdr, *PElf32_Shdr;

typedef struct _Elf64_Shdr{
/* +0x00 */    Elf64_Half     sh_name;      /* section name */
/* +0x04 */    Elf64_Half     sh_type;      /* section type */
/* +0x08 */    Elf64_Xword    sh_flags;     /* section flags */   /* 该section在进程执行时的特性 */
/* +0x10 */    Elf64_Addr     sh_addr;      /* virtual address */
/* +0x18 */    Elf64_Off      sh_offset;    /* file offset */     /* 此section内容数据在文件中的偏移 */
/* +0x20 */    Elf64_Xword    sh_size;      /* section size */    /* 此section的数据大小 */
/* +0x28 */    Elf64_Half     sh_link;      /* link to another */
/* +0x2C */    Elf64_Half     sh_info;      /* misc info */
/* +0x30 */    Elf64_Xword    sh_addralign; /* memory alignment */
/* +0x38 */    Elf64_Xword    sh_entsize;   /* table entry size */
} Elf64_Shdr, *PElf64_Shdr;


/* sh_type */
#define SHT_NULL	0		/*  section头是无效的；它没有相关的section. inactive */
#define SHT_PROGBITS	1		/* 该section保存被程序定义了的一些信息，它的格式和意义取决于程序本身 program defined information */
#define SHT_SYMTAB	2		/* 保存着符号表 symbol table section */
#define SHT_STRTAB	3		/* 保存着一个字符串表 string table section */
#define SHT_RELA	4		/* 保存着具有明确加数的重定位入口 relocation section with addends*/
#define SHT_HASH	5		/* 保存着一个标号的哈希(hash)表 symbol hash table section */
#define SHT_DYNAMIC	6		/* 保存着动态连接的信息 dynamic section */
#define SHT_NOTE	7		/* 保存着其他的一些标志文件的信息 note section */
#define SHT_NOBITS	8		/* 在文件中不占空间，sh_offset成员包含了概念上的文件偏移量 no space section */
#define SHT_REL		9		/* 保存着具有明确加数的重定位的入口 relation section without addends */
#define SHT_SHLIB	10		/* 类型保留但语意没有指明，包含这个类型的section的程序是不符合ABI的规定 reserved - purpose unknown */
#define SHT_DYNSYM	11		/* 保存着符号表 dynamic symbol table section */
#define SHT_NUM		12		/*  number of section types */
#define SHT_LOPROC	0x70000000	/* 在这范围之间的值为特定处理器语意保留  reserved range for processor */
#define SHT_HIPROC	0x7fffffff	/*  specific section header types */
#define SHT_LOUSER	0x80000000	/* 为应用程序保留的索引范围的最小边界 reserved range for application */
#define SHT_HIUSER	0xffffffff	/* 为应用程序保留的索引范围的最大边界 specific indexes */



//////////////////////////////////////////////////////////////////////////
//
/* Symbol Table Entry */
typedef struct elf32_sym {
    Elf32_Word	st_name;	/* 符号名称，字符串表中的索引 */
    Elf32_Addr	st_value;	/* 符号的值 */
    Elf32_Word	st_size;	/* 符号的长度，如一个指针的长度或struct对象中包含的字节数 */
    unsigned char	st_info;	/* 类型和绑定属性：STB_LOCAL/STB_GLOBAL/STB_WEAK */
    unsigned char	st_other;	/* 语义未定义，0 */
    Elf32_Half	st_shndx;	/*  相关节的索引，符号将绑定到该节，此外SHN_ABS指定符号是绝对值，不因重定位而改变，SHN_UNDEF标识未定义符号 */
} Elf32_Sym;

typedef struct {
    Elf64_Half  st_name;    /* Symbol name index in str table */
    Elf_Byte    st_info;    /* type / binding attrs */
    Elf_Byte    st_other;   /* unused */
    Elf64_Half  st_shndx;   /* section index of symbol */
    Elf64_Xword st_value;   /* value of symbol */
    Elf64_Xword st_size;    /* size of symbol */
} Elf64_Sym;


//////////////////////////////////////////////////////////////////////////
//
/* Dynamic structure */
typedef struct _Elf32_Dyn{
    Elf32_Sword	d_tag;		/* controls meaning of d_val */
    union {
        Elf32_Word	d_val;	/* Multiple meanings - see d_tag */
        Elf32_Addr	d_ptr;	/* program virtual address */
    } d_un;
} Elf32_Dyn, *PElf32_Dyn;

typedef struct _Elf64_Dyn{
    Elf64_Xword	d_tag;		/* controls meaning of d_val */
    union {
        Elf64_Addr	d_ptr;
        Elf64_Xword	d_val;
    } d_un;
} Elf64_Dyn, *PElf64_Dyn;

/* Dynamic Array Tags - d_tag */
#define DT_NULL		0		/* marks end of _DYNAMIC array */
#define DT_NEEDED	1		/* string table offset of needed lib */
#define DT_PLTRELSZ	2		/* size of relocation entries in PLT */
#define DT_PLTGOT	3		/* address PLT/GOT */
#define DT_HASH		4		/* address of symbol hash table */
#define DT_STRTAB	5		/* address of string table */
#define DT_SYMTAB	6		/* address of symbol table */
#define DT_RELA		7		/* address of relocation table */
#define DT_RELASZ	8		/* size of relocation table */
#define DT_RELAENT	9		/* size of relocation entry */
#define DT_STRSZ	10		/* size of string table */
#define DT_SYMENT	11		/* size of symbol table entry */
#define DT_INIT		12		/* address of initialization func. */
#define DT_FINI		13		/* address of termination function */
#define DT_SONAME	14		/* string table offset of shared obj */
#define DT_RPATH	15		/* string table offset of library search path */
#define DT_SYMBOLIC	16		/* start sym search in shared obj. */
#define DT_REL		17		/* address of rel. tbl. w addends */
#define DT_RELSZ	18		/* size of DT_REL relocation table */
#define DT_RELENT	19		/* size of DT_REL relocation entry */
#define DT_PLTREL	20		/* PLT referenced relocation entry */
#define DT_DEBUG	21		/* bugger */
#define DT_TEXTREL	22		/* Allow rel. mod. to unwritable seg */
#define DT_JMPREL	23		/* add. of PLT's relocation entries */
#define DT_BIND_NOW	24		/* Bind now regardless of env setting */
#define DT_NUM		25		/* Number used. */
#define DT_LOPROC	0x70000000	/* reserved range for processor */
#define DT_HIPROC	0x7fffffff	/*  specific dynamic array tags */


#pragma pack(pop)
#endif  // _SYS_EXEC_ELF_H_