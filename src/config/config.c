#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "config/config.h"
#include "io/output.h"
#include "mm/alloc.h"
#include "storage/fs/fs.h"

static int _config_parse(config_data_t *cfg, char *cfgdata);

#if (DEBUG_CONFIG)
static void _config_print(const config_data_t *cfg);
#endif

int config_load(config_data_t *cfg, fs_hand_t *fs, const char *path) {
    fs_file_t cfgfile;
    if(fs->find(fs, NULL, &cfgfile, path)) {
        return -1;
    }

    char *cfgdata = alloc(cfgfile.size, 0);
    if(fs->read(fs, &cfgfile, cfgdata, cfgfile.size, 0) != (ssize_t)cfgfile.size) {
        return -1;
    }
    
    memset(cfg, 0, sizeof(*cfg));

    if(_config_parse(cfg, cfgdata)) {
        free(cfgdata);
        return -1;
    }

#if (DEBUG_CONFIG)
    _config_print(cfg);
#endif

    free(cfgdata);

    return 0;
}

#if (DEBUG_CONFIG)
static void _config_print(const config_data_t *cfg) {
    printf("-------- Config Data --------\n");
    printf("  config_version: %hhu\n", cfg->config_version);
    printf("     kernel_path: %s\n",   cfg->kernel_path);
    printf("  kernel_cmdline: %s\n",   cfg->kernel_cmdline);
    printf("         modules: %u\n",   cfg->module_count);

    for(unsigned i = 0; i < cfg->module_count; i++) {
        printf("         [%2u] path: %s\n", i, cfg->modules[i].module_path);
        printf( "              name: %s\n",    cfg->modules[i].module_name);
        printf( "              addr: %p\n",    cfg->modules[i].module_addr);
    }

    printf("-----------------------------\n");
}
#endif

static void _get_line(char *str, char **next) {
    char *end_cr = strchr(str, '\r');
    char *end_lf = strchr(str, '\n');
    char *end    = (end_cr && (end_cr < end_lf)) ? end_cr : end_lf;

    if(end) {
        while(*end == '\r' || *end == '\n') {
            *(end++) = '\0';
        }
        
        if(*end) {
            *next = end;
        }
    } else {
        *next = NULL;
    }
}

static int _config_parse(config_data_t *cfg, char *cfgdata) {
    char *next = NULL;
    char *line = cfgdata;

    /* @todo Dynamically allocate based on number of MODULE lines. */
#define MAX_MODULES 4
    cfg->modules = alloc(sizeof(*cfg->modules) * MAX_MODULES, 0);

    while(line) {
        _get_line(line, &next);

        char *eq = strchr(line, '=');
        if(eq) {
            *eq = '\0';

            char  *val     = eq+1;
            size_t val_len = strlen(val);

            if(!strcmp(line, "CFGVER")) {
                cfg->config_version = strtoul(val, NULL, 10);
                if(cfg->config_version != 1) {
                    printf("_config_parse: Unsupported config version: %hhu\n", cfg->config_version);
                    return -1;
                }
            } else if(!strcmp(line, "KERNEL")) {
                cfg->kernel_path = alloc(val_len+1, 0);
                strcpy(cfg->kernel_path, val);
            } else if(!strcmp(line, "CMDLINE")) {
                cfg->kernel_cmdline = alloc(val_len+1, 0);
                strcpy(cfg->kernel_cmdline, val);
            } else if(!strcmp(line, "MODULE")) {
                if(cfg->module_count == MAX_MODULES) {
                    printf("_config_parse: Maximum module count (%u) exceeded!\n", MAX_MODULES);
                    return -1;
                }
                cfg->modules[cfg->module_count].module_path = alloc(val_len + 1, 0);
                strcpy(cfg->modules[cfg->module_count].module_path, val);
                /* @todo Support custom module names. */
                cfg->modules[cfg->module_count].module_name = cfg->modules[cfg->module_count].module_path;

                cfg->module_count++;
            } else {
                printf("_config_parse: Unsupported key: %s\n", line);
            }
        }

        line = next;
    }

    return 0;
}

