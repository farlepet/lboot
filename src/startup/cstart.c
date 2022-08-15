#include <string.h>

#include "io/vga.h"
#include "bios/bios.h"

static void _init_data(void) {
    /* Clear BSS */
    extern int __lboot_bss_begin, __lboot_bss_end;
    memset(&__lboot_bss_begin, 0, (&__lboot_bss_end - &__lboot_bss_begin));
}

void cstart(void) {
    _init_data();

    vga_init();

    vga_puts("Hello from C!\n");

    /* Test - Clear the screen */
    bios_call_t call;
    memset(&call, 0, sizeof(bios_call_t));
    call.int_n = 0x10;
    call.ax    = 0x0003;

    bios_call(&call);

    vga_puts("bios_call returned!!\n");

    for(;;);
}

