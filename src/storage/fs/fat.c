#include <string.h>

#include "storage/fs/fat.h"

typedef struct {
    off_t first_cluster; /**< Offset into filesystem to first data cluster, in bytes */
} fat_file_data_t;

typedef struct {
    size_t            cluster_size; /**< Size of cluster, in bytes */
    off_t             fat_offset;   /**< Offset into filesystem of first FAT */
    size_t            fat_size;     /**< Size of FAT, in bytes */
    off_t             data_offset;  /**< Offset into filesystem of first data cluster */
} fat_data_t;

/** Scratch memory.
 *  @todo Added a basic shared memory system for temporary memory buffers. */
static uint8_t _scratch[512];

static ssize_t _fat_read(fs_hand_t *fs, fs_file_t *file, void *buf, size_t sz, off_t off);
static int     _fat_find(fs_hand_t *fs, const fs_file_t *dir, fs_file_t *file, const char *name);
static int     _fat_file_destroy(fs_hand_t *fs, fs_file_t *file);


int fs_fat_init(fs_hand_t *fs, storage_hand_t *storage, off_t off) {
    memset(fs, 0, sizeof(fs_hand_t));

    fs->storage   = storage;
    fs->fs_offset = off;

    /* Read bootsector */
    if(storage->read(storage, _scratch, fs->fs_offset, 512) != 512) {
        return -1;
    }
    const fat_bootsector_t *bootsec = (fat_bootsector_t *)_scratch;

    /* @note Currently only supporting FAT12 */
    fs->fs_size = bootsec->total_sectors * bootsec->bytes_per_sector;

    fs->read         = _fat_read;
    fs->find         = _fat_find;
    fs->file_destroy = _fat_file_destroy;

    return 0;
}

static ssize_t _fat_read(fs_hand_t *fs, fs_file_t *file, void *buf, size_t sz, off_t off) {
    /* @todo */
    (void)fs;
    (void)file;
    (void)buf;
    (void)sz;
    (void)off;

    return -1;
}

static int _fat_find(fs_hand_t *fs, const fs_file_t *dir, fs_file_t *file, const char *name) {
    /* @todo */
    (void)fs;
    (void)dir;
    (void)file;
    (void)name;

    return -1;
}

static int _fat_file_destroy(fs_hand_t *fs, fs_file_t *file) {
    /* @todo */
    (void)fs;
    (void)file;

    return -1;
}

