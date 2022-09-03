#include <string.h>

#include "exec/fmt/elf.h"
#include "io/output.h"
#include "mm/alloc.h"

typedef struct exec_elf_data_struct {
    elf_header_t *ehdr; /**< Buffer containing ELF file header */
    elf32_phdr_t *phdr; /**< Buffer containing ELF program headers */
} exec_elf_data_t;

static int _elf_prepare(exec_hand_t *exec);
static int _elf_load(exec_hand_t *exec);

int exec_elf_test(exec_hand_t *exec, void *first_chunk) {
    (void)exec;

    return *(uint32_t *)first_chunk == ELF_IDENT;
}

int exec_elf_init(exec_hand_t *exec) {
    exec->prepare = _elf_prepare;
    exec->load    = _elf_load;

    exec->data_begin = 0xFFFFFFFF;

    exec_elf_data_t *edata = alloc(sizeof(exec_elf_data_t), 0);
    memset(edata, 0, sizeof(*edata));
    exec->data = edata;

    return 0;
}

/**
 * @brief Check that all loadable program headers can be supported properly
 *
 * @param exec Executable handle
 * @return 0 on all okay, < 0 on unsupported load or error
 */
static int _elf_phdr_check(exec_hand_t *exec) {
    exec_elf_data_t *edata = exec->data;

    for(unsigned i = 0; i < edata->ehdr->e32.phnum; i++) {
        elf32_phdr_t *phdr = &edata->phdr[i];
        if(phdr->type == ELF_PHDR_TYPE_LOAD) {
            if(phdr->vaddr < 0x100000) {
                printf("_elf_phdr_check: Contains segment < 1 MiB - unsupported.\n");
                return -1;
            }
        }
    }

    return 0;
}

static int _elf_prepare(exec_hand_t *exec) {
    exec_elf_data_t *edata = exec->data;

    if(exec->entrypoint || (exec->data_begin != 0xFFFFFFFF)) {
        /* Currently no relocation is supported. */
        printf("_elf_prepare: Explicit address given, ignoring (no relocation support)\n");
    }

    edata->ehdr = alloc(sizeof(elf_header_t), 0);
    
    if(exec->file->read(exec->file, edata->ehdr, sizeof(elf_header_t), 0) != sizeof(elf_header_t)) {
        goto elf_prep_fail_1;
    }

    if((edata->ehdr->ident.class != HOST_ELF_CLASS)        ||
       (edata->ehdr->ident.data  != ELF_DATA_LITTLEENDIAN) ||
       (edata->ehdr->machine     != HOST_ELF_MACHINE)) {
        printf("_elf_prepare: Unsupported target platform.\n");
        goto elf_prep_fail_1;
    }

    if(edata->ehdr->type != ELF_TYPE_EXEC) {
        printf("_elf_prepare: Only executable-type ELF binaries supported.\n");
        goto elf_prep_fail_1;
    }

    if(edata->ehdr->e32.entry < 0x100000) {
        printf("_elf_prepare: Entrypoint < 1 MiB - unsupported.\n");
        goto elf_prep_fail_1;
    }

    size_t phdr_tot_size = edata->ehdr->e32.phentsize * edata->ehdr->e32.phnum;
    edata->phdr = alloc(phdr_tot_size, 0);

    if(exec->file->read(exec->file, edata->phdr, phdr_tot_size, edata->ehdr->e32.phoff) != (ssize_t)phdr_tot_size) {
        goto elf_prep_fail_2;
    }

    if(_elf_phdr_check(exec)) {
        goto elf_prep_fail_2;
    }

    exec->entrypoint = edata->ehdr->e32.entry;

    return 0;

elf_prep_fail_2:
    free(edata->phdr);

elf_prep_fail_1:
    free(edata->ehdr);
    free(edata);

    return -1;
}

/**
 * @brief Loops through program headers and loads executable into memory accordingly
 *
 * @param exec Exec handle
 * @return 0 on success, < 0 on failure
 */
static int _elf_load_phdr(exec_hand_t *exec) {
    exec_elf_data_t *edata = exec->data;

    for(unsigned i = 0; i < edata->ehdr->e32.phnum; i++) {
        elf32_phdr_t *phdr = &edata->phdr[i];
        if(phdr->type == ELF_PHDR_TYPE_LOAD) {
            /* @note Using paddr, as if the kernel is linked to higher memory,
             * we may attempt to load into non-existent memory. The kernel
             * should do the mapping itself after it is loaded. */
            if(phdr->paddr < exec->data_begin) {
                exec->data_begin = phdr->paddr;
            }
            if((phdr->paddr + phdr->memsz) > exec->data_end) {
                exec->data_end = phdr->paddr + phdr->memsz;
            }

            if(phdr->filesz) {
#if (DEBUG_EXEC_ELF)
                printf("  Loading  %6u bytes from file into %p.\n", phdr->filesz, phdr->paddr);
#endif
                if(exec->file->read(exec->file, (void *)phdr->paddr, phdr->filesz, phdr->offset) != (ssize_t)phdr->filesz) {
                    printf("_elf_load_phdr: Failure reading %u bytes of data into %p from %p\n", phdr->filesz, phdr->paddr, phdr->offset);
                    return -1;
                }
            }
            if(phdr->filesz < phdr->memsz) {
#if (DEBUG_EXEC_ELF)
                printf("  Clearing %6u bytes at             %p.\n", phdr->memsz - phdr->filesz, phdr->paddr + phdr->filesz);
#endif
                memset((void *)(phdr->paddr + phdr->filesz), 0, phdr->memsz - phdr->filesz);
            }
        }
    }

    return 0;
}

static int _elf_load(exec_hand_t *exec) {
    exec_elf_data_t *edata = exec->data;

    if(_elf_load_phdr(exec)) {
        goto elf_load_fail;
    }

    return 0;

elf_load_fail:
    free(edata->phdr);
    free(edata->ehdr);
    free(edata);

    return -1;
}

