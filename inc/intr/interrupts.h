#ifndef LBOOT_INTR_INTERRUPTS_H
#define LBOOT_INTR_INTERRIPTS_H

#include <stdint.h>

#include "intr/pic.h"

/**
 * @brief Interrupt IDs
 */
typedef enum int_id_enum {
    /* Exceptions */
    INT_ID_DIVIDEBYZERO               =  0,
    INT_ID_DEBUG                      =  1,
    INT_ID_NMI                        =  2,
    INT_ID_BREAKPOINT                 =  3,
    INT_ID_OVERFLOW                   =  4,
    INT_ID_BOUNDRANGEEXCEEDED         =  5,
    INT_ID_INVALIDOPCODE              =  6,
    INT_ID_DEVICENOTAVAILABLE         =  7,
    INT_ID_DOUBLEFAULT                =  8,
    INT_ID_RESERVED9                  =  9,
    INT_ID_INVALIDTSS                 = 10,
    INT_ID_SEGMENTNOTPRESENT          = 11,
    INT_ID_STACKSEGMENTFAULT          = 12,
    INT_ID_GENERALPROTECTIONFAULT     = 13,
    INT_ID_PAGEFAULT                  = 14,
    INT_ID_RESERVED15                 = 15,
    INT_ID_FLOATINGPOINTFAULT         = 16,
    INT_ID_ALIGNMENTCHECK             = 17,
    INT_ID_MACHINECHECK               = 18,
    INT_ID_SIMDFLOATINGPOINTEXCEPTION = 19,
    INT_ID_VIRTUALIZATIONEXCEPTION    = 20,
    INT_ID_CONTROLPROTECTIONEXCEPTION = 21,
    /* Master PIC */
    INT_ID_PIT       = PIC_OFFSET_MASTER,
    INT_ID_KEYBOARD,
    INT_ID_CASCADE,
    INT_ID_COM2,
    INT_ID_COM1,
    INT_ID_LPT2,
    INT_ID_FLOPPY,
    INT_ID_LPT1,
    /* Slave PIC */
    INT_ID_CMOSCLOCK = PIC_OFFSET_SLAVE,
    INT_ID_IRQ9,
    INT_ID_IRQ10,
    INT_ID_IRQ11,
    INT_ID_PS2MOUSE,
    INT_ID_COPROCESSOR,
    INT_ID_ATAPRIMARY,
    INT_ID_ATASECONDARY,

    INT_ID_MAX
} int_id_e;

typedef void (*interrupt_handler_t)(uint8_t int_id, uint32_t errno, void *data);

/**
 * @brief Enables interrupts
 */
static inline void interrupts_enable(void) {
    asm volatile("sti");
}

/**
 * @brief Disabled interrupts
 */
static inline void interrupts_disable(void) {
    asm volatile("cli");
}

/**
 * @brief Initialize interrupts
 *
 * @return 0 on success, < 0 on failure
 */
int interrupts_init(void);

/**
 * @brief Register interrupt handler
 *
 * @param int_id Interrupt ID
 * @param handler Interrupt handler
 * @param data Data to pass into the handler, or NULL
 * @return 0 on success, < 0 on failure
 */
int interrupt_register(uint8_t int_id, interrupt_handler_t handler, void *data);

/**
 * @brief Enables a specific interrupt
 *
 * @note Only applies to IRQs
 *
 * @param int_id Interrupt to enable
 * @return 0 on success, < 0 on failure
 */
int interrupt_enable(uint8_t int_id);

/**
 * @brief Disables a specific interrupt
 *
 * @note Only applies to IRQs
 *
 * @param int_id Interrupt to disable
 * @return 0 on success, < 0 on failure
 */
int interrupt_disable(uint8_t int_id);

#endif
