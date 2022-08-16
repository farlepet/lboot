#ifndef LBOOT_IO_VGA_H
#define LBOOT_IO_VGA_H

#include <stdint.h>

#include "io/output.h"

/**
 * @brief Initialize VGA driver
 *
 * @param out Output device handle to populate
 */
int vga_init(output_hand_t *out);

#endif

