#include <string.h>

#include "exec/exec.h"
#include "io/output.h"
#include "mm/alloc.h"

#include "exec/fmt/elf.h"

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
            /* @todo */
            break;
        default:
            return -1;
    }

    return 0;
}

int exec_exec(exec_hand_t *exec) {
    if(exec->prepare(exec)) {
        return -1;
    }
    if(exec->load(exec)) {
        return -1;
    }

    /* @todo Set up multiboot */
    /* @todo Jump into process */
    
    return 0;
}

