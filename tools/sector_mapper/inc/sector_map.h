#ifndef SECTOR_MAP_H
#define SECTOR_MAP_H

#include <stdint.h>

/**
 * @brief Sector map entry, representing a consecutive set of 1 or more sectors
 */
typedef struct {
    uint16_t sector; /**< Start sector. Value of 0 represents the end. */
    uint16_t count;  /**< Consecutive sector count */
} sector_map_entry_t;

#define SECTOR_MAP_ENTRIES ((512 - 4) / sizeof(sector_map_entry_t))

/**
 * @brief Sector map chunk
 */
typedef struct {
    sector_map_entry_t entries[SECTOR_MAP_ENTRIES]; /**< Array of sector areas containing stage 2 loader */
    uint16_t           next_chunk;                  /**< Sector containing next map chunk. 0 signifies end. */
    uint16_t           _reserved;
} sector_map_chunk_t;

#endif
