#ifndef LBOOT_STORAGE_PROTOCOL_PROTOCOL_H
#define LBOOT_STORAGE_PROTOCOL_PROTOCOL_H

typedef struct protocol_hand_struct     protocol_hand_t;
typedef struct protocol_filedata_struct protocol_filedata_t;

#include "storage/file.h"

/**
 * @brief File transfer protocol handle
 */
struct protocol_hand_struct {
    void *data; /**< Pointer to data needed by protocol driver */

    /**
     * @brief Receive file from remote
     *
     * @param proto Protocol handle
     * @param file File handle to populate
     * @param uri URI describing connection and file, if applicabvle.
     * @return 0 on success, < 0 on failure
     */
    int (*recv)(protocol_hand_t *proto, file_hand_t *file, const char *uri);
};

/**
 * @brief Common file data needed by all protocols
 */
struct protocol_filedata_struct {
    void *data;     /**< Pointer to data needed by protocol driver, if applicable */

    void  *buff;    /**< Pointer to buffer containing file data */
    size_t buff_sz; /**< Size of the buffer, may not be the same as file size */
};

#define PROTOCOL_URISCHEME_KERMIT "kermit" /* e.g. kermit://COM1/kernel.elf (filename not currently used) */

/**
 * @brief Initiailizes a protocol driver based on the given URI
 *
 * @param proto Protocol handle to populate
 * @param uri RFC 3986 formatted URI containing protocol information
 * @return 0 on success, < 0 on failure
 */
int protocol_init(protocol_hand_t *proto, const char *uri);

/**
 * @brief Close open protocol, freeing any used memory
 *
 * @param proto Protocol handle to close
 */
void protocol_close(protocol_hand_t *proto);

#endif

