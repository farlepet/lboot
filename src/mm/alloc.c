#include <string.h>
#include <stddef.h>

#include "mm/alloc.h"
#include "io/output.h"

static alloc_block_t *_first_block = NULL;

#define MIN_ALIGN 4

static void _add_alloc(alloc_ent_t *al);

void alloc_init(uint32_t base, uint32_t size) {
    uint32_t newbase = base;
    if(newbase % 4) {
        newbase += 4 - (newbase % 4);
    }

    printf("Available Low Memory:\n"
           "  < 64 KiB: %3u KiB\n"
           "  > 64 KiB: %3u KiB\n\n",
           (0x10000 - base) / 1024,
           (size - (0x10000 - base)) / 1024);

    _first_block = (alloc_block_t *)newbase;
    newbase += sizeof(alloc_block_t);
    memset(_first_block, 0, sizeof(alloc_block_t));

    size -= (newbase - base);

    alloc_ent_t ae = { .flags = ALLOCENT_FLAG_VALID, .addr = newbase, .size = size };

    // Add a free allocation block encompasing all the usable memory
    _add_alloc(&ae);
}

/**
 * @brief Add an allocation entry to the first free slot
 *
 * @param al Allocation entry to add
 */
static void _add_alloc(alloc_ent_t *al) {
    alloc_block_t *block = _first_block;

    /* @todo Allocate new block if we are getting close to full */
    while(block) {
        for(unsigned i = 0; i < ALLOC_BLOCK_SZ; i++) {
            if(!(block->entries[i].flags & ALLOCENT_FLAG_VALID)) {
                memcpy(&block->entries[i], al, sizeof(alloc_ent_t));
                return;
            }
        }
        block = block->next;
    }

    panic("No available allocation entries!");
}

/**
 * @brief Add new allocation block to chain.
 *
 * @param new Block to add
 */
static void _add_alloc_block(alloc_block_t *new) {
    alloc_block_t *block = _first_block;
    while(block->next) {
        block = block->next;
    }
    block->next = new;
}

/**
 * @brief Attempt to allocate the most effecient space for the given number of,
 * with the requested stipulations.
 *
 * @param sz Size of memory requested in bytes
 * @param flags Flags affecting allocation
 * @param mrange Memory range acceptable for allocation
 * @return void* Pointer to base of allocated region, or NULL on failure
 */
static void *_find_free(size_t sz, uint32_t flags, alloc_memrange_t *mrange) {
    alloc_block_t *block  = _first_block;
    alloc_ent_t   *entry  = NULL;
    alloc_ent_t    newent = { .flags = ALLOCENT_FLAG_VALID | ALLOCENT_FLAG_USED, .addr = 0, .size = sz };

    unsigned align = MIN_ALIGN;
    if(flags & ALLOC_FLAG_SETALIGN) {
        align = (1UL << (flags & 0x0f));
    }

    unsigned free_ents = 0;

    while(block) {
        for(unsigned i = 0; i < ALLOC_BLOCK_SZ; i++) {
            alloc_ent_t *ent = &block->entries[i];
            if( (ent->flags & ALLOCENT_FLAG_VALID) &&
               !(ent->flags & ALLOCENT_FLAG_USED)) {
                uintptr_t start = ent->addr;
                uintptr_t end   = start + ent->size;

                /* Force into required memory region */
                if(start < mrange->start) {
                    start = mrange->start;
                }
                if(end > mrange->end) {
                    end = mrange->end;
                }

                if(start % align) {
                    start += align - (start % align);
                }

                /* Check size. Start can increase beyond end in the case of alignment. */
                if((start > end) || ((end - start) < sz)) {
                    continue;
                }
                
                if(entry) {
                    /* Smallest entry wins.
                     * @todo Possibly take alignment into account as well, to
                     * reduce the number of small unallocated blocks. */
                    if(ent->size < entry->size) {
                        entry       = ent;
                        newent.addr = start;
                    }
                } else {
                    entry       = ent;
                    newent.addr = start;
                }
            } else if(!(ent->flags & ALLOCENT_FLAG_VALID)) {
                free_ents++;
            }
        }
        block = block->next;
    }

    if((free_ents < (ALLOC_BLOCK_SZ * 0.7)) &&
       !(flags & ALLOC_FLAG_NONEWBLOCK)) {
        alloc_block_t *newblock = (alloc_block_t *)alloc(sizeof(alloc_block_t), ALLOC_FLAG_NONEWBLOCK);
        if(newblock == NULL) {
            panic("Could not allocate a new allocation block!");
        }
        memset(newblock, 0, sizeof(alloc_block_t));
        _add_alloc_block(newblock);
        /* @todo The previously selected entry may have been touched, best to
         * go though again. */
        return _find_free(sz, flags, mrange);
    }

    if(entry == NULL) {
        return NULL;
    }

    void *addr = NULL;

    if(entry->addr != newent.addr) {
        /* We need to create a prior entry */
        alloc_ent_t pre = {
            .flags = ALLOCENT_FLAG_VALID,
            .addr  = entry->addr,
            .size  = (newent.addr - entry->addr)
        };
        _add_alloc(&pre);

        entry->addr  = newent.addr;
        entry->size -= pre.size;
    }

    if(entry->size == newent.size) {
        entry->flags |= ALLOCENT_FLAG_USED;
        addr = (void *)entry->addr;
    } else {
        /* Need to add a subsequent entry */
        entry->addr += newent.size;
        entry->size -= newent.size;
        _add_alloc(&newent);
        addr = (void *)newent.addr;
    }

    return addr;
}

static alloc_memrange_t _mrange_16b = { .start = 0x00000000, .end = 0x0000FFFF };
static alloc_memrange_t _mrange_32b = { .start = 0x00010000, .end = 0xFFFFFFFF };

void *alloc(size_t sz, uint32_t flags) {
    if(!sz) {
        panic("Attempt to alloc 0 bytes!");
    }

    alloc_memrange_t *mrange = (flags & ALLOC_FLAG_16B) ? &_mrange_16b : &_mrange_32b;

    void *addr = _find_free(sz, flags, mrange);

    if(addr == NULL) {
        /* For now, assuming any failure to allocate is critical. */
        panic("alloc(%d, %x): Could not allocate memory!", sz, flags);
    }

    return addr;
}

/**
 * @brief Attempt to free an allocation representing the given address
 *
 * Will also join neighbor unused allocation entries into a single entry.
 *
 * @param addr Address to free
 * @return int 0 on success, non-zero on error
 */
static int _do_free(uintptr_t addr) {
    alloc_ent_t *pre  = NULL;  /* Closest allocation prior */
    alloc_ent_t *ent  = NULL;  /* Matching allocation */
    alloc_ent_t *post = NULL; /* Closest allocation after */

    alloc_block_t *block  = _first_block;

    /* Not a very effecient system, but it works. */
    while(block) {
        for(unsigned i = 0; i < ALLOC_BLOCK_SZ; i++) {
            alloc_ent_t *_ent = &block->entries[i];
            if(_ent->flags & ALLOCENT_FLAG_VALID) {
                if(_ent->flags & ALLOCENT_FLAG_USED) {
                    if(!ent && (_ent->addr == addr)) {
                        /* Found the corrent entry */
                        ent = _ent;
                    }
                } else {
                    if((pre == NULL) && ((_ent->addr + _ent->size) == addr)) {
                        /* Found preceeding entry */
                        pre = _ent;
                    } else if (_ent->addr > addr) {
                        /* We might not know the size yet, so we need to just
                         * try to find the closest one */
                        if(post == NULL) {
                            post = _ent;
                        } else if (_ent->addr < post->addr) {
                            post = _ent;
                        }
                    }
                }
            }
        }
        block = block->next;
    }

    if(ent == NULL) {
        return -1;
    }

    if(pre) {
        ent->addr  = pre->addr;
        ent->size += pre->size;
        pre->flags = 0;
    }
    if(post && ((ent->addr + ent->size) == post->addr)) {
        ent->size += post->size;
        post->flags = 0;
    }

    ent->flags &= ~ALLOCENT_FLAG_USED;

    return 0;
}

void free(void *ptr) {
    if(_do_free((uintptr_t)ptr)) {
        panic("Attempt to free address not previously allocated: %p", ptr);
    }
}

