#include <stddef.h>

#include "intr/interrupts.h"
#include "io/output.h"
#include "time/pit.h"
#include "time/time.h"

static volatile time_ticks_t _current_ticks;

static void _pit_handler(uint8_t int_id, uint32_t errno, void *data);

int time_init(void) {
    _current_ticks = 0;
    pit_init(TIME_TICKS_PER_SECOND);

    interrupt_register(INT_ID_PIT, _pit_handler, NULL);
    interrupt_enable(INT_ID_PIT);

    return 0;
}

static void _pit_handler(uint8_t int_id, uint32_t errno, void *data) {
    (void)int_id;
    (void)errno;
    (void)data;
    _current_ticks++;
}

void time_get(time_ticks_t *ticks) {
    *ticks = _current_ticks;
}

void time_offset(time_ticks_t *ticks, uint32_t ms) {
    *ticks = _current_ticks + (ms * (TIME_TICKS_PER_SECOND / 1000));
}

void time_addoffset(time_ticks_t *ticks, uint32_t ms) {
    *ticks += (ms * (TIME_TICKS_PER_SECOND / 1000));
}

int time_ispast(time_ticks_t *ticks) {
    return (*ticks <= _current_ticks);
}

void time_delay(uint32_t ms) {
    if(!interrupts_enabled()) {
        panic("Attempt to time_delay() while interrupts disabled");
    }

    time_ticks_t end;
    time_offset(&end, ms);
    while(!time_ispast(&end)) {
        asm volatile("hlt");
    }
}

