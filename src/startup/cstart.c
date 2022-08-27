#include <string.h>
#include <stddef.h>

#include "config/config.h"
#include "mm/alloc.h"
#include "exec/exec.h"
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
static exec_hand_t    _exec;

static const char   *_config_file = "LBOOT.CFG";
static config_data_t _cfg;

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
        panic("Failed initializing storage!\n");
    }

    if(fs_fat_init(&_bootfs, &_bootdev, 0x00)) {
        panic("Failed initializing filesystem!\n");
    }

    printf("Boot filesystem size: %u KiB\n", (_bootfs.fs_size / 1024));

    printf("Loading config `%s`\n", _config_file);
    if(config_load(&_cfg, &_bootfs, _config_file)) {
        panic("Failed loading config!\n");
    }

    if(_cfg.kernel_path == NULL) {
        panic("Kernel not specified in config!\n");
    }
    printf("Loading kernel `%s`\n", _cfg.kernel_path);

    fs_file_t kernel;
    if(_bootfs.find(&_bootfs, NULL, &kernel, _cfg.kernel_path)) {
        panic("Failed to find kernel!\n");
    }

    if(exec_open(&_exec, &kernel)) {
        panic("Failed to open kernel for execution!\n");
    }

    if(exec_exec(&_exec)) {
        panic("Failed to load and execute kernel!\n");
    }

    puts("OK\n");

    for(;;) {
        asm volatile("hlt");
    }
}

