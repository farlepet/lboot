#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "fat.h"

int _round1(int argc, char **argv);
int _round2(int argc, char **argv);
void _usage(void);

int main(int argc, char **argv) {
    if(argc < 2) {
        _usage();
        return -1;
    }

    int round = strtol(argv[1], NULL, 10);

    int ret = 0;

    switch(round) {
        case 1:
            ret = _round1(argc, argv);
            break;
        case 2:
            ret = _round2(argc, argv);
            break;
        default:
            _usage();
            ret = 1;
    }

    return ret;
}

/**
 * @brief Round 1 - Write sectors occupied by the stage2 loader to a file
 */
int _round1(int argc, char **argv) {
    if(argc != 5) {
        _usage();
        return -1;
    }

    fat_handle_t fat;
    if(fat_open(&fat, argv[2], O_RDONLY)) {
        return -1;
    }

    fat_file_handle_t stage2;
    if(fat_find_file(&fat, &fat.root_dir, &stage2, argv[3])) {
        fprintf(stderr, "Could not find stage 2 file `%s`\n", argv[3]);
        return -1;
    }

    fat_close(&fat);

    return 0;
}

/**
 * @brief Round 2 - Update inter-chunk sector pointers for stage2 map file after
 *        being written to the filesystem, and write pointer to first chunk into
 *        the boot sector.
 */
int _round2(int argc, char **argv) {
    if(argc != 4) {
        _usage();
        return -1;
    }

    fat_handle_t fat;
    if(fat_open(&fat, argv[2], O_RDWR)) {
        return -1;
    }

    fat_close(&fat);

    return 0;
}

void _usage(void) {
    fprintf(stderr, "USAGE:\n"
                    "  round 1: sector_mapper 1 <floppy image> <stage 2 filename> <map file>\n"
                    "  round 2: sector_mapper 2 <floppy image> <map filename>\n");

}
