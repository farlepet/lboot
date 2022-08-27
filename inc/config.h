#ifndef LBOOT_CONFIG_H
#define LBOOT_CONFIG_H

/** Whether to use serial as main output device. */
#define USE_SERIAL (0)

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

