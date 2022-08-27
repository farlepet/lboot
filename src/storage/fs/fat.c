#include <string.h>
#include <stddef.h>

#include "io/output.h"
#include "mm/alloc.h"
#include "storage/fs/fat.h"

typedef struct {
    off_t first_cluster; /**< Offset into filesystem to first data cluster, in bytes */
} fat_file_data_t;

typedef struct {
    uint16_t sector_size;  /**< Number of bytes per sector */
    uint16_t cluster_size; /**< Size of cluster, in bytes */
    off_t    fat_offset;   /**< Offset into filesystem of first FAT */
    size_t   fat_size;     /**< Size of FAT, in bytes */
    off_t    data_offset;  /**< Offset into filesystem of first data cluster */

    off_t    rootdir_first_cluster; /**< Offset into filesystem of first cluster of root directory, in bytes */
    size_t   rootdir_size;          /**< Size of root directory in bytes */

    fs_file_t       rootdir;       /**< File representing root directory - internal use only*/
    fat_file_data_t _rootdir_data; /**< Data for rootdir file, preventing an extra allocation - internal use only */

#define FAT_CACHE_CNT (4) /**< Number of FAT clusters to cache */
    struct {
        off_t   clust[FAT_CACHE_CNT]; /**< Offset of cached clusters into FS */
        uint8_t rank[FAT_CACHE_CNT];  /**< Rank of cache entry, representing which was last used */
        void   *buf;                  /**< Buffer of cluster caches */
    } cache;
} fat_data_t;

static ssize_t _fat_read(fs_hand_t *fs, const fs_file_t *file, void *buf, size_t sz, off_t off);
static int     _fat_find(fs_hand_t *fs, const fs_file_t *dir, fs_file_t *file, const char *name);
static int     _fat_file_destroy(fs_hand_t *fs, fs_file_t *file);

int fs_fat_init(fs_hand_t *fs, storage_hand_t *storage, off_t off) {
    memset(fs, 0, sizeof(fs_hand_t));

    fs->storage   = storage;
    fs->fs_offset = off;

    fat_bootsector_t *bootsec = (fat_bootsector_t *)alloc(sizeof(fat_bootsector_t), ALLOC_FLAG_16B);

    /* Read bootsector */
    if(storage->read(storage, bootsec, fs->fs_offset, 512) != 512) {
        free(bootsec);
        return -1;
    }

    fs->read         = _fat_read;
    fs->find         = _fat_find;
    fs->file_destroy = _fat_file_destroy;
    
    fat_data_t *fdata   = (fat_data_t *)alloc(sizeof(fat_data_t), 0);
    fs->data = fdata;
    
    /* @todo Currently assuming the bootsector is an accurate source of
     * information, this may not always be the case. */

    fdata->sector_size   = bootsec->bytes_per_sector;
    fdata->cluster_size  = bootsec->sectors_per_cluster * fdata->sector_size;
    fdata->fat_offset    = bootsec->reserved_sectors    * fdata->sector_size;
    fdata->fat_size      = bootsec->sectors_per_fat     * fdata->sector_size;

    fdata->rootdir.size                = bootsec->root_dir_entries * sizeof(fat_dirent_t);
    fdata->_rootdir_data.first_cluster = fdata->fat_offset + (fdata->fat_size * bootsec->fat_copies);
    fdata->rootdir.data                = &fdata->_rootdir_data;

    fdata->data_offset = fdata->_rootdir_data.first_cluster + fdata->rootdir.size;

    /* @note Currently only supporting FAT12 */
    fs->fs_size = bootsec->total_sectors * fdata->sector_size;

    fdata->cache.buf = alloc(fdata->cluster_size * FAT_CACHE_CNT, 0);

    free(bootsec);

    return 0;
}

/**
 * @brief Touch FAT cache entry, setting its rank to zero
 *
 * @param fdata FAT data structure
 * @param idx Cache entry to touch
 */
static void _fat_cache_touch(fat_data_t *fdata, unsigned idx) {
    for(unsigned i = 0; i < FAT_CACHE_CNT; i++) {
        if(i == idx) {
            continue;
        }
        if(fdata->cache.rank[i] < fdata->cache.rank[idx]) {
            fdata->cache.rank[i]++;
        }
    }
    fdata->cache.rank[idx] = 0;
}

/**
 * @brief Retrieve cached FAT entry, if available
 *
 * @param fdata FAT data structure
 * @param buf Buffer to store cluster into
 * @param clust Cluster to search for
 * @return 1 on cache hit, 0 on cache miss
 */
static int _fat_try_cache(fat_data_t *fdata, void *buf, off_t clust) {
    for(unsigned i = 0; i < FAT_CACHE_CNT; i++) {
        if(fdata->cache.clust[i] == clust) {
            if(fdata->cache.rank[i] != 0) {
                /* Update rank due to usage */
                _fat_cache_touch(fdata, i);
            }
            memcpy(buf, fdata->cache.buf + (i * fdata->cluster_size), fdata->cluster_size);
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Add cached FAT entry, overwriting oldest entry
 *
 * @param fdata FAT data structure
 * @param buf Buffer containing cluster data
 * @param clust Cluster address to store as
 */
static void _fat_add_cache(fat_data_t *fdata, void *buf, off_t clust) {
    unsigned entry = 0;

    /* Find highest-rank (oldest) cache entry */
    for(unsigned i = 1; i < FAT_CACHE_CNT; i++) {
        if(fdata->cache.rank[i] > fdata->cache.rank[entry]) {
            entry = i;
        }
    }

    fdata->cache.clust[entry] = clust;
    memcpy(fdata->cache.buf + (entry * fdata->cluster_size), buf, fdata->cluster_size);
    _fat_cache_touch(fdata, entry);
}

static uint32_t _fat_read_fat_data(fs_hand_t *fs, void *tmp, off_t off, uint8_t sz) {
    fat_data_t *fdata = (fat_data_t *)fs->data;

    off_t read_addr = off - (off % fdata->sector_size);

    if(!_fat_try_cache(fdata, tmp, read_addr)) {
        if(fs->storage->read(fs->storage, tmp, read_addr, fdata->cluster_size) != fdata->cluster_size) {
            return 0xffffffff;
        }
        _fat_add_cache(fdata, tmp, read_addr);
    }

    uint32_t data = *(uint32_t *)(tmp + (off - read_addr));

    switch(sz) {
        case 1: return data & 0x000000FF;
        case 2: return data & 0x0000FFFF;
        case 3: return data & 0x00FFFFFF;
        case 4: return data & 0xFFFFFFFF;
    }

    return 0xFFFFFFFF;
}

static uint32_t _fat_get_fat_entry(fs_hand_t *fs, void *tmp, uint32_t clust_num) {
    fat_data_t *fdata = (fat_data_t *)fs->data;

    /* @note Only FAT12 at the moment */
    off_t fat_entry_offset = fdata->fat_offset + ((clust_num * 3) / 2);

    uint16_t fat_entry = 0;
    if((fat_entry_offset % fdata->cluster_size) == (fdata->cluster_size - 1)) {
        /* Split across cluster boundry */
        fat_entry = _fat_read_fat_data(fs, tmp, fat_entry_offset, 1) | 
                    (_fat_read_fat_data(fs, tmp, fat_entry_offset + 1, 1) << 8);
    } else {
        fat_entry = _fat_read_fat_data(fs, tmp, fat_entry_offset, 2); 
    }

    if(clust_num & 1) {
        fat_entry >>= 4;
    }

    return fat_entry & 0xfff;
}

static off_t _fat_get_next_cluster(fs_hand_t *fs, void *tmp, off_t curr_clust) {
    const fat_data_t *fdata = (fat_data_t *)fs->data;
    
    if(curr_clust < fdata->data_offset) {
        if(curr_clust < fdata->_rootdir_data.first_cluster) {
            printf("ERROR: Cluster below root directory\n");
            return 0;
        }

        if((curr_clust + fdata->cluster_size) < fdata->data_offset) {
            /* Root directory is contiguous */
            return curr_clust + fdata->cluster_size;
        } else {
            /* End of root directory */
            return 0;
        }
    }

    uint32_t clust_num = ((curr_clust - fdata->data_offset) / fdata->cluster_size) + 2;
    uint32_t fat_entry = _fat_get_fat_entry(fs, tmp, clust_num);

    off_t next_clust = 0;
    if((fat_entry >= 0x002) && (fat_entry <= 0xff0)) {
        next_clust = fdata->data_offset + ((fat_entry - 2) * fdata->cluster_size);
    }

    return next_clust;
}

static ssize_t _fat_read(fs_hand_t *fs, const fs_file_t *file, void *buf, size_t sz, off_t off) {
#if (DEBUG_FS_FAT)
    printf("_fat_read(..., %p, %d, %d)\n", buf, sz, off);
#endif

    if((off + sz) > file->size) {
        panic("Attempt to read past end of file!");
    }

    const fat_data_t      *fdata    = (fat_data_t *)fs->data;
    const fat_file_data_t *filedata = (fat_file_data_t *)file->data;

    void *tmp = alloc(fdata->cluster_size, ALLOC_FLAG_16B);

    off_t cluster = filedata->first_cluster;
    /* Get to the desired cluster. */
    while(off >= fdata->cluster_size) {
        cluster = _fat_get_next_cluster(fs, tmp, cluster);
        if(!cluster) {
#if (DEBUG_FS_FAT)
            printf("ERROR: Unexpected end of file!\n");
#endif
            free(tmp);
            return -1;
        }
        off -= fdata->cluster_size;
    }


    size_t pos = 0;
    while(pos < sz) {
        if(fs->storage->read(fs->storage, tmp, cluster, fdata->cluster_size) != fdata->cluster_size) {
            free(tmp);
#if (DEBUG_FS_FAT)
            printf("ERROR: Could not read from FS!\n");
#endif
            return -1;
        }

        if((sz - pos) <= (size_t)(fdata->cluster_size - off)) {
            memcpy(buf + pos, tmp + off, sz - pos);
        } else {
            memcpy(buf + pos, tmp, fdata->cluster_size - off);

            cluster = _fat_get_next_cluster(fs, tmp, cluster);
            if(!cluster) {
#if (DEBUG_FS_FAT)
                printf("ERROR: Unexpected end of file!\n");
#endif
                free(tmp);
                return -1;
            }
        }

        off = 0;
        pos += fdata->cluster_size;
    }

    free(tmp);
    
    return sz;
}

/**
 * @brief Compare FAT file name to desired filename
 *
 * @param fat_name FAT file name
 * @param filename desired file name
 * @return int 0 on match, else non-zero
 */
static int _fat_strcmp(const char *fat_name, const char *filename) {
    unsigned fidx = 0;

    while(*filename) {
        if(*filename == '.') {
            while(fat_name[fidx] == ' ') {
                fidx++;
            }
            filename++;
            continue;
        }
        if(fat_name[fidx] != *filename) {
            return -1;
        }
        filename++;
        fidx++;
    }
    while(fidx <= 11) {
        if(fat_name[fidx] != ' ') {
            /* Trailing character */
            return -1;
        }
        fidx++;
    }
    return 0;
}

static void _fat_pop_file(fs_hand_t *fs, fs_file_t *file, const fat_dirent_t *dent) {
    const fat_data_t *fdata = (fat_data_t *)fs->data;
    
    fat_file_data_t *filedata = (fat_file_data_t *)alloc(sizeof(fat_file_data_t), 0);
    filedata->first_cluster = fdata->data_offset + ((dent->start_cluster - 2) * fdata->cluster_size);

    file->fs   = fs;
    file->data = filedata;
    file->size = dent->filesize;
}

static int _fat_find(fs_hand_t *fs, const fs_file_t *dir, fs_file_t *file, const char *name) {
#if (DEBUG_FS_FAT)
    printf("_fat_find %s\n", name);
#endif

    const fat_data_t *fdata = (fat_data_t *)fs->data;

    if(dir == NULL) {
        dir = &fdata->rootdir;
    }

    if(dir->data == NULL) {
        panic("Directory data NULL!");
    }

    if(!strcmp(name, ".")) {
        /* Current directory, just copy data */
        memcpy(file, dir, sizeof(fs_file_t));
        file->data = alloc(sizeof(fat_file_data_t), 0);
        memcpy(file->data, dir->data, sizeof(fat_file_data_t));
        return 0;
    }

    fat_dirent_t *dirents = (fat_dirent_t *)alloc(fdata->cluster_size, 0);

    off_t pos = 0;
    while((size_t)pos < dir->size) {
        /* Read one cluster at a time. */
        if(_fat_read(fs, dir, dirents, fdata->cluster_size, pos) != fdata->cluster_size) {
            free(dirents);
            return -1;
        }

        for(unsigned i = 0; i < (dir->size / sizeof(fat_dirent_t)); i++) {
#if (DEBUG_FS_FAT)
            if(dirents[i].filename[0]) {
                printf("  %3u: %s\n", (i + (pos / sizeof(fat_dirent_t))), dirents[i].filename);
            }
#endif
            if(!_fat_strcmp(dirents[i].filename, name)) {
                _fat_pop_file(fs, file, &dirents[i]);
                free(dirents);
                return 0;
            }
        }

        pos += fdata->cluster_size;
    }

    free(dirents);
    return -1;
}

static int _fat_file_destroy(fs_hand_t *fs, fs_file_t *file) {
    const fat_data_t *fdata = (fat_data_t *)fs->data;

    if(file == &fdata->rootdir) {
        panic("Attempted to destroy static root directory!");
    }

    free(file->data);    

    return 0;
}

