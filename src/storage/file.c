#include <stddef.h>
#include <string.h>

#include "mm/alloc.h"
#include "storage/file.h"
#include "storage/protocol/protocol.h"

static fs_hand_t *_default_fs;

int file_open(file_hand_t *file, const char *path) {
    if(0) {
#ifdef CONFIG_PROTOCOL
    } else if(strchr(path, ':')) {
        /* Assume URI */
        protocol_hand_t *proto = alloc(sizeof(protocol_hand_t), 0);
        if(protocol_init(proto, path)) {
            free(proto);
            return -1;
        }
        /* @todo Filename extraction */
        if(proto->recv(proto, file, path)) {
            free(proto);
            return -1;
        }

        protocol_close(proto);
#endif /* CONFIG_PROTOCOL */
    } else {
        if(_default_fs) {
            if(fs_findfile(_default_fs, NULL, file, path)) {
                return -1;
            }
        } else {
            return -1;
        }
    }

    return 0;
}

void file_set_default_fs(fs_hand_t *fs) {
    _default_fs = fs;
}

