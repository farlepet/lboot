#include <stddef.h>
#include <string.h>

#include "data/fifo.h"
#include "intr/interrupts.h"
#include "io/ioport.h"
#include "io/serial.h"
#include "mm/alloc.h"
#include "time/time.h"

typedef struct {
    uint32_t cfg;     /**< Copy of config value passed in init */
    uint16_t port;    /**< Base IO port of serial port */
    uint8_t  mcr_val; /**< Current value of MCR register, for flow control */
    fifo_t   rxfifo;  /**< Serial input buffer */
    fifo_t   txfifo;  /**< Serial output buffer */
} serial_data_t;

/*
 * To facilitate transfer of control between general purpose i/o and serial
 * data transfer protocols, allocated serial data structures are shared between
 * any i/o structure using the same port. Since this is (currently) not a
 * multitasking bootloader, this should not cause any issues.
 */
#define MAX_PORT_COUNT (2)
static serial_data_t *_port_data[MAX_PORT_COUNT];

static ssize_t _serial_write(output_hand_t *out, const void *data, size_t sz);
static ssize_t _serial_read(input_hand_t *in, void *data, size_t sz, uint32_t timeout);
static void    _serial_int_handler(uint8_t int_n, uint32_t errno, void *data);

/**
 * @brief Find matching _port_data entry, or create a new one if not found
 *
 * @param port Base I/O port address for serial port
 * @return Pointer to serial_data_t structure, or NULL of error
 */
static inline serial_data_t *_serial_find_portdata(uint16_t port) {
    for(unsigned i = 0; i < MAX_PORT_COUNT; i++) {
        if(_port_data[i] &&
           (_port_data[i]->port == port)) {
            return _port_data[i];
        }
    }

    /* Not found */
    for(unsigned i = 0; i < MAX_PORT_COUNT; i++) {
        if(_port_data[i] == NULL) {
            _port_data[i] = alloc(sizeof(serial_data_t), 0);
            memset(_port_data[i], 0, sizeof(*_port_data[0]));
            _port_data[i]->port = port;
            return _port_data[i];
        }
    }

    /* No free entries */
    return NULL;
}

int serial_init(input_hand_t *in, output_hand_t *out, uint32_t baud, uint32_t cfg) {
    if((baud > 115200) || (baud < 50)) {
        return -1;
    }

    uint16_t port = 0;
    int_id_e int_id;

    /* @todo Get actual bases from BIOS Data Area */
    switch(cfg & SERIAL_CFG_PORT__MSK) {
        case SERIAL_CFG_PORT_COM1:
            port   = 0x03f8;
            int_id = INT_ID_COM1;
            break;
        case SERIAL_CFG_PORT_COM2:
            port   = 0x02f8;
            int_id = INT_ID_COM2;
            break;
        case SERIAL_CFG_PORT_COM3:
            port   = 0x03e8;
            int_id = INT_ID_COM1;
            break;
        case SERIAL_CFG_PORT_COM4:
            port   = 0x02e8;
            int_id = INT_ID_COM2;
            break;
        default:
            return -1;
    }

    /* Disable interrupts */
    outb(SERIAL_REG_IER(port), 0x00);

    {
        /* Assumes standard baud rate, otherwise it may not necessarially find
         * the closest one. */
        uint16_t div = 115200 / baud;


        outb(SERIAL_REG_LCR(port), (1U << SERIALREG_LCR_DLAB__POS));
        outb(SERIAL_REG_DLL(port), (uint8_t)div);
        outb(SERIAL_REG_DLM(port), (uint8_t)(div >> 8));
    }

    /* 8n1 */
    outb(SERIAL_REG_LCR(port), (SERIALREG_LCR_WORDLEN_8BIT << SERIALREG_LCR_WORDLEN__POS));

    /* Disable FIFO to simplify SW FIFO */
    outb(SERIAL_REG_FCR(port), (0U << SERIALREG_FCR_FIFOEN__POS)    |
                               (1U << SERIALREG_FCR_RXFIFORST__POS) |
                               (1U << SERIALREG_FCR_TXFIFORST__POS) |
                               (0U << SERIALREG_FCR_TRIGLVL__POS));

    outb(SERIAL_REG_MCR(port), (1U << SERIALREG_MCR_DTR__POS) |
                               (1U << SERIALREG_MCR_RTS__POS) |
                               (1U << SERIALREG_MCR_OUT1__POS) |
                               (1U << SERIALREG_MCR_OUT2__POS));

    serial_data_t *sdata = _serial_find_portdata(port);
    if(sdata == NULL) {
        panic("serial: Attempt to use more than %u serial ports", MAX_PORT_COUNT);
    }

    sdata->cfg = cfg;

    uint8_t int_en = 0x00;

    if(in) {
        memset(in, 0, sizeof(*in));
        in->read = _serial_read;
        in->data = sdata;
        /* Currently, once a FIFO is created, it cannot be modified. */
        if(!fifo_isinitialized(&sdata->rxfifo)) {
            uint8_t in_sz = (cfg >> SERIAL_CFG_INBUFFSZ__POS) & SERIAL_CFG_INBUFFSZ__MSK;
            if(in_sz) {
                fifo_init(&sdata->rxfifo, 1UL << in_sz);
                /* Enable RX data available interrupt */
                int_en |= 1U << SERIALREG_IER_RXAVAIL__POS;
            }
        } else {
            /* Even if not requested, once a FIFO always a FIFO */
            int_en |= 1U << SERIALREG_IER_RXAVAIL__POS;
        }
    }

    if(out) {
        memset(out, 0, sizeof(*out));
        out->write = _serial_write;
        out->data  = sdata;
        if(!fifo_isinitialized(&sdata->txfifo)) {
            uint8_t out_sz = (cfg >> SERIAL_CFG_OUTBUFFSZ__POS) & SERIAL_CFG_OUTBUFFSZ__MSK;
            if(out_sz) {
                fifo_init(&sdata->txfifo, 1UL << out_sz);
                /* Enable TX buffer empty interrupt */
                int_en |= 1U << SERIALREG_IER_TXEMPTY__POS;
            }
        } else {
            /* Even if not requested, once a FIFO always a FIFO */
            int_en |= 1U << SERIALREG_IER_TXEMPTY__POS;
        }
    }

    if(int_en) {
        /* @todo Register interrupt */
        interrupt_register(int_id, _serial_int_handler, sdata);
        interrupt_enable(int_id);
        outb(SERIAL_REG_IER(port), int_en);
    }

    sdata->mcr_val = inb(SERIAL_REG_MCR(sdata->port));

    return 0;
}

static inline int _serial_tx_empty(const serial_data_t *sdata) {
    return inb(SERIAL_REG_LSR(sdata->port)) & (1U << SERIALREG_LSR_THRE__POS);
}

static void _serial_write_byte(const serial_data_t *sdata, uint8_t byte) {
    while(!_serial_tx_empty(sdata)) {
        /* Busy-wait */
    }

    outb(SERIAL_REG_DATA(sdata->port), byte);
}

static ssize_t _serial_write(output_hand_t *out, const void *data, size_t sz) {
    serial_data_t *sdata = out->data;
    const uint8_t *bdata = data;

    /* @todo Remove support for operation without FIFOs */

    if(interrupts_enabled() &&
       fifo_isinitialized(&sdata->txfifo)) {
        size_t i = 0;

        while(i < sz) {
            size_t wr_sz = fifo_getfree(&sdata->txfifo);
            if(wr_sz > (sz - i)) {
                wr_sz = sz - i;
            }
            if(wr_sz) {
                fifo_write(&sdata->txfifo, &bdata[i], wr_sz);
                /* Assume write succeeds */
                i += wr_sz;
            }

            if(_serial_tx_empty(sdata)) {
                /* Interrupt must have occured while FIFO was empty */
                uint8_t byte;
                if(!fifo_read(&sdata->txfifo, &byte, 1)) {
                    _serial_write_byte(sdata, byte);
                }
            }
        }
    } else {
        if(fifo_isinitialized(&sdata->txfifo)) {
            /* Flush the FIFO first */
            uint8_t byte;
            while(!fifo_read(&sdata->txfifo, &byte, 1)) {
                _serial_write_byte(sdata, byte);
            }
        }

        for(size_t i = 0; i < sz; i++) {
            _serial_write_byte(sdata, bdata[i]);
        }
    }

    return sz;
}

static inline int _serial_rx_dataready(const serial_data_t *sdata) {
    return inb(SERIAL_REG_LSR(sdata->port)) & (1U << SERIALREG_LSR_DR__POS);
}

static uint8_t _serial_read_byte(const serial_data_t *sdata) {
    while(!_serial_rx_dataready(sdata)) {
        /* Busy-wait */
    }

    return inb(SERIAL_REG_DATA(sdata->port));
}

static inline void _serial_rx_flowcontrol(serial_data_t *sdata) {
    if(!(sdata->cfg & ((1UL << SERIAL_CFG_FLOWCTRL_RTS__POS) |
                       (1UL << SERIAL_CFG_FLOWCTRL_DTR__POS)))) {
        return;
    }

    uint16_t new_mcr = sdata->mcr_val;

    if(fifo_getfree(&sdata->rxfifo) < ((sdata->rxfifo.size < 8) ? (sdata->rxfifo.size - 1) :
                                                                  (sdata->rxfifo.size - 3))) {
        /* De-assert RTS */
        if((sdata->mcr_val & (1U << SERIALREG_MCR_RTS__POS)) &&
           (sdata->cfg & (1UL << SERIAL_CFG_FLOWCTRL_RTS__POS))) {
            new_mcr &= ~(1U << SERIALREG_MCR_RTS__POS);
        }
        /* De-assert DTR */
        if((sdata->mcr_val & (1U << SERIALREG_MCR_DTR__POS)) &&
           (sdata->cfg & (1UL << SERIAL_CFG_FLOWCTRL_DTR__POS))) {
            new_mcr &= ~(1U << SERIALREG_MCR_DTR__POS);
        }
    } else {
        /* Assert RTS */
        if(!(sdata->mcr_val & (1U << SERIALREG_MCR_RTS__POS)) &&
           (sdata->cfg & (1UL << SERIAL_CFG_FLOWCTRL_RTS__POS))) {
            new_mcr |= (1U << SERIALREG_MCR_RTS__POS);
        }
        /* Assert DTR */
        if(!(sdata->mcr_val & (1U << SERIALREG_MCR_DTR__POS)) &&
           (sdata->cfg & (1UL << SERIAL_CFG_FLOWCTRL_DTR__POS))) {
            new_mcr |= (1U << SERIALREG_MCR_DTR__POS);
        }
    }

    if(new_mcr != sdata->mcr_val) {
        outb(SERIAL_REG_MCR(sdata->port), sdata->mcr_val);
    }
}

static ssize_t _serial_read(input_hand_t *in, void *data, size_t sz, uint32_t timeout) {
    serial_data_t *sdata = in->data;
    uint8_t       *bdata = data;

    if(timeout && !interrupts_enabled()) {
        /* Interrupts are need for timeout handling */
        panic("_serial_read called with interrupts disabled");
    }

    time_ticks_t timeout_end;
    time_offset(&timeout_end, timeout);

    size_t i = 0;
    while(i < sz) {
        if(interrupts_enabled() &&
           fifo_isinitialized(&sdata->rxfifo)) {
            size_t rd_sz = fifo_getused(&sdata->rxfifo);
            if(rd_sz > (sz - i)) {
                rd_sz = sz - i;
            }
            if(rd_sz) {
                fifo_read(&sdata->rxfifo, &bdata[i], rd_sz);
                _serial_rx_flowcontrol(sdata);
                i += rd_sz;
            }
        } else {
            if(fifo_isinitialized(&sdata->txfifo) &&
               fifo_getused(&sdata->rxfifo)) {
                /* Read FIFO first */
                size_t rd_sz = fifo_getused(&sdata->rxfifo);
                if(rd_sz > (sz - i)) {
                    rd_sz = sz - i;
                }
                fifo_read(&sdata->rxfifo, &bdata[i], rd_sz);
                _serial_rx_flowcontrol(sdata);
            } else if(_serial_rx_dataready(sdata)) {
                bdata[i++] = _serial_read_byte(sdata);
            }
        }

        if(timeout && time_ispast(&timeout_end)) {
            /* @todo Should we also allow for inter-byte timeouts, and the
             * supplied timeout is only for the first byte? */
            break;
        }
    }

    return i;
}

static void _serial_int_handler(uint8_t int_n, uint32_t errno, void *data) {
    (void)int_n;
    (void)errno;

    /* @note Currently does not support concurrent use of COM1/COM3 or COM2/COM4 */

    serial_data_t *sdata = data;

    uint8_t iir = inb(SERIAL_REG_IIR(sdata->port));
    switch((iir >> SERIALREG_IIR_INTID__POS) & SERIALREG_IIR_INTID__MSK) {
        case SERIALREG_IIR_INTID_TXEMPTY: {
            uint8_t data;
            if(fifo_isinitialized(&sdata->txfifo) &&
               !fifo_read(&sdata->txfifo, &data, 1)) {
                outb(SERIAL_REG_DATA(sdata->port), data);
            } else {
                /* Need to read IIR to clear interrupt */
                inb(SERIAL_REG_IIR(sdata->port));
            }
        } break;
        case SERIALREG_IIR_INTID_RXAVAIL:
            if(fifo_isinitialized(&sdata->rxfifo)) {
                uint8_t data = inb(SERIAL_REG_DATA(sdata->port));
                /* Ignore failed write - nothing to do */
                fifo_write(&sdata->rxfifo, &data, 1);

                _serial_rx_flowcontrol(sdata);
            } else {
                /* Need to read RBR to clear interrupt */
                inb(SERIAL_REG_DATA(sdata->port));
            }
            break;
        default:
            /* Assuming no other interrupts are enabled, and thus don't need to
             * be cleared. */
            panic("Unhandled serial interrupt type: %hhu", (iir >> SERIALREG_IIR_INTID__POS) & SERIALREG_IIR_INTID__MSK);
            break;
    }
}

