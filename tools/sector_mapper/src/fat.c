#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "fat.h"

int fat_open(fat_handle_t *hand, const char *path, int oflags) {
    if(hand == NULL) {
        return -1;
    }

    memset(hand, 0, sizeof(fat_handle_t));

    hand->fd = open(path, oflags);
    if(hand->fd < 0) {
        fprintf(stderr, "fat_open: Could not open `%s`: %s\n", path, strerror(errno));
        return -1;
    }

    hand->bootsector = malloc(sizeof(fat_bootsector_t));
    if(hand->bootsector == NULL) {
        fat_close(hand);
        fprintf(stderr, "fat_open: Could not allocate memory for boot sector\n");
    }

    if(read(hand->fd, hand->bootsector, sizeof(fat_bootsector_t)) != sizeof(fat_bootsector_t)) {
        fat_close(hand);
        fprintf(stderr, "fat_open: Could not allocate memory for boot sector\n");
    }

    return 0;
}

void fat_close(fat_handle_t *hand) {
    if(hand->bootsector) {
        free(hand->bootsector);
    }

    if(hand->fd) {
        close(hand->fd);
    }
}