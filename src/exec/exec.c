#include <string.h>

#include "exec/exec.h"
#include "io/output.h"
#include "mm/alloc.h"

#include "exec/fmt/elf.h"
#include "exec/fmt/flat.h"

#define DEBUG_EXEC (0)

int exec_open(exec_hand_t *exec, fs_file_t *file) {
    memset(exec, 0, sizeof(*exec));

    exec->file    = file;
    fs_hand_t *fs = file->fs;

    void *buf = alloc(EXEC_FIRSTCHUNK_SZ, 0);

    if(fs->read(fs, exec->file, buf, EXEC_FIRSTCHUNK_SZ, 0) != EXEC_FIRSTCHUNK_SZ) {
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

__attribute__((optimize("-O0")))
static void _exec_enter(uintptr_t entrypoint, uintptr_t mboot_ptr) {
    /* EAX: Magic number
     * EBX: Pointer to multiboot header */
    asm volatile("mov %0,          %%edx\n"
                 "mov $0x36D76289, %%eax\n"
                 "mov %1,          %%ebx\n"
                 "jmp *%%edx\n" ::
                 "m"(entrypoint), "m"(mboot_ptr));
}

__attribute__((optimize("-O0")))
int exec_exec(exec_hand_t *exec) {
    if(exec->prepare(exec)) {
        return -1;
    }
    if(exec->load(exec)) {
        return -1;
    }

#if (DEBUG_EXEC)
    printf("  Jumping into kernel at %p.\n", exec->entrypoint);
#endif

    /* @todo Set up multiboot */
    _exec_enter(exec->entrypoint, 0);
    
    return 0;
}

