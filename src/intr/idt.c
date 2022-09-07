#include <string.h>

#include "intr/idt.h"
#include "intr/interrupts.h"

__attribute__((aligned(8)))
static idt_entry_t _idt[INT_ID_MAX];

__attribute__((used))
static idt_idtr_t  _idtr = { .base  = (uint32_t)&_idt,
                             .limit = sizeof(_idt) - 1 };

void idt_init(void) {
    memset(&_idt, 0, sizeof(_idt));

    asm volatile("lidt (_idtr)");
}

int idt_set_entry(uint8_t n, idt_entry_t *entry) {
    if(n >= INT_ID_MAX) {
        return -1;
    }

    memcpy(&_idt[n], entry, sizeof(*entry));

    return 0;
}

