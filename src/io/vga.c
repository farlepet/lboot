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
    .res_y    = 25,
    .pos_x    = 0,
    .pos_y    = 0
};


int vga_init(void) {
    vga_clear();

    return 0;
}

void vga_clear(void) {
    memset(_vga_state.vidmem, 0, ((_vga_state.res_x * 2) * _vga_state.res_y));
}

#define VCHAR(ch, fg, bg) ((uint16_t)(ch) | ((uint16_t)(fg) << 8) | ((uint16_t)(bg) << 12))

static void _vga_place_char(uint8_t x, uint8_t y, char ch) {
    _vga_state.vidmem[(y * _vga_state.res_x) + x] = VCHAR(ch, _vga_state.color_fg, _vga_state.color_bg);
}

static void _vga_scroll(void) {
    memmove(&_vga_state.vidmem[_vga_state.res_x], &_vga_state.vidmem[0], (_vga_state.res_x * (_vga_state.res_y - 1)) * 2);
}

void vga_putchar(char ch) {
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

void vga_puts(const char *str) {
    while(*str) {
        vga_putchar(*str);
        str++;
    }
}


