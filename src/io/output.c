#include <stddef.h>
#include <string.h>

#include "intr/interrupts.h"
#include "io/output.h"

static output_hand_t *_current_output = NULL;

void output_set(output_hand_t *output) {
    _current_output = output;
}

void putchar(char ch) {
    if(_current_output == NULL) {
        return;
    }

    _current_output->write(_current_output, &ch, 1);
}

void puts(const char *str) {
    if(_current_output == NULL) {
        return;
    }

    _current_output->write(_current_output, str, strlen(str));
}




/**
 * @note This printf implementation is a bit of a mess at the moment, likely
 * more functionality than is really required.
 */

/**
 * @brief Divides two 64-bit integers
 *
 * @param a Dividend
 * @param b Divisor
 * @param rem Remainder
 * @return uint64_t Quotient
 */
static uint64_t udiv64(uint64_t a, uint64_t b, uint64_t *rem) {
    /* This Basic implementation is based off the simple 128-bit division
     * algorithm found here: https://danlark.org/2020/06/14/128-bit-division/ */

    if(a < 0x100000000ULL) {
        /* Do fast 32-bit arithmetic when possible */
        if(rem) {
            *rem = (uint32_t)a % (uint32_t)b;
        }
        return (uint32_t)a / (uint32_t)b;
    }

    if(b > a) {
        *rem = a;
        return 0;
    }

    uint64_t q = 0; /* Quotient */

    /* Get difference in position of most-significant bits between dividend and
     * divisor
     * NOTE: (63 - clz(a)) - (63 - clz(b)) == clz(b) - clz(a) */
    int shift = __builtin_clzll(b | 1) - __builtin_clzll(a | 1);

    b <<= shift;

    while(shift >= 0) {
        q <<= 1;
        if(a >= b) {
            a -= b;
            q |= 1;
        }
        b >>= 1;
        shift--;
    }

    if(rem) {
        *rem = a;
    }
    return q;
}


typedef unsigned long long arg_type_t;
typedef signed long long   sarg_type_t;

#define PRINTFLAG_UNSIGNED  (1UL << 0) /**< Treat value as unsigned */
#define PRINTFLAG_PADZERO   (1UL << 1) /**< Pad value with zeros */
#define PRINTFLAG_POSSIGN   (1UL << 2) /**< Precede positive numbers with + */
#define PRINTFLAG_POSSPACE  (1UL << 3) /**< Precede positive numbers with a space */
#define PRINTFLAG_UPPERCASE (1UL << 4) /**< Use uppercase for bases > 10 */
#define PRINTFLAG_LEFTALIGN (1UL << 5) /**< Left align */

#define FMT_SPEC '%' //!< Format specifier character

#define ZERO_ALL_VID()  \
    do {                \
        is_in_spec = 0; \
        size       = 0; \
        width      = 0; \
        precision  = 0; \
        flags      = 0; \
    } while(0)



/**
 * Converts an interger into a string and places the output into `out`
 * Used by `print` to make the code cleaner.
 * 
 * @param num The number to print
 * @param base The base in which to print the number
 * @param pad How many characters of padding to give the number
 * @param flags Flags controling operation
 * @param out The output string/buffer
 * @return The number of characters that have been put in `out`
 */
static int _print_int(arg_type_t num, uint8_t base, uint8_t pad, uint32_t flags, char *out) {
    int neg = (!(flags & PRINTFLAG_UNSIGNED) && (num & (1ULL << 63)));
    if(neg) num = (~num) + 1;

    const char *nums = (flags & PRINTFLAG_UPPERCASE) ? "0123456789ABCDEF" :
                                                       "0123456789abcdef";

    int outidx = 0;

    /* Maximum characters occupied by base 8 representation of 64-bit number:
     *   64 / log(8) = 23.333 */
    char ans[25] = { '0', 0, };
    int i = 0;
    while(num) {
        arg_type_t rem;
        num = udiv64(num, base, &rem);
        ans[i++] = nums[rem];
    }
    if(i == 0) i++;

    if(neg) {
        out[outidx++] = '-';
    } else {
        if(flags & PRINTFLAG_POSSIGN) {
            out[outidx++] = '+';
        } else if(flags & PRINTFLAG_POSSPACE) {
            out[outidx++] = ' ';
        }
    }

    int p = ((pad - i) > 0) ? (pad - i) : 0;

    while(p--) {
        out[outidx++] = ((flags & PRINTFLAG_PADZERO) ? '0' : ' ');
    }
    while(--i >= 0) {
        out[outidx++] = ans[i];
    }

    return outidx;
}


/**
 * Convert a number in a string into an integer. Used by `print` to make code cleaner.
 * 
 * @param str input string
 * @param out pointer to end of number (generated by this function)
 * @return the number found
 * @see print
 */
static int _get_dec(const char *str, uintptr_t *out) {
    int n = 0;
    while(*str >= '0' && *str <= '9') {
        n *= 10;
        n += (int)(*str - '0');
        str++;
    }
    *out = (uintptr_t)str;
    
    return n;
}

#define va_arg __builtin_va_arg // Helps clean up the code a bit
static arg_type_t _get_arg(__builtin_va_list *varg, int size) {
    arg_type_t arg = 0;

    switch(size) {
        case -2:
            arg = va_arg(*varg, int) & 0xFF;
            break;
        case -1:
            arg = va_arg(*varg, int) & 0xFFFF;
            break;
        case  0:
            arg = va_arg(*varg, int) & 0xFFFFFFFF;
            break;
        case  1:
            arg = va_arg(*varg, long);
            break;
        case  2:
            arg = va_arg(*varg, long long);
            break;
    }

    return arg;
}

static arg_type_t _sign_extend(arg_type_t val, int size) {
    switch(size) {
        case -2: return (arg_type_t)(sarg_type_t)(char)val;
        case -1: return (arg_type_t)(sarg_type_t)(short)val;
        case  0: return (arg_type_t)(sarg_type_t)(int)val;
        case  1: return (arg_type_t)(sarg_type_t)(long)val;
    }
    /* No sign extension if already full width. */
    return val;
}

/**
 * Takes a format string and a list of arguments as input, and produces a
 * string as output.
 * 
 * @param out the output string
 * @param format the format string
 * @param varg the list of arguments
 * @return the number of charactern placed in `out`
 */
static int _print(char *out, const char *format, __builtin_va_list varg) {
    uint8_t  is_in_spec = 0;
    
    int8_t   size      = 0; // Size of the integer
    uint32_t width     = 0; // Width of the number at minimum
    uint32_t precision = 0; // Precision
    int      nchars    = 0; // Number of chars printed so far
    uint32_t flags     = 0;
    
    uintptr_t  temp;
    arg_type_t arg;
    
    for(; *format != 0; format++) {
        if(!is_in_spec) {
            if(*format == FMT_SPEC) {
                is_in_spec = 1;
                continue;
            }
            *out++ = *format;
            nchars++;
            continue;
        }
        
        switch(*format) {
            case FMT_SPEC: is_in_spec = 0;
                           *out++ = FMT_SPEC;
                           nchars++;
                           break;
            
            case 'l': if(size < 2) size++;
                      break;
        
            case 'L': size = 1;
                      break;
            
            case 'h': if(size > -2) size--;
                      break;
            
            case 'z':
            case 'j':
            case 't': size = 0;
                      break;
        
            case '+': flags |= PRINTFLAG_POSSIGN;
                      break;
            
            case ' ': flags |= PRINTFLAG_POSSPACE;
                      break;
            
            case '-': flags |= PRINTFLAG_LEFTALIGN;
                      break;
        
            case '0': flags |= PRINTFLAG_PADZERO;
                      break;
            
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': width = (uint8_t)_get_dec(format, &temp);
                      format = (char *)(temp - 1);
                      break;
            
            case '.': format++;
                      precision = (uint8_t)_get_dec(format, &temp);
                      format = (char *)(temp - 1);
                      break;
            
            
        // Numbers, strings, etc...
            
            case 'u':
            case 'd':
            case 'i': arg = _get_arg(&varg, size);
                      if(*format != 'u') {
                          arg = _sign_extend(arg, size);
                      } else {
                          flags |= PRINTFLAG_UNSIGNED;
                      }
                      temp = (uintptr_t)_print_int(arg, 10, width, flags, out);
                      nchars += temp;
                      out += temp;
                      ZERO_ALL_VID();
                      break;

            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G': /* TODO: Floating point */
                      (void)va_arg(varg, double);
                      ZERO_ALL_VID();
                      break;
                      
            case 'X':
            case 'x': flags |= PRINTFLAG_UNSIGNED;
                      if(*format == 'X') {
                          flags |= PRINTFLAG_UPPERCASE;
                      }
                      arg  = _get_arg(&varg, size);
                      temp = (uintptr_t)_print_int(arg, 16, width, flags, out);
                      nchars += temp;
                      out    += temp;
                      ZERO_ALL_VID();
                      break;
            
            case 'o': flags |= PRINTFLAG_UNSIGNED;
                      arg = _get_arg(&varg, size);
                      temp = (uintptr_t)_print_int(arg, 8, width, flags, out);
                      nchars += temp;
                      out += temp;
                      ZERO_ALL_VID();
                      break;
                     
            case 's': if(!precision) precision = UINT_MAX;
                      temp = (uintptr_t)va_arg(varg, char *);
                      if(temp == 0) { temp = (uintptr_t)"(null)"; }
                      nchars += strlen((char *)temp);
                      while(*(char *)temp && precision--) *out++ = *(char *)temp++;
                      ZERO_ALL_VID();
                      break;
                    
            case 'c': arg = (arg_type_t)va_arg(varg, int);
                      *out++ = (char)arg;
                      nchars++;
                      ZERO_ALL_VID();
                      break;
                    
            case 'p': flags |= PRINTFLAG_UNSIGNED;
                      if(!width) width = sizeof(void *) * 2;
                      arg = _get_arg(&varg, size);
                      temp = (uintptr_t)_print_int(arg, 16, width, flags, out);
                      nchars += temp;
                      out += temp;
                      ZERO_ALL_VID();
                      break;
                    
            case 'a':
            case 'A': (void)va_arg(varg, double);
                      ZERO_ALL_VID();
                      break;
                    
            case 'n': (void)va_arg(varg, int);
                      ZERO_ALL_VID();
                      break;
        }
        
    }
    
    *(out) = 0;
    
    return nchars;
}

/* Not using memory allocation here, as we may need to print information after
 * when memory allocation has failed us. */
static char _printf_buff[1024];

int printf(const char *fmt, ...) {
    __builtin_va_list varg;
    __builtin_va_start(varg, fmt);

    int ret = _print(_printf_buff, fmt, varg);

    __builtin_va_end(varg);

    puts(_printf_buff);

    return ret;
}

void _panic(const char *fmt, ...) {
    interrupts_disable();

    __builtin_va_list varg;
    __builtin_va_start(varg, fmt);

    _print(_printf_buff, fmt, varg);

    puts(_printf_buff);

    __builtin_va_end(varg);

    status_working(WORKING_STATUS_ERROR);

    /* @todo Stack trace */
    for(;;) {
        asm volatile("hlt");
    }
}

#define PRINTHEX_ROWSZ (24) /**< Bytes to print per row - chosen to work well on 80-column displays. */

void print_hex(const void *data, size_t len) {
    const uint8_t *bytes = (const uint8_t *)data;

    for(size_t i = 0; i < len; i += PRINTHEX_ROWSZ) {
        for(size_t j = 0; (j < PRINTHEX_ROWSZ) && ((j + i) < len); j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\n");
    }
}

void print_status(const char *fmt, ...) {
    __builtin_va_list varg;
    __builtin_va_start(varg, fmt);

    _print(_printf_buff, fmt, varg);

    __builtin_va_end(varg);

#ifdef CONFIG_STATUSBAR
    if(_current_output && _current_output->status) {
        _current_output->status(_current_output, _printf_buff);
    } else {
#endif
        puts(_printf_buff);
        putchar('\n');
#ifdef CONFIG_STATUSBAR
    }
#endif
}

#ifdef CONFIG_WORKINGSTATUS
void status_working(working_status_e status) {
#  ifdef CONFIG_STATUSBAR
    if(_current_output && _current_output->working) {
        _current_output->working(_current_output, status);
    } else {
#  endif
        if(status == WORKING_STATUS_WORKING) {
            putchar('.');
        }
#  ifdef CONFIG_STATUSBAR
    }
#  endif
}
#endif

