#include <string.h>

#include "data/fifo.h"
#include "mm/alloc.h"


int fifo_init(fifo_t *fifo, size_t sz) {
    memset(fifo, 0, sizeof(*fifo));
    fifo->buf  = alloc(sz, 0);
    fifo->size = sz;
    return 0;
}

/**
 * @brief Writes byte to FIFO
 *
 * @note Does not check if FIFO has space
 *
 * @param fifo FIFO handle
 * @param byte Byte to write
 */
static inline void _fifo_write_byte(fifo_t *fifo, uint8_t byte) {
    fifo->buf[fifo->tail++] = byte;
    if(fifo->tail >= fifo->size) {
        fifo->tail = 0;
    }
}

/**
 * @brief Reads byte from FIFO
 *
 * @note Does not check if FIFO has data available
 *
 * @param fifo FIFO handle
 * @return Read byte
 */
static inline uint8_t _fifo_read_byte(fifo_t *fifo) {
    uint8_t byte = fifo->buf[fifo->head++];
    if(fifo->head >= fifo->size) {
        fifo->head = 0;
    }
    return byte;
}

int fifo_write(fifo_t *fifo, const void *buf, size_t sz) {
    if(fifo_getfree(fifo) < sz) {
        return -1;
    }

    const uint8_t *buf8 = buf;
    for(size_t i = 0; i < sz; i++) {
        _fifo_write_byte(fifo, buf8[i]);
    }

    return 0;
}

int fifo_read(fifo_t *fifo, void *buf, size_t sz) {
    if(fifo_getused(fifo) < sz) {
        return -1;
    }

    uint8_t *buf8 = buf;
    for(size_t i = 0; i < sz; i++) {
        buf8[i] = _fifo_read_byte(fifo);
    }

    return 0;
}

size_t fifo_getfree(fifo_t *fifo) {
    if(fifo->head <= fifo->tail) {
        return (fifo->size - (fifo->tail - fifo->head)) - 1;
    } else {
        return (fifo->head - fifo->tail) - 1;
    }
}

size_t fifo_getused(fifo_t *fifo) {
    if(fifo->head <= fifo->tail) {
        return fifo->tail - fifo->head;
    } else {
        return fifo->size - (fifo->head - fifo->tail);
    }
}

