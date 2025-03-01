#ifndef LBOOT_STORAGE_PROTOCOL_XMODEM_H
#define LBOOT_STORAGE_PROTOCOL_XMODEM_H

#ifdef CONFIG_PROTOCOL_XMODEM

#include "storage/protocol/protocol.h"

/**
 * @brief Initialize XMODEM transfer protocol
 *
 * @param proto Protocol handle
 * @param uri URI including serial port to use
 * @return 0 on success, else < 0
 */
int protocol_xmodem_init(protocol_hand_t *proto, const char *uri);

typedef struct xmodem_packet_header_struct {
    uint8_t prefix;    /**< Header prefix */
    uint8_t block;     /**< Block number, 1-255 */
    uint8_t inv_block; /**< Inverse block number (255 - block) */
} xmodem_packet_head_t;

#define XMODEM_CHAR_HEAD    ('\x01') /**< Packet start for standard XMODEM */
#define XMODEM_CHAR_HEAD_1K ('\x02') /**< Packet start for XMODEM-1K */

#define XMODEM_CHAR_EOT     ('\x04') /**< End of Transmission - Used at end of file transfer */
#define XMODEM_CHAR_ACK     ('\x06') /**< Acknowledge - Used to notify sender that packet was recieved good */
#define XMODEM_CHAR_NAK     ('\x15') /**< Negative Acknowledge - Used to notify sender that packet was not received, or corrupt, or to start a trasnfer */
#define XMODEM_CHAR_SUB     ('\x1a') /**< Substitute - Used to pad transfers not a multiple of 128 bytes */

#endif /* (FEATURE_PROTOCOL_XMODEM) */

#endif

