#ifndef LBOOT_STORAGE_FS_FS_H
#define LBOOT_STORAGE_FS_FS_H

#include <stdint.h>

typedef struct fs_hand_struct fs_hand_t;

#include "storage/storage.h"
#include "storage/file.h"

#define FS_PATHSEP '/' /**< Path separation character. */

/**
 * @brief Filesystem handle
 */
struct fs_hand_struct {
    void           *data;      /**< Data pointer used by the FS driver */

    storage_hand_t *storage;   /**< Storage device that filesystem resides on */
    off_t           fs_offset; /**< Offset into storage device at which filesystem begins */
    size_t          fs_size;   /**< Size of filesystem area, in byte */

    /**
     * @brief Find a file within a directory
     *
     * @note Does not recursively search directories in filenames including a
     * path separator, @see fs_findfile
     *
     * @param fs Filesystem handle
     * @param dir Handle of directory to search within, if NULL default to root directory
     * @param file Handle in which to store file information
     * @param name Name of file to search for
     * @return int 0 on success, else < 0
     */
    int (*find)(fs_hand_t *fs, const file_hand_t *dir, file_hand_t *file, const char *name);
};

/**
 * @brief Recursively find a file within a directory
 *
 * @note If path starts with '/', search will begin at the root of the provided
 * filesystem
 *
 * @param fs Filesystem handle
 * @param dir Handle of directory to search within
 * @param file Handle in which to store file information
 * @param name Name of file to search for
 * @return int 0 on success, else < 0
 */
int fs_findfile(fs_hand_t *fs, const file_hand_t *dir, file_hand_t *file, const char *path);

#endif

