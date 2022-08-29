#include <stddef.h>
#include <string.h>

#include "io/output.h"
#include "mm/alloc.h"
#include "storage/fs/fs.h"

int fs_findfile(fs_hand_t *fs, const fs_file_t *dir, fs_file_t *file, const char *path) {
    if(*path == FS_PATHSEP) {
        /* Start at root */
        dir = NULL;
        path++;
    }

    if(!strlen(path)) {
        /* @todo Support getting root directory via '/'? */
        return -1;
    }

    char *_path = strdup(path);
    char *cpath = _path;

    /* Not the most elegant, but ensures we aren't reading and writing the same
     * variable in one `find` call. */
    fs_file_t tdir1;
    fs_file_t tdir2;

    while(*cpath) {
        char *sep = strchr(cpath, FS_PATHSEP);
        if(sep) {
            /* Recurse */
            *sep = '\0';
            if(fs->find(fs, dir, &tdir2, cpath)) {
                printf("Directory not found: %s\n", cpath);
                goto fs_findfile_fail;
            }
            if(!(tdir2.attr & FS_FILEATTR_DIRECTORY)) {
                printf("Not a directory\n");
                goto fs_findfile_fail;
            }
            if(dir == &tdir1) {
                fs->file_destroy(fs, &tdir1);
            }
            memcpy(&tdir1, &tdir2, sizeof(tdir1));
            dir = &tdir1;
            cpath = sep + 1;
        } else {
            if(fs->find(fs, dir, file, cpath)) {
                printf("File not found: %s\n", cpath);
                goto fs_findfile_fail;
            }

            break;
        }
    }

    if(dir == &tdir1) {
        fs->file_destroy(fs, &tdir1);
    }

    return 0;

fs_findfile_fail:
    free(_path);

    return -1;
}

