#include "bios/bios.h"
#include "intr/interrupts.h"
#include "intr/pic.h"

extern void bios_call_asm(bios_call_t *call);

void bios_call(bios_call_t *call) {
    int int_en = interrupts_enabled();
    interrupts_disable();

    /* It may make more sense to abstract this to a interrupt_* function */
    pic_remap(PIC_BIOS_OFFSET_MASTER, PIC_BIOS_OFFSET_SLAVE);
    
    bios_call_asm(call);

    pic_remap(PIC_OFFSET_MASTER, PIC_OFFSET_SLAVE);

    if(int_en) {
        interrupts_enable();
    }
}

