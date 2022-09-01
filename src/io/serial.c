#include <string.h>

#include "io/ioport.h"
#include "io/serial.h"
#include "mm/alloc.h"

typedef struct {
    uint16_t port;
} serial_data_t;

static ssize_t _serial_write(output_hand_t *out, const void *data, size_t sz);

int serial_init(output_hand_t *out, uint16_t port) {
    /* Disable interrupts */
    outb(SERIAL_REG_IER(port), 0x00);

    /* Set 115200 baud. @todo Allow configuration */
    outb(SERIAL_REG_LCR(port), (1U << SERIALREG_LCR_DLAB__POS));
    outb(SERIAL_REG_DLL(port), 0x01);
    outb(SERIAL_REG_DLM(port), 0x00);

    /* 8n1 */
    outb(SERIAL_REG_LCR(port), (SERIALREG_LCR_WORDLEN_8BIT << SERIALREG_LCR_WORDLEN__POS));

    /* Enable FIFO */
    outb(SERIAL_REG_FCR(port), (1U << SERIALREG_FCR_FIFOEN__POS)    |
                               (1U << SERIALREG_FCR_RXFIFORST__POS) |
                               (1U << SERIALREG_FCR_TXFIFORST__POS) |
                               (2U << SERIALREG_FCR_TRIGLVL__POS));

    outb(SERIAL_REG_MCR(port), (1U << SERIALREG_MCR_DTR__POS) |
                               (1U << SERIALREG_MCR_RTS__POS) |
                               (1U << SERIALREG_MCR_OUT1__POS) |
                               (1U << SERIALREG_MCR_OUT2__POS));

    memset(out, 0, sizeof(output_hand_t));
    out->write = _serial_write;

    /* Perhaps excessive to allocate this, but we may want to add more in the
     * future. */
    serial_data_t *sdata = (serial_data_t *)alloc(sizeof(serial_data_t), 0);
    sdata->port = port;
    out->data = sdata;

    return 0;
}


static void _serial_putchar(const serial_data_t *sdata, uint8_t ch) {
    while(!(inb(SERIAL_REG_LSR(sdata->port)) & (1U << SERIALREG_LSR_THRE__POS))) {}

    outb(SERIAL_REG_DATA(sdata->port), ch);
}

static ssize_t _serial_write(output_hand_t *out, const void *data, size_t sz) {
    const serial_data_t *sdata = out->data;

    const uint8_t *bdata = data;

    for(size_t i = 0; i < sz; i++) {
        _serial_putchar(sdata, bdata[i]);
    }

    return sz;
}

