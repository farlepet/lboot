#include <string.h>

#include "storage/bios.h"
#include "bios/bios.h"

/* @note since we currently allow only a single device open at a time, having
 * this statically declared should be fine. */
static storage_bios_data_t _bios_data = { 0 };

static ssize_t _read(storage_hand_t *storage, void *buff, off_t offset, size_t size);

int storage_bios_init(storage_hand_t *storage, uint8_t bios_dev) {
    memset(storage, 0, sizeof(storage_hand_t));

    if(bios_dev < 0x80) {
        /* INT 0x13, AH = 0x08 supposedly can report incorrect values for a
         * floppy disk. Assuming a standard 1.44MB disk geometry here. */
        _bios_data.sectors_per_track = 18;
        _bios_data.n_heads           = 2;
        storage->size                = 2880 * 512;
    } else {
        /* @todo */
        /*bios_call_t call;
        memset(&call, 0, sizeof(bios_call_t));
        call.int_n = 0x13;
        call.ax    = 0x0800;
        bios_call(&call);*/
        return -1;
    }

    storage->data = &_bios_data;
    storage->read = _read;

    return 0;
}

static int _floppy_read_sector(storage_bios_data_t *bdata, void *buff, off_t offset) {
    if((uint32_t)buff > 0xffff) {
        /* Address too large for real mode BIOS call. */
        return -1;
    }

    bios_call_t call;
    memset(&call, 0, sizeof(bios_call_t));

    uint16_t track  = offset / bdata->sectors_per_track;
    uint8_t  sector = (offset % bdata->sectors_per_track) + 1;
    uint8_t  head   = track % bdata->n_heads;
             track  = track / bdata->n_heads;

    int attempts = 4;

    while(--attempts) {
        call.int_n = 0x13;
        call.ax    = 0x0201;
        call.bx    = (uint16_t)(uintptr_t)buff;
        call.cl    = sector;
        call.ch    = track;
        call.dl    = bdata->bios_id;
        call.dh    = head;
        bios_call(&call);

        if(!(call.eflags & EFLAGS_CF)) {
            /* Read was successful */
            break;
        }

        /* @todo Reset controller */
    }

    if(attempts == 0) {
        /* Failed to read sector */
        return -1;
    }

    return 0;
}

static ssize_t _read(storage_hand_t *storage, void *buff, off_t offset, size_t size) {
    if((offset % 512) || (size % 512)) {
        /* Currently only support sector-aligned reads */
        return -1;
    }
    storage_bios_data_t *bdata = (storage_bios_data_t *)storage->data;

    size_t pos = 0;

    if(bdata->bios_id < 0x80) {
        /* Floppy */
        while (pos < size) {
            if(_floppy_read_sector(bdata, buff + pos, offset + pos)) {
                return -1;
            }
            pos += 512;
        }
    } else {
        /* @todo Hard disk */
        return -1;
    }

    return (ssize_t)pos;
}

