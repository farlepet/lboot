#ifndef FAT_H
#define FAT_H

#include <stdint.h>

#pragma pack(1)
/**
 * @brief FAT bootsector layout
 */
typedef struct {
    uint8_t  jump[3]; /**< Jump to actual code */
    char     name[8]; /**< OEM name, or name of formatting utility */
    uint16_t bytes_per_sector;    /**< Bytes per sector */
    uint8_t  sectors_per_cluster; /**< Sectors per cluster */
    uint16_t reserved_sectors;    /**< Number of reserved sectors */
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

/**
 * @brief FAT directory entry
 */
typedef struct {
    char     filename[11];   /**< Short file name */
    uint8_t  attr;          /**< Attributes */
    uint8_t  _reserved[10]; /**< Reserved: Used for VFAT, not yet supported */
    uint16_t time;          /**< Modification time, in FAT time format */
    uint16_t date;          /**< Modification date, in FAT date format */
    uint16_t start_cluster; /**< First cluster of file, 0 if file is empty */
    uint32_t filesize;      /**< Size of file in bytes */
} fat_dirent_t;
#pragma pack()

typedef struct {
    size_t      size;          /**< Filesize, in bytes */
    off_t       first_cluster; /**< Offset into filesystem to first data cluster, in bytes */
} fat_file_handle_t;

typedef struct {
    int               fd;           /**< File handle of disk image */
    size_t            cluster_size; /**< Size of cluster, in bytes */
    off_t             fat_offset;   /**< Offset into filesystem of first FAT */
    size_t            fat_size;     /**< Size of FAT, in bytes */
    off_t             data_offset;  /**< Offset into filesystem of first data cluster */
    fat_bootsector_t *bootsector;   /**< Pointer to bootsector loaded into memory */
    fat_file_handle_t root_dir;     /**< Root directory file handle */
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

/**
 * @brief Find file within a directory
 *
 * @note Does not recursively search
 *
 * @param hand FAT handle
 * @param dir Directory in which to search
 * @param fhand Handle in which to store file information
 * @param filename Filename to search for
 * @return int 0 if file is found, else non-zero
 */
int fat_find_file(fat_handle_t *hand, const fat_file_handle_t *dir, fat_file_handle_t *fhand, const char *filename);

/**
 * @brief Get list of cluster addresses occupied by file
 *
 * @param hand FAT handle
 * @param file File handle
 * @param clusters Where to store list of clusters, must be of appropriate size
 * @return int 0 on success, else non-zero
 */
int fat_get_file_clusters(fat_handle_t *hand, const fat_file_handle_t *file, uint32_t *clusters);

#endif
