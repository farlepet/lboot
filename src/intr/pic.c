#include "intr/pic.h"
#include "io/ioport.h"

void pic_remap(uint8_t master, uint8_t slave) {
    /* Save interrupt masks */
    uint8_t m_mask = inb(PIC1_DATA);
    uint8_t s_mask = inb(PIC2_DATA);

    /* ICW 1 */
    outb(PIC1_COMMAND, PIC_ICW1_IC4 | PIC_ICW1_1);
    outb(PIC2_COMMAND, PIC_ICW1_IC4 | PIC_ICW1_1);
    /* ICW 2 - vector offset*/
    outb(PIC1_DATA, master);
    outb(PIC2_DATA, slave);
    /* ICW 3 */
    outb(PIC1_DATA, (1U << 2)); /* IRQ2 as slave input */
    outb(PIC2_DATA, 2);         /* Slave identity is 2 */
    /* ICW 4 */
    outb(PIC1_DATA, PIC_ICW4_uPM);
    outb(PIC2_DATA, PIC_ICW4_uPM);

    /* Restore interrupt masks */
    outb(PIC1_DATA, m_mask);
    outb(PIC2_DATA, s_mask);
}

void pic_eoi(uint8_t irq_id) {
    if(irq_id >= 8) {
        outb(PIC2_COMMAND, PIC_OCW2_EOI);
    }
    /* Due to chaining, EOI always needs to be commanded to the master */
    outb(PIC1_COMMAND, PIC_OCW2_EOI);
}

int pic_mask(uint8_t irq_id) {
    if(irq_id >= 16) {
        return -1;
    }

    uint16_t port = PIC1_DATA;
    if(irq_id >= 8) {
        port = PIC2_DATA;
        irq_id -= 8;
    }

    uint8_t mask = inb(port) | (1U << irq_id);
    outb(port, mask);

    return 0;
}

int pic_unmask(uint8_t irq_id) {
    if(irq_id >= 16) {
        return -1;
    }

    uint16_t port = PIC1_DATA;
    if(irq_id >= 8) {
        port = PIC2_DATA;
        irq_id -= 8;
    }

    uint8_t mask = inb(port) & ~(1U << irq_id);
    outb(port, mask);

    return 0;
}

