#ifndef LBOOT_IO_OUTPUT_H
#define LBOOT_IO_OUTPUT_H

#include <stdint.h>

#define VERBOSE_PANIC (1)

typedef struct output_hand_struct output_hand_t;

/**
 * @brief Output device handle
 */
struct output_hand_struct {
    void *data; /**< Pointer to output-specific data */

    /**
     * @brief Pass a single character to the output device
     *
     * @note While passing a single character at a time to a function pointer
     * may not be efficient, currently there is no high demand placed on the
     * output device. Perhaps this will change with the addition of XMODEM.
     *
     * @param out Output handle
     * @param ch Character
     */
    void (*putchar)(output_hand_t *out, char ch);
};

/**
 * @brief Set the currently selected output driver
 *
 * @param output Output driver to select
 */
void output_set(output_hand_t *output);

/**
 * @brief Sends a single character to the current output device
 *
 * @param ch Character to send
 */
void putchar(char ch);

/**
 * @brief Sends a (NULL-terminated) string of characters to the current
 * output device
 *
 * @param str NULL-terminated string
 */
void puts(const char *str);

/**
 * @brief Send formatted output to the current output device
 *
 * @param fmt Format string
 * @return int Number of characters sent
 */
int printf(const char *fmt, ...);

/**
 * @brief Send message and halt execution
 *
 * @param fmt Format string
 */
void _panic(const char *fmt, ...);

#if (VERBOSE_PANIC)
#  define __panic_stringify1(X) #X
#  define __panic_stringify2(X) __panic_stringify1(X)

#  define panic(...) _panic("\nPANIC at " __FILE__ ":" __panic_stringify2(__LINE__) ":\n  " __VA_ARGS__)
#else
#  define panic _panic
#endif

/**
 * @brief Print data has hex
 *
 * @param data Data to print
 * @param len Length of data in bytes
 */
void print_hex(const void *data, size_t len);

#endif

