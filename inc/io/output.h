#ifndef LBOOT_IO_OUTPUT_H
#define LBOOT_IO_OUTPUT_H

#include <stdint.h>

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

#endif

