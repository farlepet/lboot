#ifndef LBOOT_EXEC_FMT_ELF_TYPES_H
#define LBOOT_EXEC_FMT_ELF_TYPES_H

#include <stdint.h>

/*
 * ELF file header
 */

#define ELF_IDENT (0x464c457fUL) /**< ELF magic number - 0x7f + "ELF" */

typedef enum elf_class_enum {
    ELF_CLASS_NONE  = 0x00, /**< None */
    ELF_CLASS_32BIT = 0x01, /**< 32-bit format */
    ELF_CLASS_64BIT = 0x02  /**< 64-bit format */
} elf_class_e;

typedef enum elf_data_enum {
    ELF_DATA_NONE         = 0x00, /**< None */
    ELF_DATA_LITTLEENDIAN = 0x01, /**< Little-endian format */
    ELF_DATA_BIGENDIAN    = 0x02  /**< Big-endian format */
} elf_data_e;

typedef enum elf_machine_enum {
    ELF_MACHINE_NONE   = 0x0000, /**< NONE/Unspecified  */
    ELF_MACHINE_X86    = 0x0003, /**< 32-bit x86 */
    ELF_MACHINE_ARM32  = 0x0028, /**< 32-bit ARM */
    ELF_MACHINE_X86_64 = 0x003e, /**< x86-64 */
    ELF_MACHINE_ARM64  = 0x00b7, /**< 64-bit ARM */
    ELF_MACHINE_RISCV  = 0x00f3  /**< RISC-V */
} elf_machine_e;

#define HOST_ELF_CLASS   ELF_CLASS_32BIT
#define HOST_ELF_MACHINE ELF_MACHINE_X86

typedef enum elf_type_enum {
    ELF_TYPE_NONE   = 0x0000, /**< NONE/Unknown */
    ELF_TYPE_REL    = 0x0001, /**< Relocatable */
    ELF_TYPE_EXEC   = 0x0002, /**< Executable */
    ELF_TYPE_DYN    = 0x0003, /**< Shared object */
    ELF_TYPE_CORE   = 0x0004, /**< Core file */
    ELF_TYPE_LOOS   = 0xfe00, /**< OS-specific range start */
    ELF_TYPE_HIOS   = 0xfeff, /**< OS-specific range end */
    ELF_TYPE_LOPROC = 0xff00, /**< Processor-specific range start */
    ELF_TYPE_HIPROC = 0xffff  /**< Processor-specific range end */
} elf_type_e;

typedef struct elf_header_struct {
    struct {
        uint32_t magic;      /**< 32-bit magic number */
        uint8_t  class;      /**< class/bittiness */
        uint8_t  data;       /**< data format/endianness */
        uint8_t  version;    /**< version */
        uint8_t  osabi;      /**< target ABI */
        uint8_t  abiversion; /**< target ABI version */
        uint8_t  _reserved[7];
    } ident;
    uint16_t type;       /**< Type, @see elf_type_e */
    uint16_t machine;    /**< Target machine/ISA, @see elf_machine_type */
    uint32_t version;    /**< ELF version */
    union {
        struct {
            uint32_t entry;      /**< Entry point */
            uint32_t phoff;      /**< Program header table offset */
            uint32_t shoff;      /**< Section header table offset */
            uint32_t flags;      /**< Target-dependant flags */
            uint16_t ehsize;     /**< Size of this header (52 bytes) */
            uint16_t phentsize;  /**< Size of a program header table entry */
            uint16_t phnum;      /**< Number of program header table entries */
            uint16_t shentsize;  /**< Size of a section header table entry */
            uint16_t shnum;      /**< Number of section header table entries */
            uint16_t shstrndx;   /**< Section index that contains section names */
        } e32;
        struct {
            uint64_t entry;      /**< Entry point */
            uint64_t phoff;      /**< Program header table offset */
            uint64_t shoff;      /**< Section header table offset */
            uint32_t flags;      /**< Target-dependant flags */
            uint16_t ehsize;     /**< Size of this header (64 bytes) */
            uint16_t phentsize;  /**< Size of a program header table entry */
            uint16_t phnum;      /**< Number of program header table entries */
            uint16_t shentsize;  /**< Size of a section header table entry */
            uint16_t shnum;      /**< Number of section header table entries */
            uint16_t shstrndx;   /**< Section index that contains section names */           
        } e64;
    };
} elf_header_t;


/*
 * ELF program header
 */

typedef enum elf_phdr_type_enum {
    ELF_PHDR_TYPE_NULL    = 0x00000000, /**< Unused entry */
    ELF_PHDR_TYPE_LOAD    = 0x00000001, /**< Loadable secment */
    ELF_PHDR_TYPE_DYNAMIC = 0x00000002, /**< Dynamic linking information */
    ELF_PHDR_TYPE_INTERP  = 0x00000003, /**< Interpreter information */
    ELF_PHDR_TYPE_NOTE    = 0x00000004, /**< Auxiliary information */
    ELF_PHDR_TYPE_SHLIB   = 0x00000005, /**< Reserved */
    ELF_PHDR_TYPE_PHDR    = 0x00000006, /**< Program header table */
    ELF_PHDR_TYPE_TLS     = 0x00000007, /**< Thread-Local Storage template */

    ELF_PHDR_TYPE_LOOS    = 0x60000000, /**< OS-specific range start */
    ELF_PHDR_TYPE_HIOS    = 0x6fffffff, /**< OS-specific range end */
    ELF_PHDR_TYPE_LOPROC  = 0x70000000, /**< Processor-specific range start */
    ELF_PHDR_TYPE_HIPROC  = 0x7fffffff  /**< Processor-specific range end */
} elf_phdr_type_e;

typedef struct elf32_phdr_struct {
    uint32_t type;   /**< Type, @see elf_phead_type_e */
    uint32_t offset; /**< Offset of segment into file image */
    uint32_t vaddr;  /**< Virtual address of segment */
    uint32_t paddr;  /**< Physical address of segment, if relevant */
    uint32_t filesz; /**< Size of segment within file, in bytes */
    uint32_t memsz;  /**< Size of segment within memory, in bytes */
    uint32_t flags;  /**< Segment flags */
    uint32_t align;  /**< Required alignment of section */
} elf32_phdr_t;



/*
 * Elf section header
 */

typedef enum elf_shdr_type_enum {
    ELF_SHDR_TYPE_NONE          = 0x00000000, /**< Unused */
    ELF_SHDR_TYPE_PROGBITS      = 0x00000001, /**< Program data */
    ELF_SHDR_TYPE_SYMTAB        = 0x00000002, /**< Symbol table */
    ELF_SHDR_TYPE_STRTAB        = 0x00000003, /**< String table */
    ELF_SHDR_TYPE_RELA          = 0x00000004, /**< Relocation entries, with addends */
    ELF_SHDR_TYPE_HASH          = 0x00000005, /**< Symbol table hash */
    ELF_SHDR_TYPE_DYNAMIC       = 0x00000006, /**< Dynamic linking information */
    ELF_SHDR_TYPE_NOTE          = 0x00000007, /**< Notes */
    ELF_SHDR_TYPE_NOBITS        = 0x00000008, /**< Program space w/o data (bss)  */
    ELF_SHDR_TYPE_REL           = 0x00000009, /**< Relocation entries */
    ELF_SHDR_TYPE_SHLIB         = 0x0000000a, /**< Reserved */
    ELF_SHDR_TYPE_DYNSYM        = 0x0000000b, /**< Dynamic linker symbol table */
    ELF_SHDR_TYPE_INIT_ARRAY    = 0x0000000e, /**< Array of constructors */
    ELF_SHDR_TYPE_FINI_ARRAY    = 0x0000000f, /**< Array of destructors */
    ELF_SHDR_TYPE_PREINIT_ARRAY = 0x00000010, /**< Array of pre-constructors */
    ELF_SHDR_TYPE_GROUP         = 0x00000011, /**< Section group */
    ELF_SHDR_TYPE_SYMTAB_SHNDX  = 0x00000012  /**< Extended section indices */
} elf_shdr_type_e;

#define ELF_SHDR_FLAG_WRITE            (0x00000001UL) /**< Writable */
#define ELF_SHDR_FLAG_ALLOC            (0x00000002UL) /**< Occupies memory during execution */
#define ELF_SHDR_FLAG_EXECINSTR        (0x00000004UL) /**< Executable */
#define ELF_SHDR_FLAG_MERGE            (0x00000010UL) /**< Might be merged */
#define ELF_SHDR_FLAG_STRINGS          (0x00000020UL) /**< Contains NULL-terminated strings */
#define ELF_SHDR_FLAG_INFO_LINK        (0x00000040UL) /**< `info` contains symbol header table index */
#define ELF_SHDR_FLAG_LINK_ORDER       (0x00000080UL) /**< Preserve order after combining */
#define ELF_SHDR_FLAG_OS_NONCONFORMING (0x00000100UL) /**< Non-standard OS-specific handling required */
#define ELF_SHDR_FLAG_GROUP            (0x00000200UL) /**< Section is member of a group*/
#define ELF_SHDR_FLAG_TLS              (0x00000400UL) /**< Sections holds thread-local data */
#define ELF_SHDR_FLAG_MASKOS           (0x0FF00000UL) /**< OS-specific flag mask */
#define ELF_SHDR_FLAG_MASKPROC         (0xF0000000UL) /**< Processor-specific flag mask */

typedef struct elf32_shdr_struct {
    uint32_t name;      /**< Offset into .shstrtab section containing section name */
    uint32_t type;      /**< Section type, @see elf_shdr_type_e */
    uint32_t flags;     /**< Section flags */
    uint32_t addr;      /**< Virtual address of loaded section, if applicable */
    uint32_t offset;    /**< Offset of section in file */
    uint32_t size;      /**< Size of section data within file, in bytes */
    uint32_t link;      /**< Index of linked section, if applicable */
    uint32_t info;      /**< Extra section information */
    uint32_t addralign; /**< Required alignment of section */
    uint32_t entsize;   /**< Size of each section entry, if applicable */
} elf32_shdr_t;


/*
 * ELF symbol table
 */

typedef struct elf32_sym_struct {
    uint32_t name;  /**< Index into symbol string table containing name, or zero */
    uint32_t value; /**< Symbol value */
    uint32_t size;  /**< Size of symbol, if applicable */
    uint8_t  info;  /**< Symbol type and binding */
    uint8_t  other; /**< Symbol visibility */
    uint16_t shndx; /**< Section header index of relevant section */
} elf32_sym_t;

typedef enum elf_sym_bind_enum {
    ELF_SYM_BIND_LOCAL  = 0,  /**< Local symbol */
    ELF_SYM_BIND_GLOBAL = 1,  /**< Global symbol */
    ELF_SYM_BIND_WEAK   = 2,  /**< Weak symbol */
    ELF_SYM_BIND_LOOS   = 10, /**< OS-specific range start */
    ELF_SYM_BIND_HIOS   = 12, /**< OS-specific range end */
    ELF_SYM_BIND_LOPROC = 13, /**< Processor-specific range start */
    ELF_SYM_BIND_HIPROC = 15  /**< Processor-specific range end */
} elf_sym_bind_e;
#define ELF_SYM_INFO_BIND(i) ((i)>>4)

typedef enum elf_sym_type_enum {
    ELF_SYM_TYPE_NOTYPE  =  0, /**< Not specified */
    ELF_SYM_TYPE_OBJECT  =  1, /**< Data object */
    ELF_SYM_TYPE_FUNC    =  2, /**< Function, or other executable code */
    ELF_SYM_TYPE_SECTION =  3, /**< Section */
    ELF_SYM_TYPE_FILE    =  4, /**< File */
    ELF_SYM_TYPE_COMMON  =  5, /**< Uninitialized commo block */
    ELF_SYM_TYPE_LOOS    = 10, /**< OS-specific range start */
    ELF_SYM_TYPE_HIOS    = 12, /**< OS-specific range end */
    ELF_SYM_TYPE_LOPROC  = 13, /**< Processor-specific range start */
    ELF_SYM_TYPE_HIPROC  = 15  /**< Processor-specific range end */
} elf_sym_type_e;
#define ELF_SYM_INFO_TYPE(i) ((i) & 0x0f)



/*
 * ELF relocations
 */

typedef struct elf32_rel_struct {
    uint32_t offset; /**< Offset at which to apply relocation */
    uint32_t info;   /**< Symbol table index, and relocation type */
} elf32_rel_t;

typedef struct elf32_rela_struct {
    uint32_t offset; /**< Offset at which to apply relocation */
    uint32_t info;   /**< Symbol table index, and relocation type */
    int32_t  addend; /**< Addend used in relocation computation */
} elf32_rela_t;

/** Relocation r_info symbol table index. */
#define ELF32_R_SYM(i)    ((i) >> 8)
/** Relocation r_info relcation type. */
#define ELF32_R_TYPE(i)   ((uint8_t)(i))
/** Generate r_info from symbol table index and relocation type. */
#define ELF32_R_INFO(s,t) (((s) << 8) + (uint8_t)(t))

typedef enum elf_reltype_x86_enum {
    ELF_RELTYPE_X86_NONE     =  0, /**< None */
    ELF_RELTYPE_X86_32       =  1, /**< X = Symbol value + Addend */
    ELF_RELTYPE_X86_PC32     =  2, /**< X = Symbol value + Addend - Section address */
    ELF_RELTYPE_X86_GOT32    =  3, /**< X = GOT offset + Addend */
    ELF_RELTYPE_X86_PLT32    =  4, /**< X = PLT address + Addend - Section address */
    ELF_RELTYPE_X86_COPY     =  5, /**< None */
    ELF_RELTYPE_X86_GLOB_DAT =  6, /**< X = Symbol value */
    ELF_RELTYPE_X86_JMP_SLOT =  7, /**< X = Symbol value */
    ELF_RELTYPE_X86_RELATIVE =  8, /**< X = Base address + Addend */
    ELF_RELTYPE_X86_GOTOFF   =  9, /**< X = Symbol value + Addend - GOT address */
    ELF_RELTYPE_x86_GOTPC    = 10, /**< X = GOT address + Addend - Section address */
    ELF_RELTYPE_X86_32PLT    = 11  /**< X = PLT offset + Addend */
} elf_reltype_x86_e;


/*
 * ELF dynamic table
 */

/** Dynamic table d_tag values: */
typedef enum elf_dyn_tag_enum {
    ELF_DYN_TAG_NULL            =  0, /**< Marks end of dynamic array */
    ELF_DYN_TAG_NEEDED          =  1, /**< Contains index into string table representing a name of a required library */
    ELF_DYN_TAG_PLTRELSZ        =  2, /**< */
    ELF_DYN_TAG_PLTGOT          =  3, /**< */
    ELF_DYN_TAG_HASH            =  4, /**< Contains address of symbol table hash */
    ELF_DYN_TAG_STRTAB          =  5, /**< Contains address of string table */
    ELF_DYN_TAG_SYMTAB          =  6, /**< Contains address of symbol table */
    ELF_DYN_TAG_RELA            =  7, /**< Contains address of relocation table w/ addends */
    ELF_DYN_TAG_RELASZ          =  8, /**< Contains size of relocation table w/ addends */
    ELF_DYN_TAG_RELAENT         =  9, /**< Contains size of relocation table entrty w/ addends */
    ELF_DYN_TAG_STRSZ           = 10, /**< Contains size of string table*/
    ELF_DYN_TAG_SYMENT          = 11, /**< Contains size of a symbol table entry */
    ELF_DYN_TAG_INIT            = 12, /**< Contains address of the initialization function */
    ELF_DYN_TAG_FINI            = 13, /**< Contains address of teh terminitaion function */
    ELF_DYN_TAG_SONAME          = 14, /**< Contains index into string table representing a name of a shared object */
    ELF_DYN_TAG_RPATH           = 15, /**< Contains index into string table representing a library search path */
    ELF_DYN_TAG_SYMBOLIC        = 16, /**< */
    ELF_DYN_TAG_REL             = 17, /**< Contains address of relocation table */
    ELF_DYN_TAG_RELSZ           = 18, /**< Contains size of relocation table */
    ELF_DYN_TAG_RELENT          = 19, /**< Contains size of relocation table */
    ELF_DYN_TAG_PLTREL          = 20, /**< */
    ELF_DYN_TAG_DEBUG           = 21, /**< Debug, value not standardized*/
    ELF_DYN_TAG_TEXTREL         = 22, /**< */
    ELF_DYN_TAG_JMPREL          = 23, /**< */
    ELF_DYN_TAG_BIND_NOW        = 24, /**< */
    ELF_DYN_TAG_INIT_ARRAY      = 25, /**< Contains address of array of pointers to initialization functions */
    ELF_DYN_TAG_FINI_ARRAY      = 26, /**< Contains address of array of pointers to termination functions */
    ELF_DYN_TAG_INIT_ARRAYSZ    = 27, /**< Contains size of initialization function array */
    ELF_DYN_TAG_FINI_ARRAYSZ    = 28, /**< Contains size of termination function array */
    ELF_DYN_TAG_RUNPATH         = 29, /**< Contains index into string table representing a library search path */
    ELF_DYN_TAG_FLAGS           = 30, /**< Contains object-specific flag values */
    ELF_DYN_TAG_ENCODING        = 32, /**< */
    ELF_DYN_TAG_PREINIT_ARRAY   = 32, /**< Contains address of array of pointers to pre-initialization functions */
    ELF_DYN_TAG_PREINIT_ARRAYSZ = 33, /**< Contains size of pre-initialization function array */
} elf_dyn_tag_e;

typedef struct elf32_dyn_struct {
    uint32_t tag; /**< Dynamic array tag, @see elf_dyn_tag_e */
    uint32_t val; /**< Dynamic tag value */
} elf32_dyn_t;

#endif

