#include <string.h>

#include "intr/idt.h"
#include "intr/interrupts.h"
#include "intr/pic.h"
#include "io/output.h"

typedef struct isr_entry_struct {
    void               *data;
    interrupt_handler_t handler;
} isr_entry_t;

isr_entry_t _isr_handlers[INT_ID_MAX];

static void _isr_wrappers_init(void);

int interrupts_init(void) {
    interrupts_disable();

    idt_init();
    pic_remap(PIC_OFFSET_MASTER, PIC_OFFSET_SLAVE);

    _isr_wrappers_init();

    memset(&_isr_handlers, 0, sizeof(_isr_handlers));

    interrupts_enable();

    return 0;
}

int interrupt_register(uint8_t int_id, interrupt_handler_t handler, void *data) {
    /* @todo */
    if(int_id >= INT_ID_MAX) {
        return -1;
    }

    _isr_handlers[int_id].data    = data;
    _isr_handlers[int_id].handler = handler;

    return 0;
}

int interrupt_enable(uint8_t int_id) {
    if((int_id >= PIC_OFFSET_MASTER) &&
       (int_id < (PIC_OFFSET_MASTER + 8))) {
        if(pic_unmask(int_id - PIC_OFFSET_MASTER)) {
            return -1;
        }
    } else if ((int_id >= PIC_OFFSET_SLAVE) &&
               (int_id < (PIC_OFFSET_SLAVE + 8))) {
        if(pic_unmask((int_id - PIC_OFFSET_SLAVE) + 8)) {
            return -1;
        }
    } else {
        return -1;
    }

    return 0;
}

int interrupt_disable(uint8_t int_id) {
    if((int_id >= PIC_OFFSET_MASTER) &&
       (int_id < (PIC_OFFSET_MASTER + 8))) {
        if(pic_mask(int_id - PIC_OFFSET_MASTER)) {
            return -1;
        }
    } else if ((int_id >= PIC_OFFSET_SLAVE) &&
               (int_id < (PIC_OFFSET_SLAVE + 8))) {
        if(pic_mask((int_id - PIC_OFFSET_SLAVE) + 8)) {
            return -1;
        }
    } else {
        return -1;
    }

    return 0;
}

void interrupt_wrapper(uint32_t int_id, uint32_t errcode) {
    if((int_id < INT_ID_MAX) &&
       _isr_handlers[int_id].handler) {
        _isr_handlers[int_id].handler(int_id, errcode, _isr_handlers[int_id].data);
    } else if(int_id < 32) {
        panic("Unhandled exception: %2u, %08x", int_id, errcode);
    }

    /* If interrupt came from the PIC, send EOI. */
    if((int_id >= PIC_OFFSET_MASTER) &&
       (int_id < (PIC_OFFSET_MASTER + 8))) {
        pic_eoi(int_id - PIC_OFFSET_MASTER);
    } else if ((int_id >= PIC_OFFSET_SLAVE) &&
               (int_id < (PIC_OFFSET_SLAVE + 8))) {
        pic_eoi((int_id - PIC_OFFSET_SLAVE) + 8);
    }
}

extern void isr_wrapper_0(void);
extern void isr_wrapper_1(void);
extern void isr_wrapper_2(void);
extern void isr_wrapper_3(void);
extern void isr_wrapper_4(void);
extern void isr_wrapper_5(void);
extern void isr_wrapper_6(void);
extern void isr_wrapper_7(void);
extern void isr_wrapper_8(void);
extern void isr_wrapper_9(void);
extern void isr_wrapper_10(void);
extern void isr_wrapper_11(void);
extern void isr_wrapper_12(void);
extern void isr_wrapper_13(void);
extern void isr_wrapper_14(void);
extern void isr_wrapper_15(void);
extern void isr_wrapper_16(void);
extern void isr_wrapper_17(void);
extern void isr_wrapper_18(void);
extern void isr_wrapper_19(void);
extern void isr_wrapper_20(void);
extern void isr_wrapper_21(void);
extern void isr_wrapper_22(void);
extern void isr_wrapper_23(void);
extern void isr_wrapper_24(void);
extern void isr_wrapper_25(void);
extern void isr_wrapper_26(void);
extern void isr_wrapper_27(void);
extern void isr_wrapper_28(void);
extern void isr_wrapper_29(void);
extern void isr_wrapper_30(void);
extern void isr_wrapper_31(void);
extern void isr_wrapper_32(void);
extern void isr_wrapper_33(void);
extern void isr_wrapper_34(void);
extern void isr_wrapper_35(void);
extern void isr_wrapper_36(void);
extern void isr_wrapper_37(void);
extern void isr_wrapper_38(void);
extern void isr_wrapper_39(void);
extern void isr_wrapper_40(void);
extern void isr_wrapper_41(void);
extern void isr_wrapper_42(void);
extern void isr_wrapper_43(void);
extern void isr_wrapper_44(void);
extern void isr_wrapper_45(void);
extern void isr_wrapper_46(void);
extern void isr_wrapper_47(void);

static void _idt_set_wrapper(uint8_t int_n, void *wrapper) {
    idt_entry_t ent  = {
        .offset_low  = (uint16_t)(uintptr_t)wrapper,
        .segment     = 0x0008,
        ._reserved   = 0,
        .flags       = IDT_FLAGS_INTR(1),
        .offset_high = (uint16_t)((uintptr_t)wrapper >> 16)
    };
    idt_set_entry(int_n, &ent);
}

static void _isr_wrappers_init(void) {
    _idt_set_wrapper( 0, isr_wrapper_0);
    _idt_set_wrapper( 1, isr_wrapper_1);
    _idt_set_wrapper( 2, isr_wrapper_2);
    _idt_set_wrapper( 3, isr_wrapper_3);
    _idt_set_wrapper( 4, isr_wrapper_4);
    _idt_set_wrapper( 5, isr_wrapper_5);
    _idt_set_wrapper( 6, isr_wrapper_6);
    _idt_set_wrapper( 7, isr_wrapper_7);
    _idt_set_wrapper( 8, isr_wrapper_8);
    _idt_set_wrapper( 9, isr_wrapper_9);
    _idt_set_wrapper(10, isr_wrapper_10);
    _idt_set_wrapper(11, isr_wrapper_11);
    _idt_set_wrapper(12, isr_wrapper_12);
    _idt_set_wrapper(13, isr_wrapper_13);
    _idt_set_wrapper(14, isr_wrapper_14);
    _idt_set_wrapper(15, isr_wrapper_15);
    _idt_set_wrapper(16, isr_wrapper_16);
    _idt_set_wrapper(17, isr_wrapper_17);
    _idt_set_wrapper(18, isr_wrapper_18);
    _idt_set_wrapper(19, isr_wrapper_19);
    _idt_set_wrapper(20, isr_wrapper_20);
    _idt_set_wrapper(21, isr_wrapper_21);
    _idt_set_wrapper(22, isr_wrapper_22);
    _idt_set_wrapper(23, isr_wrapper_23);
    _idt_set_wrapper(24, isr_wrapper_24);
    _idt_set_wrapper(25, isr_wrapper_25);
    _idt_set_wrapper(26, isr_wrapper_26);
    _idt_set_wrapper(27, isr_wrapper_27);
    _idt_set_wrapper(28, isr_wrapper_28);
    _idt_set_wrapper(29, isr_wrapper_29);
    _idt_set_wrapper(30, isr_wrapper_30);
    _idt_set_wrapper(31, isr_wrapper_31);
    _idt_set_wrapper(32, isr_wrapper_32);
    _idt_set_wrapper(33, isr_wrapper_33);
    _idt_set_wrapper(34, isr_wrapper_34);
    _idt_set_wrapper(35, isr_wrapper_35);
    _idt_set_wrapper(36, isr_wrapper_36);
    _idt_set_wrapper(37, isr_wrapper_37);
    _idt_set_wrapper(38, isr_wrapper_38);
    _idt_set_wrapper(39, isr_wrapper_39);
    _idt_set_wrapper(40, isr_wrapper_40);
    _idt_set_wrapper(41, isr_wrapper_41);
    _idt_set_wrapper(42, isr_wrapper_42);
    _idt_set_wrapper(43, isr_wrapper_43);
    _idt_set_wrapper(44, isr_wrapper_44);
    _idt_set_wrapper(45, isr_wrapper_45);
    _idt_set_wrapper(46, isr_wrapper_46);
    _idt_set_wrapper(47, isr_wrapper_47);
}

