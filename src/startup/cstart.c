#include <string.h>

#include "io/vga.h"

static void _init_data(void) {
    /* Clear BSS */
    extern int __lboot_bss_begin, __lboot_bss_end;
    memset(&__lboot_bss_begin, 0, (&__lboot_bss_end - &__lboot_bss_begin));
}

void cstart(void) {
    _init_data();

    vga_init();

    vga_puts("Hello from C!\n");

    for(;;);
}

