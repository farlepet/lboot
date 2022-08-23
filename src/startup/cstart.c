#include <string.h>
#include <stddef.h>

#include "mm/alloc.h"
#include "io/output.h"
#include "io/serial.h"
#include "io/vga.h"
#include "storage/bios.h"
#include "storage/fs/fs.h"
#include "storage/fs/fat.h"

#define USE_SERIAL (0)

static void _init_data(void) {
    /* Clear BSS */
    extern int __lboot_bss_begin, __lboot_bss_end;
    memset(&__lboot_bss_begin, 0, (&__lboot_bss_end - &__lboot_bss_begin));
}

static output_hand_t  _vga;
#if (USE_SERIAL)
static output_hand_t  _serial;
#endif
static storage_hand_t _bootdev;
static fs_hand_t      _bootfs;

void cstart(void) {
    _init_data();

    vga_init(&_vga);
    output_set(&_vga);

    extern int __lboot_end;
    /* Assuming fully populated conventional memory. Could also use INT 12.
     * Realistically, it's unlikely this will ever be used on a system with
     * less than 1 MiB of RAM. */
    alloc_init((uint32_t)&__lboot_end, 0x80000 - (uint32_t)&__lboot_end);

#if (USE_SERIAL)
    serial_init(&_serial, 0x3f8);
    output_set(&_serial);
#endif

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
    
    fs_file_t stage2;
    if(_bootfs.find(&_bootfs, NULL, &stage2, "STAGE2.BIN")) {
        puts("Failed to find STAGE2.BIN!\n");
        for(;;);
    }
    printf("Stage 2 file size: %d B\n", stage2.size);

    puts("OK\n");

    for(;;) {
        asm volatile("hlt");
    }
}

