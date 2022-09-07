#ifndef LBOOT_STORAGE_FILE_H
#define LBOOT_STORAGE_FILE_H

typedef struct file_hand_struct file_hand_t;

#include "storage/fs/fs.h"
#include "storage/protocol/protocol.h"

/**
 * @brief File handle
 */
struct file_hand_struct {
    union {
        fs_hand_t       *fs;    /**< Pointer to handle of owning filesystem */
        protocol_hand_t *proto; /**< Pointer to handle of owning protocol */
    };

    void      *data; /**< Data pointer used by the FS driver */

    size_t     size; /**< File size in bytes */
    uint32_t   attr; /**< File attributes */
#define FS_FILEATTR_FILE      (1UL << 0)
#define FS_FILEATTR_DIRECTORY (1UL << 1)

    /**
     * @brief Read data from a file into a buffer
     *
     * @param file Handle of file to read from
     * @param buf Where to read data into
     * @param sz Number of bytes to read
     * @param off Offset into file to start reading at
     * @return ssize_t Number of bytes read on success, else < 0
     */
    ssize_t (*read)(const file_hand_t *file, void *buf, size_t sz, off_t off);

    /**
     * @brief Close a file handle, freeing any allocated memory
     *
     * @param file File handle to destroy
     * @return int 0 on success, else < 0
     */
    int (*close)(file_hand_t *file);
};

/**
 * @brief Find and open a file
 *
 * @param file File handle to populate
 * @param path Path or URI to file
 * @return 0 on success, < 0 on failure
 */
int file_open(file_hand_t *file, const char *path);

/**
 * @brief Sets default filesystem to search when opening a file
 *
 * @param fs Filesystem handle
 */
void file_set_default_fs(fs_hand_t *fs);

#endif

