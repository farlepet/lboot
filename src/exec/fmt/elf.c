#include "exec/fmt/elf.h"
#include "io/output.h"

static int _elf_prepare(exec_hand_t *exec);
static int _elf_load(exec_hand_t *exec);

int exec_elf_test(exec_hand_t *exec, void *first_chunk) {
    (void)exec;

    return *(uint32_t *)first_chunk == ELF_IDENT;
}

int exec_elf_init(exec_hand_t *exec) {
    exec->prepare = _elf_prepare;
    exec->load    = _elf_load;

    return 0;
}

static int _elf_prepare(exec_hand_t *exec) {
    if(exec->entrypoint || exec->data_begin) {
        /* Currently no relocation is supported. */
        printf("_elf_prepare: Explicit address given, ignoring (no relocation support)\n");
    }
    /* @todo */
    return 0;
}

static int _elf_load(exec_hand_t *exec) {
    (void)exec;
    /* @todo */
    return 0;
}

