#include <string.h>

#include "bios/bios.h"
#include "io/output.h"
#include "storage/bios.h"

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

static void _floppy_reset(uint8_t bios_id) {
    bios_call_t call;
    memset(&call, 0, sizeof(bios_call_t));

    call.int_n = 0x13;
    call.ax    = 0x00;
    call.dl    = bios_id;
    bios_call(&call);
}

static int _floppy_read_sector(storage_bios_data_t *bdata, void *buff, off_t offset) {
    if((uint32_t)buff > 0xffff) {
        panic("Attempted to read from floppy into an invalid memory address!");
    }

    bios_call_t call;
    memset(&call, 0, sizeof(bios_call_t));

    offset /= 512;

    uint16_t track  = offset / bdata->sectors_per_track;
    uint8_t  sector = (offset % bdata->sectors_per_track) + 1;
    uint8_t  head   = track % bdata->n_heads;
             track  = track / bdata->n_heads;

#if (DEBUG_STORAGE_BIOS)
    printf(" [%02hu,%02hhu,%02hhu]", track, head, sector);
#endif

    int attempts = 4;

    while(--attempts) {
#if (DEBUG_STORAGE_BIOS)
        printf(" TRY");
#endif
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

        _floppy_reset(bdata->bios_id);
    }

    status_working(WORKING_STATUS_WORKING);

    if(attempts == 0) {
        /* Failed to read sector */
        return -1;
    }

    return 0;
}

static ssize_t _read(storage_hand_t *storage, void *buff, off_t offset, size_t size) {
#if (DEBUG_STORAGE_BIOS)
    printf("_bios_read(..., %p, %5d, %4d)", buff, offset, size);
#endif

    if((offset % 512) || (size % 512)) {
        /* Currently only support sector-aligned reads */
        panic("Address or size not aligned to sector count!");
    }
    if(offset < 0) {
        panic("Negative offset!");
    }

    storage_bios_data_t *bdata = (storage_bios_data_t *)storage->data;

    size_t pos = 0;

    if(bdata->bios_id < 0x80) {
        /* Floppy */
        while (pos < size) {
            if(_floppy_read_sector(bdata, buff + pos, offset + pos)) {
#if (DEBUG_STORAGE_BIOS)
                printf(" FAIL\n");
#endif
                return -1;
            }
            pos += 512;
        }
    } else {
        /* @todo Hard disk */
        return -1;
    }

#if (DEBUG_STORAGE_BIOS)
#  if (DEBUG_STORAGE_BIOS > 1)
    uint16_t chksum = 0;
    uint8_t *data   = buff;
    for(size_t i = 0; i < size; i++) {
        chksum += data[i];
    }
    printf(" [CHK: %4x]", chksum);
#  endif
    printf(" OK\n");
#endif

    return (ssize_t)pos;
}

