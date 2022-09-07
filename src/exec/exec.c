#include <stddef.h>
#include <string.h>

#include "exec/exec.h"
#include "exec/multiboot.h"
#include "io/output.h"
#include "mm/alloc.h"

#include "exec/fmt/elf.h"
#include "exec/fmt/flat.h"

int exec_open(exec_hand_t *exec, file_hand_t *file) {
    memset(exec, 0, sizeof(*exec));

    exec->file    = file;

    void *buf = alloc(EXEC_FIRSTCHUNK_SZ, 0);

    if(exec->file->read(exec->file, buf, EXEC_FIRSTCHUNK_SZ, 0) != EXEC_FIRSTCHUNK_SZ) {
        printf("exec_open: Could not read first chunk from file.\n");
        return -1;
    }

    /* Determine file format */
    if(exec_elf_test(exec, buf)) {
        exec->fmt = EXEC_FILEFMT_ELF;
    } else {
        printf("exec_open: Format could not be determined, assuming flat binary.\n");
        exec->fmt = EXEC_FILEFMT_FLAT;
    }

    free(buf);

    switch(exec->fmt) {
        case EXEC_FILEFMT_ELF:
            if(exec_elf_init(exec)) {
                return -1;
            }
            break;
        case EXEC_FILEFMT_FLAT:
            if(exec_flat_init(exec)) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    return 0;
}

/* @todo Move this elsewhere, as it could be useful. */
#define ALIGN(P, A) (((P) % (A)) ? (P) : ((P) + ((A) - ((P) % (A)))))

static int _exec_load_modules(exec_hand_t *exec, config_data_t *cfg) {
    uintptr_t addr = exec->data_end;

    file_hand_t modfile;

    for(unsigned i = 0; i < cfg->module_count; i++) {
        addr = ALIGN(addr, 8);

        print_status("Loading module `%s` (%s)", cfg->modules[i].module_name, cfg->modules[i].module_name);

        /* @todo Do not only search this filesystem, create generic accessor. */
        if(file_open(&modfile, cfg->modules[i].module_path)) {
            printf("_exec_load_modules: Could not find file\n");
            return -1;
        }

        if(modfile.read(&modfile, (void *)addr, modfile.size, 0) != (ssize_t)modfile.size) {
            printf("_exec_load_modules: Could not read from file\n");
            return -1;
        }

        cfg->modules[i].module_addr = addr;
        cfg->modules[i].module_size = modfile.size;

        modfile.close(&modfile);

        addr += cfg->modules[i].module_size;
    }
    return 0;
}

static int _exec_detect_multiboot(exec_hand_t *exec) {
    /* Assuming data_begin is 8-byte aligned. */
    uint32_t *search = (uint32_t *)exec->data_begin;

    /* Multiboot header must be within first 8 KiB of data, and be 8-byte aligned. */
    for(unsigned i = 0; i < 2048; i+=2) {
        if(search[i] == MULTIBOOT1_HEAD_MAGIC) {
            return 1;
        } if(search[i] == MULTIBOOT2_HEAD_MAGIC) {
            exec->multiboot = (multiboot2_head_t *)&search[i];
            return 2;
        }
    }

    return 0;
}

static void _exec_enter_multiboot2(uintptr_t entrypoint, uintptr_t mboot_ptr) {
    /* EAX: Magic number
     * EBX: Pointer to multiboot header */
    asm volatile("mov %0,          %%edx\n"
                 "mov $0x36D76289, %%eax\n"
                 "mov %1,          %%ebx\n"
                 "jmp *%%edx\n" ::
                 "m"(entrypoint), "m"(mboot_ptr));
}

static void _exec_enter(uintptr_t entrypoint) {
    asm volatile("mov %0,          %%edx\n"
                 "jmp *%%edx\n" ::
                 "m"(entrypoint));
}


int exec_exec(exec_hand_t *exec, config_data_t *cfg) {
    if(exec->prepare(exec)) {
        return -1;
    }
    if(exec->load(exec)) {
        return -1;
    }

    if(_exec_load_modules(exec, cfg)) {
        return -1;
    }

    int mboot = _exec_detect_multiboot(exec);
    if(mboot == 1) {
        printf("Kernel uses multiboot 1 - this is unsupported.\n");
        return -1;
    } else if(mboot == 2) {
#if (DEBUG_EXEC)
        printf("Multiboot 2 header found at %p\n", exec->multiboot);
#endif
        multiboot2_t *mboot2 = multiboot2_parse(exec->multiboot, cfg);
        if(mboot2 == NULL) {
            return -1;
        }
        /* @todo Set up multiboot */
        _exec_enter_multiboot2(exec->entrypoint, (uintptr_t)mboot2);
    } else {
#if (DEBUG_EXEC)
        printf("Jumping into kernel at %p.\n", exec->entrypoint);
#endif
        _exec_enter(exec->entrypoint);
    }
    
    return 0;
}

