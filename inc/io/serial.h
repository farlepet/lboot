#ifndef LBOOT_IO_SERIAL_H
#define LBOOT_IO_SERIAL_H

#include <stdint.h>

#include "io/output.h"
#include "io/input.h"

#define SERIAL_CFG_PORT__POS         (   0  ) /**< Serial port to use */
#define SERIAL_CFG_PORT__MSK         (0x03UL)
#define SERIAL_CFG_PORT_COM1         (   0UL)
#define SERIAL_CFG_PORT_COM2         (   1UL)
#define SERIAL_CFG_PORT_COM3         (   2UL)
#define SERIAL_CFG_PORT_COM4         (   3UL)
#define SERIAL_CFG_INBUFFSZ__POS     (   2  ) /**< n, where input buffer size = 2^n. n=0 means no buffer */
#define SERIAL_CFG_INBUFFSZ__MSK     (0x0fUL)
#define SERIAL_CFG_OUTBUFFSZ__POS    (   6  ) /**< n, where output buffer size = 2^n. n=0 means no buffer */
#define SERIAL_CFG_OUTBUFFSZ__MSK    (0x0fUL)
#define SERIAL_CFG_FLOWCTRL_RTS__POS (  10  ) /**< Enable RTS/CTS flow control (RX-only currently) */
#define SERIAL_CFG_FLOWCTRL_DTR__POS (  11  ) /**< Enable DTR/DSR flow control (RX-only currently) */

/**
 * @brief Initialize serial driver
 *
 * @param in Input device handle to populate, or NULL
 * @param out Output device handle to populate, or NULL
 * @param baud Desired baud rate
 * @param cfg Desired serial config, see `SERIAL_CFG_*`
 * @return 0 on success, < 0 on error
 */
int serial_init(input_hand_t *in, output_hand_t *out, uint32_t baud, uint32_t cfg);



#define SERIAL_REG_DATA(P) ((P) + 0) /**< Rx/Tx buffer */
#define SERIAL_REG_IER(P)  ((P) + 1) /**< Interrupt enable register */
#define SERIAL_REG_IIR(P)  ((P) + 2) /**< Interrupt identification */
#define SERIAL_REG_FCR(P)  ((P) + 2) /**< FIFO Control Register */
#define SERIAL_REG_LCR(P)  ((P) + 3) /**< Line control */
#define SERIAL_REG_MCR(P)  ((P) + 4) /**< Modem control */
#define SERIAL_REG_LSR(P)  ((P) + 5) /**< Line status */
#define SERIAL_REG_MSR(P)  ((P) + 6) /**< Modem status */
#define SERIAL_REG_SCR(P)  ((P) + 7) /**< Scratch register */
/* DLAB = 1 */
#define SERIAL_REG_DLL(P)  ((P) + 0) /**< Divisor latch low */
#define SERIAL_REG_DLM(P)  ((P) + 1) /**< Divisor latch high */


/*
 * 8250 UART controller registers
 */

#define SERIALREG_IER_RXAVAIL__POS      (   0) /**< Received data available */
#define SERIALREG_IER_TXEMPTY__POS      (   1) /**< Transmit holding register empty */
#define SERIALREG_IER_RXLINESTATUS__POS (   2) /**< Receiver line status */
#define SERIALREG_IER_MODEMSTATUS__POS  (   3) /**< Modem status */

#define SERIALREG_IIR_NOTPENDING__POS      (   0) /**< 0 if interrupt pending */
#define SERIALREG_IIR_INTID__POS           (   1) /**< Interrupt ID */
#define SERIALREG_IIR_INTID__MSK           (0x03)
#define SERIALREG_IIR_INTID_MODEMSTATUS    (   0) /**< Modem status interrupt */
#define SERIALREG_IIR_INTID_TXEMPTY        (   1) /**< TX holding register empty */
#define SERIALREG_IIR_INTID_RXAVAIL        (   2) /**< RX data available */
#define SERIALREG_IIR_INTID_RXLINESTATUS   (   3) /**< RX line status */
#define SERIALREG_IIR_INTID_TIMEOUTPENDING (   6) /**< Timeout interrupt pending */

#define SERIALREG_FCR_FIFOEN__POS       (   0) /**< FIFO Enable */
#define SERIALREG_FCR_RXFIFORST__POS    (   1) /**< RX FIFO Reset */
#define SERIALREG_FCR_TXFIFORST__POS    (   2) /**< TX FIFO Reset */
#define SERIALREG_FCR_DMAMODE__POS      (   3) /**< DMA MODE */
#define SERIALREG_FCR_64BFIFO__POS      (   5) /**< 64 Byte FIFO */
#define SERIALREG_FCR_TRIGLVL__POS      (   6) /**< RX FIFO trigger level */
#define SERIALREG_FCR_TRIGLVL__MSK      (0x03)

#define SERIALREG_LCR_WORDLEN__POS      (   0) /**< Word length*/
#define SERIALREG_LCR_WORDLEN__MSK      (0x03)
#define SERIALREG_LCR_WORDLEN_5BIT      (0x00) /**< 5 bit words */
#define SERIALREG_LCR_WORDLEN_6BIT      (0x01) /**< 6 bit words */
#define SERIALREG_LCR_WORDLEN_7BIT      (0x02) /**< 7 bit words */
#define SERIALREG_LCR_WORDLEN_8BIT      (0x03) /**< 8 bit words */
#define SERIALREG_LCR_STOPBITS__POS     (   2) /**< 1 or 1.5/2 stop bits */
#define SERIALREG_LCR_PARITY__POS       (   3) /**< Parity enable */
#define SERIALREG_LCR_EVENPARITY__POS   (   4) /**< Even parity */
#define SERIALREG_LCR_STICKPARITY__POS  (   5) /**< Stick parity */
#define SERIALREG_LCR_SETBREAK__POS     (   6) /**< Break control */
#define SERIALREG_LCR_DLAB__POS         (   7) /**< Divisor Latch Access Bit (DLAB) */

#define SERIALREG_MCR_DTR__POS          (   0) /**< Set Data Terminal Ready (DTR) */
#define SERIALREG_MCR_RTS__POS          (   1) /**< Set Request To Send (RTS) */
#define SERIALREG_MCR_OUT1__POS         (   2) /**< OUT1 control (RI in loopback) */
#define SERIALREG_MCR_OUT2__POS         (   3) /**< OUT2 control (DCD in loopback) */
#define SERIALREG_MCR_LOOP__POS         (   4) /**< Enable loopback */

#define SERIALREG_LSR_DR__POS           (   0) /**< Data Ready */
#define SERIALREG_LSR_OE__POS           (   1) /**< Overrun Error */
#define SERIALREG_LSR_PE__POS           (   2) /**< Parity Error */
#define SERIALREG_LSR_FE__POS           (   3) /**< Framing Error */
#define SERIALREG_LSR_BI__POS           (   4) /**< Break Interrupt */
#define SERIALREG_LSR_THRE__POS         (   5) /**< Transmitter Holding Register Empty */
#define SERIALREG_LSR_TEMT__POS         (   6) /**< Data Holding Register Empty */

#define SERIALREG_MSR_DCTS__POS         (   0) /**< Change in CTS */
#define SERIALREG_MSR_DDSR__POS         (   1) /**< Change in DSR */
#define SERIALREG_MSR_TERI__POS         (   2) /**< RI De-asserted */
#define SERIALREG_MSR_DDCD__POS         (   3) /**< Change in DCD */
#define SERIALREG_MSR_CTS__POS          (   4) /**< CTS asserted */
#define SERIALREG_MSR_DSR__POS          (   5) /**< DSR asserteed */
#define SERIALREG_MSR_RI__POS           (   6) /**< RI asserted */
#define SERIALREG_MSR_DCD__POS          (   7) /**< DCD asserted */

#endif

