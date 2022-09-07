#include "io/ioport.h"
#include "time/pit.h"

/**
 * @brief Gets the nearest PIT reload value for a given frequency.
 *
 * @param freq frequency in Hz
 * @return Reload value
 */
static uint16_t _get_reload(uint32_t freq)
{
	if(freq < 18)      return 0xffff; /* Minimum frequency */
	if(freq > 1193181) return 0x0001; /* Maximum frequency */
	return (1193180 / freq);
}

int pit_init(uint32_t freq) {
    uint16_t reload = _get_reload(freq);

    /* Channel 0: Rate generator, lo/hi byte access reload */
    outb(PIT_COMMAND, (2 << PIT_COMMAND_OPERMODE__POS)   |
                      (3 << PIT_COMMAND_ACCESSMODE__POS) |
                      (0 << PIT_COMMAND_CHANNEL__POS));
    /* Reload */
    outb(PIT0_DATA, (uint8_t)reload);
    outb(PIT0_DATA, (uint8_t)(reload >> 8));

    return 0;
}

