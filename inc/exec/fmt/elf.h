#ifndef LBOOT_EXEC_FMT_ELF_H
#define LBOOT_EXEC_FMT_ELF_H

#include "exec/exec.h"

/**
 * @brief Test if file is in the ELF format.
 *
 * @param exec Exec handle
 * @param first_chunk Buffer containing first chunk of the file
 * @return 1 if file is in ELF format, else 0
 */
int exec_elf_test(exec_hand_t *exec, void *first_chunk);

/**
 * @brief Populate exec handle with information needed by the ELF loader.
 *
 * @param exec Exec handle to populate
 * @return 0 on success, < 0 on failure
 */
int exec_elf_init(exec_hand_t *exec);

#define ELF_IDENT (0x464c457fUL) /**< ELF magic number - 0x7f + "ELF" */

#endif

