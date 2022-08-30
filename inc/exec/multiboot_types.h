#ifndef LBOOT_EXEC_MULTIBOOT_TYPES_H
#define LBOOT_EXEC_MULTIBOOT_TYPES_H

#include <stdint.h>

#define MULTIBOOT1_HEAD_MAGIC (0x1BADB002UL)

typedef struct {
    uint32_t size;
    uint32_t _reserved;
    uint8_t  tags[];
} multiboot2_t;

typedef struct {
    uint32_t magic;
#  define MULTIBOOT2_HEAD_MAGIC (0xE85250D6UL)
    uint32_t architecture;
#  define MULTIBOOT2_HEAD_ARCHITECTURE_X86    (0UL)
#  define MULTIBOOT2_HEAD_ARCHITECTURE_MIPS32 (4UL)
    uint32_t header_length;
    uint32_t checksum;
    uint8_t  tags[];
} multiboot2_head_t;

typedef enum {
    MULTIBOOT2_TAGTYPE_END             = 0,
    MULTIBOOT2_TAGTYPE_CMDLINE         = 1,
    MULTIBOOT2_TAGTYPE_BOOTLOADER_NAME = 2,
    MULTIBOOT2_TAGTYPE_MODULE          = 3,
    MULTIBOOT2_TAGTYPE_BASIC_MEMINFO   = 4,
    MULTIBOOT2_TAGTYPE_BOOTDEV         = 5,
    MULTIBOOT2_TAGTYPE_MMAP            = 6,
    MULTIBOOT2_TAGTYPE_VBE             = 7,
    MULTIBOOT2_TAGTYPE_FRAMEBUFFER     = 8,
    MULTIBOOT2_TAGTYPE_ELF_SECTIONS    = 9,
    MULTIBOOT2_TAGTYPE_APM             = 10,
    MULTIBOOT2_TAGTYPE_EFI32           = 11,
    MULTIBOOT2_TAGTYPE_EFI64           = 12,
    MULTIBOOT2_TAGTYPE_SMBIOS          = 13,
    MULTIBOOT2_TAGTYPE_ACPI_OLD        = 14,
    MULTIBOOT2_TAGTYPE_ACPI_NEW        = 15,
    MULTIBOOT2_TAGTYPE_NETWORK         = 16,
    MULTIBOOT2_TAGTYPE_EFI_MMAP        = 17,
    MULTIBOOT2_TAGTYPE_EFI_BS          = 18,
    MULTIBOOT2_TAGTYPE_EFI21_IH        = 19,
    MULTIBOOT2_TAGTYPE_EFI64_IH        = 20,
    MULTIBOOT2_TAGTYPE_LOAD_BASE_ADDR  = 21
} multiboot2_tag_type_e;

typedef enum {
    MULTIBOOT2_HEADERTAG_END                 =  0,
    MULTIBOOT2_HEADERTAG_INFORMATION_REQUEST =  1,
    MULTIBOOT2_HEADERTAG_ADDRESS             =  2,
    MULTIBOOT2_HEADERTAG_ENTRY_ADDRESS       =  3,
    MULTIBOOT2_HEADERTAG_CONSOLE_FLAGS       =  4,
    MULTIBOOT2_HEADERTAG_FRAMEBUFFER         =  5,
    MULTIBOOT2_HEADERTAG_MODULE_ALIGN        =  6,
    MULTIBOOT2_HEADERTAG_EFI_BS              =  7,
    MULTIBOOT2_HEADERTAG_ENTRY_ADDRESS_EFI32 =  8,
    MULTIBOOT2_HEADERTAG_ENTRY_ADDRESS_EFI64 =  9,
    MULTIBOOT2_HEADERTAG_RELOCATEABLE        = 10,
} multiboot2_header_tag_e;

typedef struct {
    uint16_t type;
    uint16_t flags;
#define MULTIBOOT2_TAG_FLAG_OPTIONAL (1UL << 0)
    uint32_t size;
    uint8_t  data[];
} multiboot2_tag_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    char     cmdline[];
} multiboot2_tag_cmdline_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char     name[];
} multiboot2_tag_module_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t size_lower; /**< Available contiguous memory below 1 MiB, starting at 0, in KiB */
    uint32_t size_upper; /**< Available contiguous memory above 1 MiB, in KiB */
} multiboot2_tag_basicmem_t;

typedef struct {
    uint64_t base_addr; /**< Base address of memory region */
    uint64_t length;    /**< Length of memory region in bytes */
    uint32_t type;      /**< Type of memory region */
#define MULTIBOOT2_MMAP_TYPE_RESERVED  (0) /**< Memory region not available for use. */
#define MULTIBOOT2_MMAP_TYPE_AVAILABLE (1) /**< Memory region is available for use. */
#define MULTIBOOT2_MMAP_TYPE_ACPI      (3) /**< Memory region contains ACPI info. */
#define MULTIBOOT2_MMAP_TYPE_PRESERVE  (4) /**< Memory region must be preserved through hibernation. */
#define MULTIBOOT2_MMAP_TYPE_DEFECTIVE (5) /**< Memory region is defective. */
    uint32_t _reserved;
} multiboot2_mmap_entry_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;    /**< Size of each mmap entry. */
    uint32_t entry_version; /**< Version of mmap entry format, currently 0. */
    multiboot2_mmap_entry_t entries[];
} multiboot2_tag_mmap_t;

#pragma pack(push, 1)

#define RSDPv1_SIZE (20)
#define RSDPv2_SIZE (36)

typedef struct {
    char     signature[8];
#define ACPI_RSDP_SIGNATURE ("RSD PTR") /**< RSDP signature, not NULL terminated */
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;
    uint32_t rsdt_address;

    /* The following are only valid for V2 */
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t  checksum_extended;
    uint8_t  _reserved[3];
} acpi_rsdp_desc_t;
#pragma pack(pop)

typedef struct {
    uint32_t         type;
    uint32_t         size;
    acpi_rsdp_desc_t rsdp;
} multiboot2_tag_acpi_rsdp_t;

#endif

