#ifndef LBOOT_STORAGE_FILE_H
#define LBOOT_STORAGE_FILE_H

#include "storage/fs/fs.h"

typedef struct file_hand_struct file_hand_t;

/**
 * @brief File handle
 */
struct file_hand_struct {
    fs_hand_t *fs;   /**< Pointer to handle of owning filesystem. */
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

#endif

