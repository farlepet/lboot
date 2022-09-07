#include <stddef.h>
#include <string.h>

#include "io/output.h"
#include "storage/protocol/protocol.h"

#define _check_scheme(SCHEME, URI, COLON) (((size_t)((COLON) - (URI)) == strlen(SCHEME)) && \
                                           !strncasecmp(SCHEME, URI, strlen(SCHEME)))

int protocol_init(protocol_hand_t *proto, const char *uri) {
    const char *colon = strchr(uri, ':');
    if(colon == NULL) {
        /* Not a valid URI */
        return -1;
    }

    memset(proto, 0, sizeof(*proto));

    if(_check_scheme(PROTOCOL_URISCHEME_KERMIT, uri, colon)) {
        printf("Using Kermit\n");
        /* @todo */
    } else {
        printf("Unsupported scheme in URI: `%s`\n", uri);
        return -1;
    }

    return 0;
}

void protocol_close(protocol_hand_t *proto) {
    /* Currently nothing to do */
    (void)proto;
}

