#ifndef LBOOT_CONFIG_CONFIG_TYPES_H
#define LBOOT_CONFIG_CONFIG_TYPES_H

#include <stdint.h>

/**
 * @brief Represents a single module to be loaded.
 */
typedef struct config_data_module_struct {
    char     *module_path; /**< Path to module file. */
    char     *module_name; /**< Name of module passed to kernel, may point to module_path. */
    uintptr_t module_addr; /**< Physical address to which module is loaded. Will be set when the module is actually loaded. */
} config_data_module_t;

/**
 * @brief Representation of data read in from config file.
 */
typedef struct config_data_struct {
    uint8_t               config_version; /**< Config version */
    char                 *kernel_path;    /**< Path to kernel file. */
    char                 *kernel_cmdline; /**< Commandline to pass to kernel. */

    unsigned              module_count;   /**< Number of modules to be loaded. */
    config_data_module_t *modules;        /**< Pointer to array of module data. */
} config_data_t;

#endif

