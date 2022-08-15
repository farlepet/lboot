#ifndef LBOOT_BIOS_BIOS_H
#define LBOOT_BIOS_BIOS_H

#include <stdint.h>

typedef struct {
    uint8_t  int_n;    /**< Interrupt ID */
    uint8_t  _padding;
    union {
        struct {
            uint16_t ax;       /**< AX regsiter */
            uint16_t bx;       /**< BX register */
            uint16_t cx;       /**< CX register */
            uint16_t dx;       /**< DX register */
        };
        struct {
            uint8_t al;
            uint8_t ah;
            uint8_t bl;
            uint8_t bh;
            uint8_t cl;
            uint8_t ch;
            uint8_t dl;
            uint8_t dh;
        };
    };
    uint16_t si;       /**< SI register */
    uint16_t di;       /**< DI register */
    uint16_t eflags;   /**< EFLAGS register (lower 16 bits) */
} bios_call_t;

#define EFLAGS_CF (1U <<  0) /**< Carry flag */
#define EFLAGS_PF (1U <<  2) /**< Parity flag */
#define EFLAGS_AF (1U <<  4) /**< Auxiliary flag */
#define EFLAGS_ZF (1U <<  6) /**< Zero flag */
#define EFLAGS_SF (1U <<  7) /**< Sign flag */
#define EFLAGS_TF (1U <<  8) /**< Trap flag */
#define EFLAGS_IF (1U <<  9) /**< Interrupt enable flag */
#define EFLAGS_DF (1U << 10) /**< Direction flag */
#define EFLAGS_OF (1U << 11) /**< Overflow flag */
#define EFLAGS_NT (1U << 14) /**< Nested task flag */

/**
 * @brief Makes a call to a BIOS interrupt in real mode
 *
 * @note All segments are set to 0 prior to making tha call
 *
 * @param call Pointer of parameters to use for interrupt call
 */
void bios_call(bios_call_t *call);

#endif

