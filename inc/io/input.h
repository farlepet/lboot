#ifndef LBOOT_IO_INPUT_H
#define LBOOT_IO_INPUT_H

#include <stdint.h>

typedef struct input_hand_struct input_hand_t;

/**
 * @brief Input device handle
 */
struct input_hand_struct {
    void *data; /**< Pointer to data needed by input device */

    /**
     * @brief Read date from input
     *
     * @param in Input device handle
     * @param data Buffer in which to store data
     * @parma sz Maximum number of bytes to read
     * @param timeout Maximum timeout to receive data in milliseconds
     * @return Number of bytes read on success, < 0 on error
     */
    ssize_t (*read)(input_hand_t *in, void *data, size_t sz, uint32_t timeout);
};

#endif

