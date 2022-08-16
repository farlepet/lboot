#include <string.h>

#include "io/vga.h"
#include "io/output.h"
#include "bios/bios.h"
#include "storage/bios.h"
#include "storage/fs/fs.h"
#include "storage/fs/fat.h"

static void _init_data(void) {
    /* Clear BSS */
    extern int __lboot_bss_begin, __lboot_bss_end;
    memset(&__lboot_bss_begin, 0, (&__lboot_bss_end - &__lboot_bss_begin));
}

static output_hand_t  _vga;
static storage_hand_t _bootdev;
static fs_hand_t      _bootfs;

void cstart(void) {
    _init_data();

    vga_init(&_vga);
    output_set(&_vga);

    puts("LBoot -- Built "__DATE__"\n");

    /* @todo Actually use the device ID passed to us from the BIOS */
    if(storage_bios_init(&_bootdev, 0x00)) {
        puts("Failed initializing storage!\n");
        for(;;);
    }

    if(fs_fat_init(&_bootfs, &_bootdev, 0x00)) {
        puts("Failed initializing filesystem!\n");
        for(;;);
    }

    printf("Boot filesystem size: %u KiB\n", (_bootfs.fs_size / 1024));
    
    puts("OK\n");

    for(;;);
}

