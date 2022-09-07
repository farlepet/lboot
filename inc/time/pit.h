#ifndef LBOOT_TIME_PIT_H
#define LBOOT_TIME_PIT_H

#include <stdint.h>

/* PIT I/O Ports */
#define PIT_BASE (0x40)

#define PIT0_DATA   (PIT_BASE + 0)
#define PIT1_DATA   (PIT_BASE + 1)
#define PIT2_DATA   (PIT_BASE + 2)
#define PIT_COMMAND (PIT_BASE + 3)

#define PIT_COMMAND_BCD             (1U << 0) /**< 0: 16-bit binary, 1: 4-digit BCD */
#define PIT_COMMAND_OPERMODE__POS   (      1) /**< Operating mode */
#define PIT_COMMAND_ACCESSMODE__POS (      4) /**< Access mode */
#define PIT_COMMAND_CHANNEL__POS    (      6) /**< Channel select */

/**
 * @brief Initialize PIT channel 0 to the given frequency
 *
 * @note Does not touch interrupt registration
 *
 * @param freq Desired frequency
 * @return 0 on success, < 0 on failure
 */
int pit_init(uint32_t freq);

#endif

