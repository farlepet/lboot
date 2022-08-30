#include <stddef.h>
#include <string.h>

#include "bios/bios.h"
#include "exec/multiboot.h"
#include "io/output.h"
#include "mm/alloc.h"

const char *_bootloader_name = "LBoot";

#define MULTIBOOT2_PREALLOC (1024)

#define NEXT_TAG(T) (multiboot2_tag_t *)((uintptr_t)(T) + \
                                         (T)->size + \
                                         (((T)->size % 8) ? (8 - ((T)->size % 8)) : 0))

static int _mboot2_poptag_module(multiboot2_tag_t *tag, const config_data_module_t *mod);
static int _mboot2_poptag_meminfo(multiboot2_tag_t *tag);
static int _mboot2_poptag_mmap(multiboot2_tag_t *tag);
static int _mboot2_poptag_acpiold(multiboot2_tag_t *tag);
static int _mboot2_poptag_acpinew(multiboot2_tag_t *tag);

multiboot2_t *multiboot2_parse(const multiboot2_head_t *head, const config_data_t *cfg) {
    if((head->magic        != MULTIBOOT2_HEAD_MAGIC) ||
       (head->architecture != MULTIBOOT2_HEAD_ARCHITECTURE_X86)) {
        return NULL;
    }

    if((head->magic + head->architecture + head->header_length + head->checksum) != 0) {
        /* Invalid checksum */
        printf("multiboot2_parse: Invalid checksum!\n");
        return NULL;
    }

    multiboot2_t *mboot2 = alloc(MULTIBOOT2_PREALLOC, ALLOC_FLAG_ALIGN(3));
    memset(mboot2, 0, MULTIBOOT2_PREALLOC);

    multiboot2_tag_t *next_tag = (multiboot2_tag_t *)&mboot2->tags;

    const multiboot2_tag_t *htag = (multiboot2_tag_t *)&head->tags;
    while(htag < (multiboot2_tag_t *)(head + head->header_length)) {
        if(htag->type == MULTIBOOT2_HEADERTAG_END) {
            break;
        }

        switch(htag->type) {
            case MULTIBOOT2_HEADERTAG_INFORMATION_REQUEST: {
                uint32_t *reqs = (uint32_t *)&htag->data;
                for(unsigned i = 0; i < (htag->size - 8) / 4; i++) {
                    switch(reqs[i]) {
                        case MULTIBOOT2_TAGTYPE_CMDLINE:
                            if(cfg->kernel_cmdline) {
                                next_tag->type = MULTIBOOT2_TAGTYPE_CMDLINE;
                                next_tag->size = sizeof(multiboot2_tag_t) + strlen(cfg->kernel_cmdline) + 1;
                                strcpy((char *)&next_tag->data, cfg->kernel_cmdline);
#if (DEBUG_EXEC_MULTIBOOT)
                                printf("%p  TAG  1: %s\n", next_tag, (char *)next_tag->data);
#endif
                                next_tag = NEXT_TAG(next_tag);
                            }
                            break;
                        case MULTIBOOT2_TAGTYPE_BOOTLOADER_NAME:
                            next_tag->type = MULTIBOOT2_TAGTYPE_BOOTLOADER_NAME;
                            next_tag->size = sizeof(multiboot2_tag_t) + strlen(_bootloader_name) + 1;
                            strcpy((char *)&next_tag->data, _bootloader_name);
#if (DEBUG_EXEC_MULTIBOOT)
                                printf("%p  TAG  2: %s\n", next_tag, (char *)next_tag->data);
#endif
                            next_tag = NEXT_TAG(next_tag);
                            break;
                        case MULTIBOOT2_TAGTYPE_MODULE:
                            for(unsigned i = 0; i < cfg->module_count; i++) {
                                if(_mboot2_poptag_module(next_tag, &cfg->modules[i])) {
                                    goto mboot2_parse_error;
                                }
                                next_tag = NEXT_TAG(next_tag);
                            }
                            break;
                        case MULTIBOOT2_TAGTYPE_BASIC_MEMINFO:
                            if(_mboot2_poptag_meminfo(next_tag)) {
                                goto mboot2_parse_error;
                            }
                            next_tag = NEXT_TAG(next_tag);
                            break;
                        case MULTIBOOT2_TAGTYPE_MMAP:
                            if(_mboot2_poptag_mmap(next_tag)) {
                                goto mboot2_parse_error;
                            }
                            next_tag = NEXT_TAG(next_tag);
                            break;
                        case MULTIBOOT2_TAGTYPE_ACPI_OLD:
                            if(!_mboot2_poptag_acpiold(next_tag)) {
                                next_tag = NEXT_TAG(next_tag);
                            }
                            break;
                        case MULTIBOOT2_TAGTYPE_ACPI_NEW:
                            if(!_mboot2_poptag_acpinew(next_tag)) {
                                next_tag = NEXT_TAG(next_tag);
                            }
                            break;
                        default:
                            printf("Unhandled multiboot tag request: %d\n", reqs[i]);
                            goto mboot2_parse_error;
                    }
                }
            } break;
            default:
                printf("Unhandled multiboot header tag type: %d\n", htag->type);
                if(!(htag->flags & MULTIBOOT2_TAG_FLAG_OPTIONAL)) {
                    goto mboot2_parse_error;
                }
                break;
        }

        htag = NEXT_TAG(htag);
    }

    mboot2->size = (uint32_t)((void *)next_tag - (void *)mboot2);

    return mboot2;

mboot2_parse_error:
    free(mboot2);

    return NULL;
}

static int _mboot2_poptag_module(multiboot2_tag_t *tag, const config_data_module_t *mod) {
    multiboot2_tag_module_t *tmod = (multiboot2_tag_module_t *)tag;

    tmod->type      = MULTIBOOT2_TAGTYPE_MODULE;
    tmod->size      = sizeof(multiboot2_tag_module_t) +
                             strlen(mod->module_name) + 1;
    tmod->mod_start = mod->module_addr;
    tmod->mod_end   = mod->module_addr + mod->module_size;

    strcpy(tmod->name, mod->module_name);

#if (DEBUG_EXEC_MULTIBOOT)
    printf("%p  TAG  3: %s: %p-%p\n", tag, tmod->name, tmod->mod_start, tmod->mod_end);
#endif

    return 0;
}

static int _mboot2_poptag_meminfo(multiboot2_tag_t *tag) {
    multiboot2_tag_basicmem_t *bmem = (multiboot2_tag_basicmem_t *)tag;

    bmem->type = MULTIBOOT2_TAGTYPE_BASIC_MEMINFO;
    bmem->size = sizeof(*bmem);

    bios_call_t call;

    /* Lower memory */

    /* INT 0x12: Get Memory Size */
    call.int_n = 0x12;
    bios_call(&call);

    if(call.eflags & EFLAGS_CF) {
        /* Just assume 640 KiB */
        bmem->size_lower = 640;
    } else {
        bmem->size_lower = call.ax;
    }

    /* Upper memory */

    /* INT 0x15, AH=0x88: Get Extended Memory Size */
    call.int_n = 0x15;
    call.ah    = 0x88;
    bios_call(&call);

    if(call.eflags & EFLAGS_CF) {
        return -1;
    } else {
        bmem->size_upper = call.ax;
    }

#if (DEBUG_EXEC_MULTIBOOT)
    printf("%p  TAG  4: lower: %u, upper: %u\n", tag, bmem->size_lower, bmem->size_upper);
#endif

    return 0;
}

/**
 * @brief Structure of memory entry as used by BIOS INT 0x15, AX=0xE850
 */
typedef struct bios_mmap_entry_struct {
    uint64_t base;   /**< Base address of memory region. */
    uint64_t length; /**< Size of memory region. */
    uint32_t type;   /**< Type of memory region. */
#define BIOS_MMAP_TYPE_AVAILABLE   (1) /**< Available for general use. */
#define BIOS_MMAP_TYPE_RESERVED    (2) /**< Not available for general use. */
#define BIOS_MMAP_TYPE_ACPIRECLAIM (3) /**< ACPI memory, available for general use. */
#define BIOS_MMAP_TYPE_ACPINVS     (4) /**< ACPI memory, must be preserved over sleep. */
} bios_mmap_entry_t;

static int _mboot2_poptag_mmap(multiboot2_tag_t *tag) {
    multiboot2_tag_mmap_t *mmap = (multiboot2_tag_mmap_t *)tag;

    mmap->type          = MULTIBOOT2_TAGTYPE_MMAP;
    mmap->size          = sizeof(*mmap);
    mmap->entry_size    = sizeof(multiboot2_mmap_entry_t);
    mmap->entry_version = 0;

    bios_call_t call;
    memset(&call, 0, sizeof(call));

    /* INT 0x15, AX=0xE820: Get System Memory Map */
    bios_mmap_entry_t *mmap_entry = alloc(sizeof(bios_mmap_entry_t), ALLOC_FLAG_16B);

    int idx = 0;

    call.int_n = 0x15;
    call.ecx   = 0x00000000;
    do {
        call.eax = 0x0000E820;
        call.ecx = sizeof(bios_mmap_entry_t);
        call.edx = 0x534D4150;
        call.di  = (uint16_t)(uintptr_t)mmap_entry;
        bios_call(&call);

        if(call.eflags & EFLAGS_CF) {
            break;
        }

        mmap->entries[idx].base_addr = mmap_entry->base;
        mmap->entries[idx].length    = mmap_entry->length;
        switch(mmap_entry->type) {
            case BIOS_MMAP_TYPE_AVAILABLE:   mmap->entries[idx].type = MULTIBOOT2_MMAP_TYPE_AVAILABLE;
                                             break;
            case BIOS_MMAP_TYPE_ACPIRECLAIM: mmap->entries[idx].type = MULTIBOOT2_MMAP_TYPE_ACPI;
                                             break;
            case BIOS_MMAP_TYPE_ACPINVS:     mmap->entries[idx].type = MULTIBOOT2_MMAP_TYPE_PRESERVE;
                                             break;
            case BIOS_MMAP_TYPE_RESERVED:
            default:                         mmap->entries[idx].type = MULTIBOOT2_MMAP_TYPE_RESERVED;
                                             break;
        }
        
#if (DEBUG_EXEC_MULTIBOOT)
        printf("%p  TAG  6: %16llx, %16llx, %d\n", tag, mmap->entries[idx].base_addr, mmap->entries[idx].length, mmap->entries[idx].type);
#endif

        idx++;
        mmap->size += sizeof(multiboot2_mmap_entry_t);
    } while(call.ebx > 0x00000000);

    free(mmap_entry);

    /* @todo Attempt other methods if the previous is unavailable. */

    return 0;
}

static acpi_rsdp_desc_t *_rsdp = NULL;

static int _mboot_find_acpi_rsdp() {
    void *tst;

    const char *rsdp_sig = ACPI_RSDP_SIGNATURE;

    /* Search region 1: 0x00080000 - 0x0009FFFF (EBDA) */
    for(tst = (void *)0x00080000; tst < (void *)0x000A0000; tst += 16) {
        if(!memcmp(tst, rsdp_sig, 7)) {
            _rsdp = tst;
            return 0;
        }
    }

    /* Search region 2: 0x000E0000 - 0x000FFFFF (BIOS data) */
    for(tst = (void *)0x000E0000; tst < (void *)0x00100000; tst += 16) {
        if(!memcmp(tst, rsdp_sig, 7)) {
            _rsdp = tst;
            return 0;
        }
    }

#if (DEBUG_EXEC_MULTIBOOT)
    printf("No RSDP table found\n");
#endif

    return -1;
}

static int _mboot2_poptag_acpiold(multiboot2_tag_t *tag) {
    if(_rsdp == NULL) {
        /* RSDP could be found already if both tags were requested. */
        if(_mboot_find_acpi_rsdp()) {
            return -1;
        }
    }

    uint8_t checksum = 0;
    for(unsigned i = 0; i < RSDPv1_SIZE; i++) {
        checksum += ((uint8_t *)_rsdp)[i];
    }

    if(checksum != 0) {
#if (DEBUG_EXEC_MULTIBOOT)
        printf("Bad RSDPv1 checksum: %hhu\n", checksum);
#endif
        return -1;
    }

    tag->type = MULTIBOOT2_TAGTYPE_ACPI_OLD;
    tag->size = sizeof(multiboot2_tag_t) + RSDPv1_SIZE;
    memcpy(tag->data, _rsdp, RSDPv1_SIZE);

#if (DEBUG_EXEC_MULTIBOOT)
    printf("%p  TAG 14: %6s, %p\n", tag, _rsdp->oem_id, _rsdp->rsdt_address);
#endif

    return 0;
}

static int _mboot2_poptag_acpinew(multiboot2_tag_t *tag) {
    if(_rsdp == NULL) {
        /* RSDP could be found already if both tags were requested. */
        if(_mboot_find_acpi_rsdp()) {
            return -1;
        }
    }

    if(_rsdp->revision != 2) {
#if (DEBUG_EXEC_MULTIBOOT)
        printf("RSDP is from ACPIv1\n");
#endif
        return -1;
    }

    uint8_t checksum = 0;
    for(unsigned i = 0; i < RSDPv1_SIZE; i++) {
        checksum += ((uint8_t *)_rsdp)[i];
    }

    if(checksum != 0) {
#if (DEBUG_EXEC_MULTIBOOT)
        printf("Bad RSDPv1 checksum: %hhu\n", checksum);
#endif
        return -1;
    }

    for(unsigned i = RSDPv1_SIZE; i < RSDPv2_SIZE; i++) {
        checksum += ((uint8_t *)_rsdp)[i];
    }

    if(checksum != 0) {
#if (DEBUG_EXEC_MULTIBOOT)
        printf("Bad RSDPv2 checksum: %hhu\n", checksum);
#endif
        return -1;
    }

    tag->type = MULTIBOOT2_TAGTYPE_ACPI_NEW;
    tag->size = sizeof(multiboot2_tag_t) + RSDPv2_SIZE;
    memcpy(tag->data, _rsdp, RSDPv2_SIZE);

#if (DEBUG_EXEC_MULTIBOOT)
    printf("%p  TAG 15: %6s, %p, %lp\n", tag, _rsdp->oem_id, _rsdp->rsdt_address, _rsdp->xsdt_addr);
#endif

    return 0;
}
