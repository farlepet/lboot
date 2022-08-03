#ifndef FAT_H
#define FAT_H

#include <stdint.h>

#pragma pack(1)
typedef struct {
    uint8_t  jump[3]; /**< Jump to actual code */
    char     name[8]; /**< OEM name, or name of formatting utility */
    uint16_t bytes_per_sector;    /**< Bytes per sector */
    uint8_t  sectors_per_cluster; /**< Sectors per cluster */
    uint16_t reserved_clusters;   /**< Number of reserved clusters */
    uint8_t  fat_copies;          /**< Number of FAT copies */
    uint16_t root_dir_entries;    /**< Number of root directory entries */
    uint16_t total_sectors;       /**< Total sectors in the filesystem, overridden by `total_sectors_big` */
    uint8_t  media_desc_type;     /**< Media descriptor type */
    uint16_t sectors_per_fat;     /**< Sectors per FAT */
    uint16_t sectors_per_track;   /**< Sectors per track */
    uint16_t heads;               /**< Number of heads */
    /* @note FAT12 need not have all the data following this point (apart from a
     * 16-bit hidden_sectors), but most modern implementations will. */
    uint32_t hidden_sectors;      /**< Number of hidden sectors */
    uint32_t total_sectors_big;   /**< Total sectors in filesystem */

    union {
        struct {
            uint8_t  drive_number;     /**< BIOS drive number of this drive */
            uint8_t  _reserved;        /**< Reserved: Different meanings on different systems */
            uint8_t  ext_signature;    /**< Extended signature: If 0x28 or 0x29, the following two fields are present. */
            uint32_t serial_number;    /**< Volume serial number */
            char     volume_label[11]; /**< Volume label */
            char     fs_type[8];       /**< Filesystem type ["FAT12   ", "FAT16   ", "FAT     ", "\0"] */
            uint8_t  code[444];        /**< Boot sector code */
        } fat12; /**< FAT12/FAT16 */

        struct {
            uint32_t sectors_per_fat_big; /**< Sectors per FAT, used in place of the 16-bit version */
            uint16_t mirror_flags;        /**< FAT mirror flags */
            uint16_t fs_version;          /**< Filesystem version */
            uint32_t root_cluster;        /**< First cluster of root directory */
            uint16_t info_sector;         /**< Filesystem information sector */
            uint16_t bs_backup;           /**< Boot sector backup sector */
            uint8_t  _reserved0[11];      /**< Reserved */
            uint8_t  drive_number;        /**< BIOS drive number of this drive */
            uint8_t  _reserved1;          /**< Reserved */
            uint8_t  ext_signature;       /**< Extended signature: If 0x28 or 0x29, the following two fields are present. */
            uint32_t serial_number;       /**< Volume serial number */
            char     volume_label[11];    /**< Volume label */
            char     fs_type[8];          /**< Filesystem type ("FAT32   ") */
            uint8_t  code[416];           /**< Boot sector code */
        } fat32; /**< FAT32 */
    };

    uint16_t stage2_map_sector; /**< lboot-specific: First sector of stage2 sector map */
    uint16_t stage2_addr;       /**< lboot-specific: Address at which to load stage2 */

    uint16_t signature;         /**< Boot sector signatore: 0x55, 0xAA */
} fat_bootsector_t;
#pragma pack()

typedef struct {
    int fd;                       /**< File handle of disk image */
    fat_bootsector_t *bootsector; /**< Pointer to bootsector loaded into memory */
} fat_handle_t;

/**
 * @brief Create and open FAT handle
 *
 * @param hand FAT handle to setup
 * @param path Path to FAT filesystem
 * @param oflags Flags to open filesystem with
 * @return int 0 on success, -1 on failure
 */
int fat_open(fat_handle_t *hand, const char *path, int oflags);

/**
 * @brief Close FAT handle
 *
 * Frees allocated memory, and closes file.
 *
 * @param hand FAT handle
 */
void fat_close(fat_handle_t *hand);

#endif
