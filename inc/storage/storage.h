#ifndef LBOOT_STORAGE_STORAGE_H
#define LBOOT_STORAGE_STORAGE_H

#include <stdint.h>

typedef struct storage_hand_struct storage_hand_t;

/**
 * @brief Storage device handle
 */
struct storage_hand_struct {
    size_t size; /**< Size of storage device, in bytes */

    void  *data; /**< Pointer to data needed by storage driver */

    /**
     * @brief Read bytes from storage device
     *
     * @param buff Buffer to read bytes into
     * @param offset Offset into storage device to start reading
     * @param size Number of bytes to read
     * @return ssize_t Number of bytes successfully read, or < 0 on error
     */
    ssize_t (*read)(storage_hand_t *storage, void *buff, off_t offset, size_t size);
};

#endif

