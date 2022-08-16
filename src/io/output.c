#include <stddef.h>

#include "io/output.h"

static output_hand_t *_current_output = NULL;

void output_set(output_hand_t *output) {
    _current_output = output;
}

void putchar(char ch) {
    if(_current_output == NULL) {
        return;
    }

    _current_output->putchar(_current_output, ch);
}

void puts(const char *str) {
    if(_current_output == NULL) {
        return;
    }

    while(*str) {
        _current_output->putchar(_current_output, *str);
        str++;
    }
}

