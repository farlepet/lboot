#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "fat.h"
#include "sector_map.h"

int _round1(int argc, char **argv);
int _round2(int argc, char **argv);
void _usage(void);

int main(int argc, char **argv) {
    if(argc < 2) {
        _usage();
        return -1;
    }

    int round = strtol(argv[1], NULL, 10);

    int ret = 0;

    switch(round) {
        case 1:
            ret = _round1(argc, argv);
            break;
        case 2:
            ret = _round2(argc, argv);
            break;
        default:
            _usage();
            ret = 1;
    }

    return ret;
}

/**
 * @brief Round 1 - Write sectors occupied by the stage2 loader to a file
 */
int _round1(int argc, char **argv) {
    /* @todo This function could do with splitting up */
    if(argc != 5) {
        _usage();
        return -1;
    }

    fat_handle_t fat;
    if(fat_open(&fat, argv[2], O_RDONLY)) {
        fprintf(stderr, "Could not load FAT file `%s`\n", argv[2]);
        return -1;
    }

    if(fat.cluster_size != 512) {
        fprintf(stderr, "Cluster sizes != 512 not currently supported, saw %lu\n", fat.cluster_size);
        return -1;
    }

    int map_fd = open(argv[4], O_WRONLY | O_CREAT, 0660);
    if(map_fd < 0) {
        fprintf(stderr, "Could not open mapmfile `%s` for writing: %s\n", argv[4], strerror(errno));
        return -1;
    }

    fat_file_handle_t stage2;
    if(fat_find_file(&fat, &fat.root_dir, &stage2, argv[3])) {
        fprintf(stderr, "Could not find stage 2 file `%s`\n", argv[3]);
        close(map_fd);
        fat_close(&fat);
        return -1;
    }

    unsigned n_clusters = stage2.size / fat.cluster_size;
    if(stage2.size % fat.cluster_size) {
        n_clusters++;
    }

    uint32_t *clusters = malloc(n_clusters * sizeof(uint32_t));
    if(clusters == NULL) {
        fprintf(stderr, "Could not allocate space for %u cluster addresses\n", n_clusters);
        close(map_fd);
        fat_close(&fat);
        return -1;
    }

    if(fat_get_file_clusters(&fat, &stage2, clusters)) {
        fprintf(stderr, "Could not obtain cluster addresses\n");
        free(clusters);
        close(map_fd);
        fat_close(&fat);
        return -1;
    }

    unsigned n_map_entries = 1;
    for(unsigned i = 1; i < n_clusters; i++) {
        //printf("%u: %08x -> %04lx\n", i, clusters[i], clusters[i] / fat.cluster_size);
        if(clusters[i] != (clusters[i-1] + fat.cluster_size)) {
            n_map_entries++;
        }
    }

    unsigned n_chunks = n_map_entries / SECTOR_MAP_ENTRIES;
    if(n_map_entries % SECTOR_MAP_ENTRIES) {
        n_chunks++;
    }
    sector_map_chunk_t *chunks = calloc(n_chunks, sizeof(sector_map_chunk_t));
    if(chunks == NULL) {
        fprintf(stderr, "Could not allocate space for %u cluster chunks\n", n_chunks);
        free(clusters);
        close(map_fd);
        fat_close(&fat);
        return -1;
    }

    unsigned c_map   = 0;
    unsigned c_chunk = 0;
    chunks[0].entries[0].sector = (clusters[0] / fat.cluster_size) * fat.bootsector->sectors_per_cluster;
    chunks[0].entries[0].count  = 1;
    for(unsigned i = 1; i < n_clusters; i++) {
        if(clusters[i] != (clusters[i-1] + fat.cluster_size)) {
            c_map++;
            if(c_map >= SECTOR_MAP_ENTRIES) {
                c_map = 0;
                c_chunk++;
            }
            chunks[c_chunk].entries[c_map].sector = (clusters[i] / fat.cluster_size) * fat.bootsector->sectors_per_cluster;
            chunks[c_chunk].entries[c_map].count  = 1;
        } else {
            chunks[c_chunk].entries[c_map].count++;
        }
    }

    for(unsigned i = 0; i < n_chunks; i++) {
        if(write(map_fd, &chunks[i], sizeof(sector_map_chunk_t)) != sizeof(sector_map_chunk_t)) {
            fprintf(stderr, "Could not write chunk to map file\n");
            free(chunks);
            free(clusters);
            close(map_fd);
            fat_close(&fat);
            return -1;
        }
    }

    free(chunks);
    free(clusters);
    close(map_fd);
    fat_close(&fat);

    return 0;
}

/**
 * @brief Round 2 - Update inter-chunk sector pointers for stage2 map file after
 *        being written to the filesystem, and write pointer to first chunk into
 *        the boot sector.
 */
int _round2(int argc, char **argv) {
    if(argc != 4) {
        _usage();
        return -1;
    }

    fat_handle_t fat;
    if(fat_open(&fat, argv[2], O_RDWR)) {
        return -1;
    }

    if(fat.cluster_size != 512) {
        fprintf(stderr, "Cluster sizes != 512 not currently supported, saw %lu\n", fat.cluster_size);
        return -1;
    }

    fat_file_handle_t map_file;
    if(fat_find_file(&fat, &fat.root_dir, &map_file, argv[3])) {
        fprintf(stderr, "Could not find map file `%s`\n", argv[3]);
        fat_close(&fat);
        return -1;
    }

    unsigned n_clusters = map_file.size / fat.cluster_size;
    if(map_file.size % fat.cluster_size) {
        n_clusters++;
    }

    uint32_t *clusters = malloc(n_clusters * sizeof(uint32_t));
    if(clusters == NULL) {
        fprintf(stderr, "Could not allocate space for %u cluster addresses\n", n_clusters);
        fat_close(&fat);
        return -1;
    }

    if(fat_get_file_clusters(&fat, &map_file, clusters)) {
        fprintf(stderr, "Could not obtain cluster addresses\n");
        free(clusters);
        fat_close(&fat);
        return -1;
    }

    off_t pre_addr = offsetof(fat_bootsector_t, stage2_map_sector);
    for(unsigned i = 0; i < n_clusters; i++) {
        uint16_t clust = (clusters[i] / fat.cluster_size) * fat.bootsector->sectors_per_cluster;
        //fprintf(stderr, "%08lx <- %04hx (%04x)\n", pre_addr, clust, clusters[i]);
        if(pwrite(fat.fd, &clust, 2, pre_addr) != 2) {
            fprintf(stderr, "Error while writing to image: %s\n", strerror(errno));
            free(clusters);
            fat_close(&fat);
            return -1;
        }
        pre_addr = clusters[i] + offsetof(sector_map_chunk_t, next_chunk);
    }
    /* Last sector pointer should already be zero from round 1. */

    free(clusters);
    fat_close(&fat);

    return 0;
}

void _usage(void) {
    fprintf(stderr, "USAGE:\n"
                    "  round 1: sector_mapper 1 <floppy image> <stage 2 filename> <map file>\n"
                    "  round 2: sector_mapper 2 <floppy image> <map filename>\n");

}
