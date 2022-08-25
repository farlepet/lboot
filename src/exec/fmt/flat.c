#include "exec/fmt/flat.h"
#include "io/output.h"

static int _flat_prepare(exec_hand_t *exec);
static int _flat_load(exec_hand_t *exec);

int exec_flat_init(exec_hand_t *exec) {
    exec->prepare = _flat_prepare;
    exec->load    = _flat_load;

    return 0;
}

static int _flat_prepare(exec_hand_t *exec) {
    if((exec->entrypoint < 0x10000) ||
       (exec->data_begin < 0x10000)) {
        /* Currently no relocation is supported. */
        printf("_flat_prepare: Refusing to load binary below 1 MiB boundry\n");
        return -1;
    }
    /* @todo */
    return 0;
}

static int _flat_load(exec_hand_t *exec) {
    (void)exec;
    /* @todo */
    return 0;
}

