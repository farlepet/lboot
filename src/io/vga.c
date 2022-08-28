#include <string.h>

#include "io/vga.h"

static struct {
    uint16_t *vidmem;

    uint8_t color_fg;
    uint8_t color_bg;

    uint8_t res_x;
    uint8_t res_y;

    uint8_t pos_x;
    uint8_t pos_y;
} _vga_state = {
    .vidmem   = (uint16_t *)0xB8000,
    .color_fg = 0x07,
    .color_bg = 0x00,
    .res_x    = 80,
#if (FEATURE_STATUSBAR)
    .res_y    = 24,
#else
    .res_y    = 25,
#endif
    .pos_x    = 0,
    .pos_y    = 0
};

static void _vga_clear(void);
static void _vga_putchar(output_hand_t *out, char ch);
#if (FEATURE_STATUSBAR)
static void _vga_status(output_hand_t *out, const char *str);
#  if (FEATURE_WORKINGSTATUS)
static void _vga_working(output_hand_t *out, working_status_e status);
#  endif
#endif

int vga_init(output_hand_t *out) {
    _vga_clear();

    memset(out, 0, sizeof(output_hand_t));
    out->putchar = _vga_putchar;
#if (FEATURE_STATUSBAR)
    out->status  = _vga_status;
#  if (FEATURE_WORKINGSTATUS)
    out->working = _vga_working;
#  endif
#endif

    return 0;
}

static void _vga_clear(void) {
    memset(_vga_state.vidmem, 0, ((_vga_state.res_x * 2) * _vga_state.res_y));
}

#define VCHAR(ch, fg, bg) ((uint16_t)(ch) | ((uint16_t)(fg) << 8) | ((uint16_t)(bg) << 12))

static void _vga_place_char(uint8_t x, uint8_t y, char ch) {
    _vga_state.vidmem[(y * _vga_state.res_x) + x] = VCHAR(ch, _vga_state.color_fg, _vga_state.color_bg);
}

static void _vga_scroll(void) {
    memmove(&_vga_state.vidmem[0], &_vga_state.vidmem[_vga_state.res_x], (_vga_state.res_x * (_vga_state.res_y - 1)) * 2);
    memset(&_vga_state.vidmem[_vga_state.res_x * (_vga_state.res_y - 1)], 0, _vga_state.res_x * 2);
}

static void _vga_putchar(output_hand_t *out, char ch) {
    (void)out;

    switch(ch) {
        case '\n':
            _vga_state.pos_x = 0;
            _vga_state.pos_y++;
            break;
        default:
            _vga_place_char(_vga_state.pos_x, _vga_state.pos_y, ch);
            _vga_state.pos_x++;
            break;
    }

    if(_vga_state.pos_x >= _vga_state.res_x) {
        _vga_state.pos_x = 0;
        _vga_state.pos_y++;
    }
        
    if(_vga_state.pos_y >= _vga_state.res_y) {
        _vga_state.pos_y = _vga_state.res_y-1;
        _vga_scroll();
    }
}

#if (FEATURE_STATUSBAR)
static void _vga_status(output_hand_t *out, const char *str) {
    (void)out;

    int pos = 1;

    uint16_t *bar = &_vga_state.vidmem[_vga_state.res_x * _vga_state.res_y];

    bar[pos++] = VCHAR(' ', 0x0, 0x7);
    while (*str && (pos < _vga_state.res_x)) {
        bar[pos++] = VCHAR(*(str++), 0x0, 0x7);
    }
    while(pos < _vga_state.res_x) {
        bar[pos++] = VCHAR(' ', 0x0, 0x7);
    }
}

#  if (FEATURE_WORKINGSTATUS)
static void _vga_working(output_hand_t *out, working_status_e status) {
    (void)out;

    const char      spinner[]   = { '|', '/', '-', '\\', '-' };
    static unsigned spinner_pos = 0;

    if(spinner_pos >= sizeof(spinner)) {
        spinner_pos = 0;
    }

    uint16_t *icon_data = &_vga_state.vidmem[_vga_state.res_x * _vga_state.res_y];

    switch(status) {
        case WORKING_STATUS_NOTWORKING:
            *icon_data = VCHAR(' ', 0x0, 0x7);
            break;
        case WORKING_STATUS_WORKING:
            *icon_data = VCHAR(spinner[spinner_pos++], 0x0, 0x7);
            break;
        case WORKING_STATUS_ERROR:
            *icon_data = VCHAR('!', 0x4, 0x7);
            break;
    }
}
#  endif
#endif

