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

static int _mboot2_poptag_meminfo(multiboot2_tag_t *tag);
static int _mboot2_poptag_mmap(multiboot2_tag_t *tag);

multiboot2_t *multiboot2_parse(const multiboot2_head_t *head) {
    if((head->magic        != MULTIBOOT2_HEAD_MAGIC) ||
       (head->architecture != MULTIBOOT2_HEAD_ARCHITECTURE_X86)) {
        return NULL;
    }

    if((head->magic + head->architecture + head->header_length + head->checksum) != 0) {
        /* Invalid checksum */
        printf("multiboot2_parse: Invalid checksum!\n");
        return NULL;
    }

    multiboot2_t *mboot2 = alloc(MULTIBOOT2_PREALLOC, 0);
    memset(mboot2, 0, MULTIBOOT2_PREALLOC);
    mboot2->size = sizeof(multiboot2_t);

    multiboot2_tag_t *next_tag = (multiboot2_tag_t *)&mboot2->tags;

    const multiboot2_tag_t *htag = (multiboot2_tag_t *)&head->tags;
    while(htag < (multiboot2_tag_t *)(head + head->header_length)) {
        printf("tag: %d\n", htag->type);
        if(htag->type == MULTIBOOT2_HEADERTAG_END) {
            break;
        }

        switch(htag->type) {
            case MULTIBOOT2_HEADERTAG_INFORMATION_REQUEST: {
                uint32_t *reqs = (uint32_t *)&htag->data;
                for(unsigned i = 0; i < (htag->size - 8) / 4; i++) {
                    switch(reqs[i]) {
                        case MULTIBOOT2_TAGTYPE_CMDLINE:
                            /* @todo Cmdline support*/
                            break;
                        case MULTIBOOT2_TAGTYPE_BOOTLOADER_NAME:
                            next_tag->type = MULTIBOOT2_TAGTYPE_BOOTLOADER_NAME;
                            next_tag->size = sizeof(multiboot2_tag_t) + strlen(_bootloader_name) + 1;
                            strcpy((char *)&next_tag->data, _bootloader_name);
                            next_tag = NEXT_TAG(next_tag);
                            break;
                        case MULTIBOOT2_TAGTYPE_MODULE:
                            /* @todo Module support */
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
                            /* @todo */
                            break;
                        case MULTIBOOT2_TAGTYPE_ACPI_NEW:
                            /* @todo */
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

    return mboot2;

mboot2_parse_error:
    free(mboot2);

    return NULL;
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

    /* @todo Check for 15MiB memory hole, and check other BIOS function availability. */

    printf("  TAG  4: lower: %u, upper: %u\n", bmem->size_lower, bmem->size_upper);

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
        
        printf("  TAG  6: %16llx, %16llx, %d\n", mmap->entries[idx].base_addr, mmap->entries[idx].length, mmap->entries[idx].type);

        idx++;
        mmap->size += sizeof(multiboot2_mmap_entry_t);
    } while(call.ebx > 0x00000000);

    free(mmap_entry);

    /* @todo Attempt other methods if the previous is unavailable. */

    return 0;
}
