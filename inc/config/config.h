#ifndef LBOOT_CONFIG_CONFIG_H
#define LBOOT_CONFIG_CONFIG_H

#include "config/config_types.h"

/**
 * @brief Loads config from file and populates config structure.
 *
 * @param cfg Config structure to populate
 * @parma path Path to config file
 * @return 0 on success, else < 0
 */
int config_load(config_data_t *cfg, const char *path);

#endif

