#ifndef MM_ALLOC_H
#define MM_ALLOC_H

#include <stdint.h>

#define ALLOCENT_FLAG_VALID (1U << 0) /**< Alloc entry is valid */
#define ALLOCENT_FLAG_USED  (1U << 1) /**< Alloc entry is used */

/**
 * @brief Describes a block of memory for allocation
 */
typedef struct {
	uint32_t  flags; /**< Flags */
	uintptr_t addr;  /**< The address of the block of memory */
	uintptr_t size;  /**< Size of the block of memory */
} alloc_ent_t;

#define ALLOC_BLOCK_SZ (128) /**< Entries per allocation block */

typedef struct alloc_block_struct alloc_block_t;
/**
 * @brief Allocation block containing allocation entries.
 */
struct alloc_block_struct {
    alloc_ent_t    entries[ALLOC_BLOCK_SZ]; /**< Array of allocation entries */
    alloc_block_t *next;                    /**< Pointer to next allocation block, or NULL */
};

/**
 * @brief Memory range allowed for a requested allocation.
 */
typedef struct {
    uintptr_t start; /**< Start of memory rangeo (inclusive) */
    uintptr_t end;   /**< End of memory range (inclusive) */
} alloc_memrange_t;

/**
 * @brief Initialize allocation functions.
 *
 * @param base location of usable memory
 * @param size size of usable memory
 */
void alloc_init(uint32_t base, uint32_t size);

#define ALLOC_FLAG_ALIGN(n)   ((((n) > 15) ? 15 : (n)) | ALLOC_FLAG_SETALIGN) /**< Set alignment to 2^n */
#define ALLOC_FLAG_SETALIGN   (1UL <<  4) /**< Use alignment in the first lower 4 bits */
#define ALLOC_FLAG_16B        (1UL <<  5) /**< Require memory < 0x10000. If not set, returns memory >= 0x10000. */
#define ALLOC_FLAG_NONEWBLOCK (1UL << 31) /**< INTERNAL ONLY. If set, do not attempt to allocate a new block if below free threshold. */

/**
 * @brief Allocates a block of memory with requested options
 *
 * @param sz    Size of the required memory block
 * @param flags Flags affecting operation, see MALLOC_FLAG_*
 * @returns pointer to memory block on success, else NULL
 */
void *alloc(size_t sz, uint32_t flags);

/**
 * @brief Frees a previously allocated block of memory.
 * 
 * @param ptr Pointer to previously-allocated memory block.
 */
void free(void *ptr);

#endif

