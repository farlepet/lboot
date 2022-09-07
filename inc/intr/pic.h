#ifndef LBOOT_INTR_PIC_H
#define LBOOT_INTR_PIC_H

#include <stdint.h>

#define PIC_OFFSET_MASTER (32)
#define PIC_OFFSET_SLAVE  (PIC_OFFSET_MASTER + 8)

#define PIC_BIOS_OFFSET_MASTER (0x08)
#define PIC_BIOS_OFFSET_SLAVE  (0x70)

#define PIC1_BASE (0x20)
#define PIC2_BASE (0xa0)

#define PIC1_COMMAND (PIC1_BASE + 0)
#define PIC1_DATA    (PIC1_BASE + 1)
#define PIC2_COMMAND (PIC2_BASE + 0)
#define PIC2_DATA    (PIC2_BASE + 1)

#define PIC_ICW1_IC4  (1U << 0) /**< ICW4 needed */
#define PIC_ICW1_SNGL (1U << 1) /**< 1: Single mode, 0: Cascade mode */
#define PIC_ICW1_ADI  (1U << 2) /**< Call address interval (1: 4, 0: 8) */
#define PIC_ICW1_LTIM (1U << 3) /**< 1: Level triggered mode, 0: Edge triggered mode */
#define PIC_ICW1_1    (1U << 4) /**< Set to 1 */

#define PIC_ICW4_uPM  (1U << 0) /**< 0: MCS-80/85 Mode, 1: 8086/8088 Mode */
#define PIC_ICW4_AEOI (1U << 1) /**< Enable automatic EOI */
#define PIC_ICW4_MS   (1U << 2) /**< 0: Slave, 1: Master */
#define PIC_ICW4_BUF  (1U << 3) /**< Buffered mode enable */
#define PIC_ICW4_SFNM (1U << 4) /**< Enable Special Fully Nested Mode */

#define PIC_OCW2_EOI  (1U << 5) /**< End of interrupt */
#define PIC_OCW2_SL   (1U << 5) /**<  */
#define PIC_OCW2_R    (1U << 5) /**< Rotate */

/**
 * @brief Remaps PIC offsets
 *
 * @param master Master PIC offset
 * @param slave Slave PIC offset
 */
void pic_remap(uint8_t master, uint8_t slave);

/**
 * @brief Sends an End of Interrupt (EOI) to the applicable PIC
 *
 * @param irq_id IRQ ID (not to be confused with interrupt ID from the IDT's perspective)
 */
void pic_eoi(uint8_t irq_id);

/**
 * @brief Masks (disables) an IRQ
 *
 * @param irq_id IRQ to disable
 * @return 0 on success, < 0 on error
 */
int pic_mask(uint8_t irq_id);

/**
 * @brief Unmasks (enables) an IRQ
 *
 * @param irq_id IRQ to disable
 * @return 0 on success, < 0 on error
 */
int pic_unmask(uint8_t irq_id);

#endif

