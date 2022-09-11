#ifndef LBOOT_DATA_FIFO_H
#define LBOOT_DATA_FIFO_H

#include <stdint.h>

/**
 * @brief FIFO data structure
 */
typedef struct fifo_struct {
             uint8_t *buf;  /**< Pointer to buffer containing FIFO data */
             uint32_t size; /**< Size of FIFO buffer */
    volatile uint32_t head; /**< Location of oldest FIFO entry */
    volatile uint32_t tail; /**< Location of next FIFO entry */
} fifo_t;

/**
 * @brief Initialize FIFO, including allocating buffer
 *
 * @param fifo FIFO handle
 * @param sz Desired size of the FIFO in bytes
 */
int fifo_init(fifo_t *fifo, size_t sz);

/**
 * @brief Checks if FIFO handle has been initialized
 *
 * @param fifo FIFIO handle
 * @return 1 if FIFO handle has been initialized, else 0
 */
static inline int fifo_isinitialized(fifo_t *fifo) {
    return (fifo && fifo->buf && fifo->size);
}

/**
 * @brief Writes data to the FIFO, appending it to the end
 *
 * @note If the entire buf cannot fit into the fifo, no data is written
 *
 * @param fifo FIFO handle
 * @param buf Where to read data from
 * @baram sz Number of bytes to write to the FIFO
 * @return 0 on success, < 0 on failure
 */
int fifo_write(fifo_t *fifo, const void *buf, size_t sz);

/**
 * @brief Reads data from the FIFO
 *
 * @note If the entire requested size is not available, no data is read
 *
 * @param fifo FIFO handle
 * @param buf Where to write data to
 * @baram sz Number of bytes to read from the FIFO
 * @return 0 on success, < 0 on failure
 */
int fifo_read(fifo_t *fifo, void *buf, size_t sz);

/**
 * @brief Gets the number of unused bytes from the FIFO
 *
 * @note Due to the chosen way head/tail operates, the actual usable space will
 * be one less than the allocated size.
 *
 * @return Number of unused bytes
 */
size_t fifo_getfree(fifo_t *fifo);

/**
 * @brief Gets the number of used bytes from the FIFO
 *
 * @return Number of used bytes
 */
size_t fifo_getused(fifo_t *fifo);

#endif

