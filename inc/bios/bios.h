#ifndef LBOOT_BIOS_BIOS_H
#define LBOOT_BIOS_BIOS_H

#include <stdint.h>

typedef struct {
    uint8_t  int_n;    /**< Interrupt ID */
    uint8_t  _padding;
    uint16_t ax;       /**< AX regsiter */
    uint16_t bx;       /**< BX register */
    uint16_t cx;       /**< CX register */
    uint16_t dx;       /**< DX register */
    uint16_t si;       /**< SI register */
    uint16_t di;       /**< DI register */
} bios_call_t;

/**
 * @brief Makes a call to a BIOS interrupt in real mode
 *
 * @note All segments are set to 0 prior to making tha call
 *
 * @param call Pointer of parameters to use for interrupt call
 */
void bios_call(bios_call_t *call);

#endif

