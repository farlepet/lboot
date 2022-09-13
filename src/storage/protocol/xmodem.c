#if (FEATURE_PROTOCOL_XMODEM)

#include <stddef.h>
#include <string.h>

#include "intr/interrupts.h"
#include "io/output.h"
#include "io/serial.h"
#include "mm/alloc.h"
#include "storage/protocol/xmodem.h"

#if (DEBUG_PROTOCOL_XMODEM)
#  define DEBUG_PRINT(...) printf("xmodem: "__VA_ARGS__)
#else
#  define DEBUG_PRINT(...)
#endif

/**
 * @brief Structure to keep track of XMODEM config and state
 */
typedef struct xmodem_parameters_struct {
    uint16_t block_sz;    /**< Block size, currently 128 and 1024 supported */
    uint8_t  chksum_type; /**< Which checksum method to use */
#define XMODEM_CHKSUMTYPE_SUM   (1) /**< Simple 8-bit sum of data */
#define XMODEM_CHKSUMTYPE_CRC16 (2) /**< 16-bit CRC */
    char     start_ch;    /**< Character to send to initiate transfer */
#define XMODEM_STARTCH_XMODEM ('\x15')
#define XMODEM_STARTCH_YMODEM (   'C')
    uint8_t  last_block;  /**< Block ID of the previously received block */
    size_t   total_sz;    /**< Total size, unknown until the end with standard XMODEM */
    uint32_t timeout;     /**< Data reception timeout */
    off_t    curr_off;    /**< Current offset within file */
} xmodem_params_t;

static int _xmodem_recv(protocol_hand_t *proto, file_hand_t *file, const char *uri);

int protocol_xmodem_init(protocol_hand_t *proto, const char *uri) {
    const char *sep = strstr(uri, "//");
    if(sep == NULL) {
        printf("xmodem: URI `%s` missing `//`\n", uri);
        return -1;
    }

    uint8_t port;

    /* @todo Move this, as it will also be used by Kermit */
    const char *port_str = sep + 2;
    if(!strncmp(port_str, "COM1", 4)) {
        port = SERIAL_CFG_PORT_COM1;
    } else if(!strncmp(port_str, "COM2", 4)) {
        port = SERIAL_CFG_PORT_COM2;
    } else if(!strncmp(port_str, "COM3", 4)) {
        port = SERIAL_CFG_PORT_COM3;
    } else if(!strncmp(port_str, "COM4", 4)) {
        port = SERIAL_CFG_PORT_COM4;
    } else {
        printf("xmodem: Bad serial port name `%s`\n", port_str);
        return -1;
    }

    serial_init(&proto->in, &proto->out, SERIAL_BAUDRATE, 
                ((port             << SERIAL_CFG_PORT__POS)      |
                 (SERIAL_FIFO_SIZE << SERIAL_CFG_INBUFFSZ__POS)  |
                 (SERIAL_FIFO_SIZE << SERIAL_CFG_OUTBUFFSZ__POS) |
                 (1UL                  << SERIAL_CFG_FLOWCTRL_RTS__POS)));

    proto->recv = _xmodem_recv;

    return 0;
}

#define XMODEM_RET_OK    ( 0)
#define XMODEM_RET_ERROR (-1)
#define XMODEM_RET_FATAL (-2)
#define XMODEM_RET_EOT   ( 1)

static int _xmodem_checksum(protocol_hand_t *proto, xmodem_params_t *params, void *data) {
    uint8_t *bdata = data;

    switch(params->chksum_type) {
        case XMODEM_CHKSUMTYPE_SUM: {
            uint8_t checksum;
            if(proto->in.read(&proto->in, &checksum, 1, params->timeout) != 1) {
                return XMODEM_RET_ERROR;
            }

            for(uint16_t i = 0; i < params->block_sz; i++) {
                checksum -= bdata[i];
            }

            if(checksum != 0) {
                return -1;
            }
        } break;
        case XMODEM_CHKSUMTYPE_CRC16: {
            uint16_t checksum;
            if(proto->in.read(&proto->in, &checksum, 2, params->timeout) != 2) {
                return XMODEM_RET_ERROR;
            }
            /* @todo Check */
        } break;
        default:
            DEBUG_PRINT("Bad checksum type: %hhu\n", params->chksum_type);
            return -1;
    }

    return 0;
}

static int _xmodem_rx_packet(protocol_hand_t *proto, protocol_filedata_t *fdata, xmodem_params_t *params) {
    xmodem_packet_head_t pkt;

    /* @todo Only read one byte first, to more quickly catch EOT */
    ssize_t ret = proto->in.read(&proto->in, &pkt, sizeof(pkt), params->timeout);
    if((ret >= 1) &&
       (pkt.prefix == XMODEM_CHAR_EOT)) {
        DEBUG_PRINT("EOT\n");
        return XMODEM_RET_EOT;
    } else if(ret != sizeof(pkt)) {
        if(ret != 0) {
            DEBUG_PRINT("Bad packet size: %d\n", ret);
        }
        return XMODEM_RET_ERROR;
    }

    switch(pkt.prefix) {
        case XMODEM_CHAR_HEAD:
            params->block_sz = 128;
            break;
        case XMODEM_CHAR_HEAD_1K:
            params->block_sz = 1024;
            break;
        default:
            DEBUG_PRINT("Bad packet prefix: %hhu\n", pkt.prefix);
            return XMODEM_RET_ERROR;
    }

    if(pkt.block != (255 - pkt.inv_block)) {
        /* Corrupt? */
        DEBUG_PRINT("Corrupt block?\n");
        return XMODEM_RET_ERROR;
    }

    /* @todo Add support for, or at least read and ignore, YMODEM NULL block */

    uint8_t next_block = params->last_block + 1;

    /* Allow re-transmitted blocks */
    if((pkt.block != params->last_block) &&
       (pkt.block != next_block)) {
        /* Unexpected block number */
        DEBUG_PRINT("Unexpected block number %hhu, expected %hhu or %hhu\n", pkt.block, params->last_block, next_block);
        return XMODEM_RET_ERROR;
    }

    if(params->curr_off &&
       (pkt.block == params->last_block)) {
        /* Simplify logic by simply re-reading block */
        params->curr_off -= params->block_sz;
    }

    if((ret = proto->in.read(&proto->in, fdata->buff + params->curr_off, params->block_sz, params->timeout)) != params->block_sz) {
        DEBUG_PRINT("Failed to read %u bytes, only read %d\n", params->block_sz, ret);
        return XMODEM_RET_ERROR;
    }

    if(_xmodem_checksum(proto, params, fdata->buff + params->curr_off)) {
        DEBUG_PRINT("Checksum failure\n");
        return -1;
    }


    params->curr_off  += params->block_sz;
    params->last_block = pkt.block;

    return XMODEM_RET_OK;
}

static int _xmodem_ack(protocol_hand_t *proto, xmodem_params_t *params) {
    (void)params;
    uint8_t ack = XMODEM_CHAR_ACK;
    if(proto->out.write(&proto->out, &ack, 1) != 1) {
        return -1;
    }
    return 0;
}

static int _xmodem_nak(protocol_hand_t *proto, xmodem_params_t *params) {
    (void)params;
    uint8_t nak = XMODEM_CHAR_NAK;
    if(proto->out.write(&proto->out, &nak, 1) != 1) {
        return -1;
    }
    return 0;
}

static int _xmodem_start(protocol_hand_t *proto, xmodem_params_t *params) {
    if(proto->out.write(&proto->out, &params->start_ch, 1) != 1) {
        return -1;
    }
    return 0;
}

static int _xmodem_rx(protocol_hand_t *proto, protocol_filedata_t *fdata, xmodem_params_t *params) {
    /* Hack to force output flush of using serial.
     * @todo Add flush function for output. */
    interrupts_disable();
    printf("XMODEM start\n");
    interrupts_enable();

    /* @todo When start byte != NAK, allow for multiple attempts */
    if(_xmodem_start(proto, params)) {
        return -1;
    }

    int ret = 0;
    while((ret != XMODEM_RET_EOT) &&
          (ret != XMODEM_RET_FATAL)) {
        ret = _xmodem_rx_packet(proto, fdata, params);

        /* Read any garbage still present */
        if(ret) {
            proto->in.read(&proto->in, NULL, params->block_sz * 4, 25);
        }

        /* @todo Set maximum retry limit */

        switch(ret) {
        case XMODEM_RET_OK:
        case XMODEM_RET_EOT:
            status_working(WORKING_STATUS_WORKING);
            _xmodem_ack(proto, params);
            break;
        case XMODEM_RET_ERROR:
            status_working(WORKING_STATUS_ERROR);
            _xmodem_nak(proto, params);
            break;
        }
    }

    if(ret == XMODEM_RET_FATAL) {
        return -1;
    }

    params->total_sz = (size_t)params->curr_off;

    return 0;
}

static int _xmodem_recv(protocol_hand_t *proto, file_hand_t *file, const char *uri) {
    (void)uri;

    xmodem_params_t params = { 0, };
    /* Get defaults for the chosen protocol */
    switch(proto->type) {
        case PROTOCOLTYPE_XMODEM:
            params.chksum_type = XMODEM_CHKSUMTYPE_SUM;
            params.start_ch    = XMODEM_STARTCH_XMODEM;
            break;
        case PROTOCOLTYPE_YMODEM:
            params.chksum_type = XMODEM_CHKSUMTYPE_CRC16;
            params.start_ch    = XMODEM_STARTCH_YMODEM;
            break;
        default:
            return -1;
    }

    params.timeout = 5000;

    /* @todo If using YMODEM, wait to see if first packet is a NULL packet
     * containing file metadata */
    if(protocol_file_init(proto, file, 0)) {
        return -1;
    }

    protocol_filedata_t *filedata = file->data;

    if(_xmodem_rx(proto, filedata, &params)) {
        return -1;
    }

    file->size = params.total_sz;

    return 0;
}

#endif /* (FEATURE_PROTOCOL_XMODEM) */

