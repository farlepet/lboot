#ifndef LBOOT_CONFIG_H
#define LBOOT_CONFIG_H

/*
 * USE
 */

/** Whether to use serial as main output device. */
#define USE_SERIAL (0)

/*
 * FEATURE
 */

/** Print file and line number in panic(). */
#define FEATURE_VERBOSE_PANIC   (1)
/** Enable status=bar showing current operations. Only available via VGA. */
#define FEATURE_STATUSBAR       (1)
/** Show working status when nothing else is being displayed. */
#define FEATURE_WORKINGSTATUS   (1)
/** Enable serial file transfer protocol system */
#define FEATURE_PROTOCOL        (1)
/** Enable XMODEM protocol */
#define FEATURE_PROTOCOL_XMODEM (1)

/*
 * DEBUG
 */

/** Enable debug prints from config code. */
#define DEBUG_CONFIG          (0)
/** Enable debug prints from BIOS storage code. */
#define DEBUG_STORAGE_BIOS    (0)
/** Enable debug prints from FAT filesystem code. */
#define DEBUG_FS_FAT          (0)
/** Enable debug prints from execution code. */
#define DEBUG_EXEC            (0)
/** Enable debug prints from multiboot code. */
#define DEBUG_EXEC_MULTIBOOT  (0)
/** Enable debug prints from ELF execution code. */
#define DEBUG_EXEC_ELF        (0)
/** Enable debug prints for XMODEM protocol */
#define DEBUG_PROTOCOL_XMODEM (0)

/*
 * SERIAL
 */
/** Whether to enable SW FIFO for serial, and what size to use (as 2^n). */
#define SERIAL_FIFO_SIZE   (     8)
/** Baud rate to use for serial communication.
 *  @todo Allow dynamic configuration */
#define SERIAL_BAUDRATE    (115200)


/*
 * ASSERT
 */
#if (USE_SERIAL && DEBUG_PROTOCOL_XMODEM)
/* Technically this would be fine if COM1 is used for debug, and COM2 was used
 * for XMODEM, but this isn't terribly likely. */
#  error "Cannot enable both USE_SERIAL and DEBUG_PROTOCOL_XMODEM"
#endif

#if (FEATURE_PROTOCOL_XMODEM && !FEATURE_PROTOCOL)
#  error "FEATURE_PROTOCOL_XMODEM requires FEATURE_PROTOCOL"
#endif

#endif

