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
#define FEATURE_VERBOSE_PANIC (1)
/** Enable status=bar showing current operations. Only available via VGA. */
#define FEATURE_STATUSBAR     (1)
/** Show working status when nothing else is being displayed. */
#define FEATURE_WORKINGSTATUS (1)

/*
 * DEBUG
 */

/** Enable debug prints from config code. */
#define DEBUG_CONFIG         (0)
/** Enable debug prints from BIOS storage code. */
#define DEBUG_STORAGE_BIOS   (0)
/** Enable debug prints from FAT filesystem code. */
#define DEBUG_FS_FAT         (0)
/** Enable debug prints from execution code. */
#define DEBUG_EXEC           (0)
/** Enable debug prints from multiboot code. */
#define DEBUG_EXEC_MULTIBOOT (0)
/** Enable debug prints from ELF execution code. */
#define DEBUG_EXEC_ELF       (0)


#endif

