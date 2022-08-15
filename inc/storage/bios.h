#ifndef LBOOT_STORAGE_BIOS_H
#define LBOOT_STORAGE_BIOS_H

#include "storage/storage.h"

/**
 * @brief Data structure for holding parameters specific to the BIOS disk driver.
 */
typedef struct {
    uint8_t  bios_id;           /**< BIOS drive ID */

    uint16_t sectors_per_track; /**< Sectors per track */
    uint16_t n_heads;           /**< Number of heads */
} storage_bios_data_t;

/**
 * @brief Setup storage handle from BIOS device ID
 *
 * @param storage Storage handle to populate
 * @param bios_dev BIOS device ID
 * @return int 0 on success, else < 0 on error
 */
int storage_bios_init(storage_hand_t *storage, uint8_t bios_dev);

#endif

