#include <stddef.h>
#include <string.h>

#include "io/output.h"
#include "mm/alloc.h"
#include "storage/protocol/protocol.h"
#include "storage/protocol/xmodem.h"

#define _check_scheme(SCHEME, URI, COLON) (((size_t)((COLON) - (URI)) == strlen(SCHEME)) && \
                                           !strncasecmp(SCHEME, URI, strlen(SCHEME)))

int protocol_init(protocol_hand_t *proto, const char *uri) {
    const char *colon = strchr(uri, ':');
    if(colon == NULL) {
        /* Not a valid URI */
        return -1;
    }

    memset(proto, 0, sizeof(*proto));

    if(0) {
#ifdef CONFIG_PROTOCOL_KERMIT
    } else if(_check_scheme(PROTOCOL_URISCHEME_KERMIT, uri, colon)) {
        proto->type = PROTOCOLTYPE_KERMIT;
#endif
#ifdef CONFIG_PROTOCOL_XMODEM
    } else if(_check_scheme(PROTOCOL_URISCHEME_XMODEM, uri, colon)) {
        proto->type = PROTOCOLTYPE_XMODEM;
    } else if(_check_scheme(PROTOCOL_URISCHEME_YMODEM, uri, colon)) {
        proto->type = PROTOCOLTYPE_YMODEM;
#endif
    }

    switch(proto->type) {
#ifdef CONFIG_PROTOCOL_XMODEM
        case PROTOCOLTYPE_XMODEM:
            if(protocol_xmodem_init(proto, uri)) {
                return -1;
            }
            break;
#endif
        default:
            printf("Unsupported scheme: `%s`\n", uri);
            return -1;
    }

    return 0;
}

static ssize_t _protocol_file_read(const file_hand_t *file, void *buf, size_t sz, off_t off) {
    if((sz + off) > file->size) {
        return -1;
    }

    protocol_filedata_t *filedata = file->data;

    memcpy(buf, filedata->buff + off, sz);

    return sz;
}

static int _protocol_file_close(file_hand_t *file) {
    protocol_filedata_t *filedata = file->data;

    free(filedata->buff);
    free(filedata);

    return 0;
}

/* @todo Allocate based on size currently available to allocate */
#define PREALLOCATION_SIZE (256 * 1024) /** Max file size of 256 KiB */

int protocol_file_init(protocol_hand_t *proto, file_hand_t *file, size_t buff_sz) {
    /* Currently unused */
    (void)proto;

    memset(file, 0, sizeof(*file));

    if(buff_sz == 0) {
        buff_sz = PREALLOCATION_SIZE;
    } else if(buff_sz > PREALLOCATION_SIZE) {
        /* @todo Allow larger files, perhaps by adding an allocation region at the
         * end of usable memory. Though notably this may limit where kernel can be
         * placed without relocation. */
        return -1;
    }

    protocol_filedata_t *filedata = alloc(sizeof(protocol_filedata_t), 0);
    memset(filedata, 0, sizeof(*filedata));
    file->data = filedata;

    filedata->buff_sz = buff_sz;
    filedata->buff    = alloc(filedata->buff_sz, 0);

    file->read  = _protocol_file_read;
    file->close = _protocol_file_close;

    return 0;
}

void protocol_close(protocol_hand_t *proto) {
    /* Currently nothing to do */
    (void)proto;
}

