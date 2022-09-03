#ifndef LBOOT_EXEC_EXEC_H
#define LBOOT_EXEC_EXEC_H

#include <stdint.h>

#include "config/config_types.h"
#include "exec/multiboot_types.h"
#include "storage/fs/fs.h"

#define EXEC_FIRSTCHUNK_SZ (512) /**< Size of first chunk to read when determining file type. */

typedef struct exec_hand_struct exec_hand_t;

/**
 * @brief Executable file format enumeration
 */
typedef enum exec_filefmt_enum {
    EXEC_FILEFMT_NONE = 0, /**< None/unknown */
    EXEC_FILEFMT_FLAT,     /**< Flat binary w/o container */
    EXEC_FILEFMT_ELF       /**< ELF binary format */
} exec_filefmt_e;

/**
 * @brief Executable handle
 */
struct exec_hand_struct {
    exec_filefmt_e     fmt;        /**< Format executable is in. */
    void              *data;       /**< Data for use by particular exec handler. */

    uintptr_t          data_begin; /**< First address of data loaded from file. */
    uintptr_t          data_end;   /**< Last address + 1 of data loaded from file. */
    uintptr_t          entrypoint; /**< Address of executable entrypoint. (phys == virt) */

    multiboot2_head_t *multiboot;  /**< Location of multiboot header, if applicable */

    file_hand_t       *file;       /**< File containing executable */

    /**
     * @brief Grab executable information from file, and determine placement
     * (if applicable).
     *
     * @param exec Exec handle
     * @return 0 on success, < 0 on failure.
     */
    int (*prepare)(exec_hand_t *exec);

    /**
     * @brief Load executable into memory.
     *
     * @note For now, we can only load into identity-mapped memory, with the
     * assumption that the loaded kernel will wipe out the page table without
     * reading it first.
     *
     * @param exec Exec handle
     * @return 0 on success, < 0 on failure.
     */   
    int (*load)(exec_hand_t *exec);
};

/**
 * @brief Create exec handle based on file type passed in
 *
 * @param exec Exec handle to be populated
 * @param file File to be executed
 * @return 0 on success, < 0 on failure.
 */
int exec_open(exec_hand_t *exec, file_hand_t *file);

/**
 * @brief Execute loaded binary.
 *
 * @param exec Exec handle
 * @param cfg Configuration
 * @return no-return on success, < 0 on failure.
 */
int exec_exec(exec_hand_t *exec, config_data_t *cfg);

#endif

