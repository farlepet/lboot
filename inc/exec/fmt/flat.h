#ifndef LBOOT_EXEC_FMT_FLAT_H
#define LBOOT_EXEC_FMT_FLAT_H

#include "exec/exec.h"

/**
 * @brief Populate exec handle with information needed by the FLAT loader.
 *
 * @param exec Exec handle to populate
 * @return 0 on success, < 0 on failure
 */
int exec_flat_init(exec_hand_t *exec);

#endif

