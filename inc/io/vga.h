#ifndef LBOOT_IO_VGA_H
#define LBOOT_IO_VGA_H

#include <stdint.h>

/**
 * @brief Initialize VGA driver
 */
int vga_init(void);

/**
 * @brief Clears the screen
 */
void vga_clear(void);

/**
 * @brief Print a character to the screen
 *
 * @param ch Character to print
 */
void vga_putchar(char ch);

/**
 * @brief Print a string to the screen
 *
 * @param str String to pring
 */
void vga_puts(const char *str);

#endif

