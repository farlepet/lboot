#ifndef LBOOT_TIME_TIME_H
#define LBOOT_TIME_TIME_H

#include <stdint.h>

/**
 * Type for storing milliseconds since boot. Since the bootloader will not be
 * active for long, 32-bit is fine. If finer precision than milliseconds is
 * required, then it may need to switch to uint64_t.
 *
 * 2^32 - 1 ms = ~50 days 
 */
typedef uint32_t time_ticks_t;

#define TIME_TICKS_PER_SECOND (1000) /* 1 tick = 1 ms */

/**
 * @brief Initialize time system
 *
 * @return 0 on success, < 0 on failure
 */
int time_init(void);

/**
 * @brief Delay for the given duration in milliseconds
 *
 * @param ms Number of milliseconds to wait for
 */
void time_delay(uint32_t ms);

/**
 * @brief Get the current tick count
 *
 * @param ticks Where to store the current tick count
 */
void time_get(time_ticks_t *ticks);

/**
 * @brief Get the current tick count with the given offset into the future
 *
 * @param ticks Where to store tick count
 * @param ms Offset in milliseconds
 */
void time_offset(time_ticks_t *ticks, uint32_t ms);

/**
 * @brief Adds an offset to existing tick count
 *
 * @param ticks Tick count to add to
 * @param ms Offset to add in milliseconds
 */
void time_addoffset(time_ticks_t *ticks, uint32_t ms);

/**
 * @brief Checks if the provided tick count is in the past
 *
 * @param ticks Tick count to check
 * @return 0 if tick count is in the future, 1 if tick count is now or in the past
 */
int time_ispast(time_ticks_t *ticks);

#endif

