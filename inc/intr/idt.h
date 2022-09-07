#ifndef LBOOT_INTR_IDT_H
#define LBOOT_INTR_IDT_H

#include <stdint.h>

#pragma pack(1)
/**
 * @brief IDT entry structure
 */
typedef struct idt_entry_struct {
    uint16_t offset_low;  /**< Low 16 bits of offset */
    uint16_t segment;     /**< Segment selector */
    uint8_t  _reserved;   /**< Reserved bits, set to 0 */
    uint8_t  flags;       /**< IDT entry flags */
    uint16_t offset_high; /**< High 16 bits of offset */
} idt_entry_t;

/**
 * @brief IDT Register (IDTR) structure
 */
typedef struct idt_idtr_struct {
    uint16_t limit;
    uint32_t base;
} idt_idtr_t;
#pragma pack()

/* @note Only using DPL of 0 for the bootloader. */

#define IDT_FLAGS_TASK     (0x85)                        /** Flags for IDT task gate descriptor */
#define IDT_FLAGS_INTR(SZ) (0x86 | ((SZ) ? 0x08 : 0x00)) /** Flags for IDT interrupt gate descriptor */
#define IDT_FLAGS_TRAP(SZ) (0x87 | ((SZ) ? 0x08 : 0x00)) /** Flags for IDT trap gate descriptor */

/**
 * @brief Initializes IDT with INT_ID_MAX NULL entries
 */
void idt_init(void);

/**
 * @brief Set IDT entry
 *
 * @param n IDT entry number to set
 * @param entry Value to give entry
 */
int idt_set_entry(uint8_t n, idt_entry_t *entry);

#endif

