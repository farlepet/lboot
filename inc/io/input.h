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
     * @brief Read data from input
     *
     * @note If an overflow/overrun condition occurs during the read, only the
     * data that is currently available will be copied into data, and this will
     * be reflected by the return value. This data is not guaranteed to be
     * intact, as the overrun could have occured in the middle. This also
     * applies to data errors. If data is NULL however, the read will not be
     * interrupted on these errors.
     *
     * @param in Input device handle
     * @param data Buffer in which to store data, NULL to throw away data
     * @parma sz Maximum number of bytes to read
     * @param timeout Maximum timeout to receive data in milliseconds
     * @return Number of bytes read on success, < 0 on error
     */
    ssize_t (*read)(input_hand_t *in, void *data, size_t sz, uint32_t timeout);

    uint8_t status; /**< Status flags, only guaranteed to be updated on call to read. Error flags are cleared at the beginning of the read call. */
#define INPUTSTATUS_OVERRUN (1U << 0) /**< A buffer overrun has occured, and not all data was captured. */
#define INPUTSTATUS_DATAERR (1U << 0) /**< A data error (e.g. parity) occured during the read. */
};

#endif

