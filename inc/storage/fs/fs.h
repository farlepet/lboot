#ifndef LBOOT_STORAGE_FS_FS_H
#define LBOOT_STORAGE_FS_FS_H

#include <stdint.h>

#include "storage/storage.h"

typedef struct fs_hand_struct fs_hand_t;
typedef struct fs_file_struct fs_file_t;

/**
 * @brief Filesystem handle
 */
struct fs_hand_struct {
    void           *data;      /**< Data pointer used by the FS driver */

    storage_hand_t *storage;   /**< Storage device that filesystem resides on */
    off_t           fs_offset; /**< Offset into storage device at which filesystem begins */
    size_t          fs_size;   /**< Size of filesystem area, in byte */

    /**
     * @brief Read data from a file into a buffer
     *
     * @param fs Filesystem handle
     * @param file Handle of file to read from
     * @param buf Where to read data into
     * @param sz Number of bytes to read
     * @param off Offset into file to start reading at
     * @return ssize_t Number of bytes read on success, else < 0
     */
    ssize_t (*read)(fs_hand_t *fs, const fs_file_t *file, void *buf, size_t sz, off_t off);

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
    int (*find)(fs_hand_t *fs, const fs_file_t *dir, fs_file_t *file, const char *name);

    /**
     * @brief Destroy a file handle, freeing any allocated memory
     *
     * @param fs Filesystem handle
     * @param file File handle to destroy
     * @return int 0 on success, else < 0
     */
    int (*file_destroy)(fs_hand_t *fs, fs_file_t *file);
};

/**
 * @brief File handle
 */
struct fs_file_struct {
    fs_hand_t *fs;   /**< Pointer to handle of owning filesystem. */
    void      *data; /**< Data pointer used by the FS driver */

    size_t     size; /**< File size in bytes */
    uint32_t   attr; /**< File attributes */
#define FS_FILEATTR_FILE      (1UL << 0)
#define FS_FILEATTR_DIRECTORY (1UL << 1)
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
int fs_findfile(fs_hand_t *fs, const fs_file_t *dir, fs_file_t *file, const char *path);

#endif

