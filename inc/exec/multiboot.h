#ifndef LBOOT_EXEC_MULTIBOOT_H
#define LBOOT_EXEC_MULTIBOOT_H

#include "config/config_types.h"
#include "exec/multiboot_types.h"

/**
 * @brief Parse Multiboot 2 header and generate Multiboot 2 structure to pass
 * to the kernel.
 *
 * @param head Pointer to kernel's Multiboot 2 header
 * @param cfg Configuration
 * @return Pointer to Multiboot 2 structure to pass to kernel, NULL on error
 */
multiboot2_t *multiboot2_parse(const multiboot2_head_t *head, const config_data_t *cfg);

#endif

