#include <ctype.h>
#include <stdlib.h>

static int _char_to_num(char c) {
    if((c >= '0') & (c <= '9')) {
        return c - '0';
    } else if ((c >= 'A') && (c <= 'Z')) {
        return 10 + (c - 'A');
    } else if ((c >= 'a') && (c <= 'z')) {
        return 10 + (c - 'a');
    } else {
        return -1;
    }
}

unsigned long strtoul(const char *restrict nptr, char **restrict endptr, int base) {
    /* @todo Support base=0 */
    unsigned long res = 0;

    while(*nptr) {
        if(!isalnum(*nptr)) {
            break;
        }

        int ch = _char_to_num(*nptr);
        if(ch > base) {
            break;
        }
    
        res = (res * base) + ch;

        nptr++;
    }

    if(endptr) {
        *endptr = (char *)nptr;
    }

    return res;
}

