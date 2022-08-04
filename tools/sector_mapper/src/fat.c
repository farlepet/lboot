#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "fat.h"

#define DEBUG 0
#if DEBUG
#  define FAT_DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#  define FAT_DEBUG(...)
#endif
#define FAT_ERROR(...) fprintf(stderr, __VA_ARGS__)

int fat_open(fat_handle_t *hand, const char *path, int oflags) {
    if(hand == NULL) {
        return -1;
    }

    memset(hand, 0, sizeof(fat_handle_t));

    hand->fd = open(path, oflags);
    if(hand->fd < 0) {
        FAT_ERROR("fat_open: Could not open `%s`: %s\n", path, strerror(errno));
        return -1;
    }

    hand->bootsector = malloc(sizeof(fat_bootsector_t));
    if(hand->bootsector == NULL) {
        fat_close(hand);
        FAT_ERROR("fat_open: Could not allocate memory for boot sector\n");
        return -1;
    }

    if(read(hand->fd, hand->bootsector, sizeof(fat_bootsector_t)) != sizeof(fat_bootsector_t)) {
        fat_close(hand);
        FAT_ERROR("fat_open: Could not allocate memory for boot sector\n");
        return -1;
    }

    hand->cluster_size = hand->bootsector->sectors_per_cluster * hand->bootsector->bytes_per_sector;
    hand->fat_offset   = hand->bootsector->reserved_sectors    * hand->bootsector->bytes_per_sector;
    hand->fat_size     = hand->bootsector->sectors_per_fat     * hand->bootsector->bytes_per_sector;

    hand->root_dir.first_cluster = hand->fat_offset + (hand->fat_size * hand->bootsector->fat_copies);
    hand->root_dir.size          = hand->bootsector->root_dir_entries * sizeof(fat_dirent_t);

    hand->data_offset = hand->root_dir.first_cluster + hand->root_dir.size;

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

/**
 * @brief Get an entry from the FAT for a given cluster
 *
 * @param hand FAT handle
 * @param cluster_num Cluster number relative to the first data cluster
 * @return uint32_t Value of FAT, or 0xFFFFFFFF on error
 */
static uint32_t _get_fat_entry(fat_handle_t *hand, uint32_t cluster_num) {
    /* @todo Support FAT16/32 */

    off_t fat_entry_offset = hand->fat_offset + ((cluster_num * 3) / 2);

    uint16_t fat_entry;

    if(pread(hand->fd, &fat_entry, 2, fat_entry_offset) != 2) {
        FAT_ERROR("_get_fat_entry: Could not read FAT entry at %lu\n", fat_entry_offset);
        return 0xFFFFFFFF;
    }

    if(cluster_num & 0x01) {
        fat_entry >>= 4;
    }

    return fat_entry & 0xFFF;
}

/**
 * @brief Get the next cluster of a file
 *
 * @param hand FAT handle
 * @param cluster Offset of cluster into filesystem
 * @return off_t Offset of next cluster into filesystem, or 0 if it does not exist
 */
static off_t _get_next_cluster(fat_handle_t *hand, off_t cluster) {
    if(cluster < hand->data_offset) {
        if(cluster < hand->root_dir.first_cluster) {
            FAT_ERROR("_get_next_cluster: %04lx is below root directory\n", cluster);
            return 0;
        }

        /* @note Assuming `cluster` is aligned properly */
        if((off_t)(cluster + hand->cluster_size) < hand->data_offset) {
            /* Root directory is continuous */
            return cluster + hand->cluster_size;
        } else {
            /* End of root directory */
            return 0;
        }
    }

    uint32_t cluster_num = (cluster - hand->data_offset) / hand->cluster_size;
    uint32_t fat_entry   = _get_fat_entry(hand, cluster_num);

    off_t next_cluster = 0;
    if((fat_entry >= 0x002) && (fat_entry < 0xff0)) {
        next_cluster = hand->data_offset + (fat_entry * hand->cluster_size);
    }

    FAT_DEBUG("_get_next_cluster: %08lx -> %08lx (%03x)\n", cluster, next_cluster, fat_entry);

    return next_cluster;
}

/**
 * @brief Populate file handle with information from FAT directory entry
 *
 * @param hand FAT handle
 * @param file File handle to populate
 * @param dirent Dirent to source data from
 */
static void _populate_file_handle(fat_handle_t *hand, fat_file_handle_t *file, const fat_dirent_t *dirent) {
    file->first_cluster = hand->data_offset + (dirent->start_cluster * hand->cluster_size);
    file->size          = dirent->filesize;
}

/**
 * @brief Compare FAT file name to desired filename
 *
 * @param fat_name FAT file name
 * @param filename desired file name
 * @return int 0 on match, else non-zero
 */
static int _fat_strcmp(const char *fat_name, const char *filename) {
    size_t len = 11;
    while(len && (fat_name[len-1] == ' ')) { len--; }

    if(strlen(filename) != len) {
        return -1;
    }
    return strncmp(fat_name, filename, len);
}

int fat_find_file(fat_handle_t *hand, const fat_file_handle_t *dir, fat_file_handle_t *fhand, const char *filename) {
    if(strlen(filename) > 11) {
        /* Long filenames not yet supported */
        return -1;
    }

    off_t  dent_off = dir->first_cluster;
    size_t curr_pos = 0;

    fat_dirent_t *dents = (fat_dirent_t *)malloc(hand->cluster_size);
    if(dents == NULL) {
        FAT_ERROR("fat_find_file: Could not allocate memory for directory entry cluster\n");
        return -1;
    }

    /* @note Perhaps it would be better to just read the entire directory file
     * into a buffer, then traverse it. */
    while(curr_pos < dir->size) {
        size_t dent_sz = hand->cluster_size;
        if((dent_sz + curr_pos) > dir->size) {
            dent_sz = (dir->size - curr_pos);
        }

        if((size_t)pread(hand->fd, dents, dent_sz, dent_off) != dent_sz) {
            FAT_ERROR("fat_find_file: Could not read directory entry cluster\n");
            goto file_not_found;
        }

        for(size_t i = 0; i < (dent_sz / sizeof(fat_dirent_t)); i++) {
            FAT_DEBUG("file: `%11s`\n", dents[i].filename);
            if(!_fat_strcmp(dents[i].filename, filename)) {
                _populate_file_handle(hand, fhand, &dents[i]);
                free(dents);
                return 0;
            }
        }

        curr_pos += hand->cluster_size;
        dent_off = _get_next_cluster(hand, dent_off);
        if((curr_pos < dir->size) && (dent_off == 0)) {
            FAT_ERROR("fat_find_file: Unexpected end of directory entries\n");
            goto file_not_found;
        }
    }
    /* File not found */

file_not_found:
    free(dents);

    return -1;
}

int fat_get_file_clusters(fat_handle_t *hand, const fat_file_handle_t *file, uint32_t *clusters) {
    size_t pos = 0;

    off_t clust = file->first_cluster;

    while(pos < file->size) {
        if(clust == 0) {
            FAT_ERROR("fat_get_file_clusters: Unexpected end of cluster chain\n");
            return -1;
        }
        *(clusters++) = (uint32_t)clust;
        pos          += hand->cluster_size;
        clust         = _get_next_cluster(hand, clust);
    }

    return 0;
}
