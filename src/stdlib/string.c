#include <string.h>
#include <stddef.h>

#include "mm/alloc.h"

size_t strlen(const char *str) {
	size_t i = 0;
	while(str[i]) { i++; }
	return i;
}

int strcmp(const char *str1, const char *str2) {
	while(*str1 && (*str1 == *str2)) {
		str1++;
		str2++;
	}
	return (int)(*str1 - *str2);
}

int strncmp(const char *str1, const char *str2, size_t num) {
	while(*str1 && (*str1 == *str2) && --num) {
		str1++;
		str2++;
	}
	return (int)(*str1 - *str2);
}

#define _tolower(C) ((((C) >= 'A') && ((C) <= 'Z')) ? ((C) + ('a' - 'A')) : (C))

int strcasecmp(const char *str1, const char *str2) {
    char c1 = _tolower(*str1);
    char c2 = _tolower(*str2);

    while(c1 && (c1 == c2)) {
        str1++;
        str2++;

        c1 = _tolower(*str1);
        c2 = _tolower(*str2);
    }

    return (int)(c1 - c2);
}

int strncasecmp(const char *str1, const char *str2, size_t num) {
    char c1 = _tolower(*str1);
    char c2 = _tolower(*str2);

    while(c1 && (c1 == c2) && --num) {
        str1++;
        str2++;

        c1 = _tolower(*str1);
        c2 = _tolower(*str2);
    }

    return (int)(c1 - c2);
}

char *strchr(const char *s, int c) {
	while(*s) {
		if(*s == (char)c) return (char *)s;
		s++;
	}
	return NULL;
}

char *strstr(const char *haystack, const char *needle) {
    size_t haystack_sz = strlen(haystack);
    size_t needle_sz   = strlen(needle);

    if(haystack_sz < needle_sz) {
        return NULL;
    }

    for(size_t i = 0; i <= (haystack_sz - needle_sz); i++) {
        size_t j = 0;
        for(; j < needle_sz; j++) {
            if(haystack[i + j] != needle[j]) {
                break;
            }
        }

        if(j == needle_sz) {
            return (char *)&haystack[i];
        }
    }

    return NULL;
}

char *strcpy(char *dest, const char *src) {
	size_t i = 0;
	while(src[i]) {
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	
	return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
	size_t i = 0;
	while((i < n) && src[i]) {
		dest[i] = src[i];
		i++;
	}
	while(i < n) {
		dest[i++] = '\0';
	}
	
	return dest;
}

char *strdup(const char *str) {
    char *dup = alloc(strlen(str)+1, 0);
    /* @note Failed alloc causes panic, so no need to check for NULL. */
    strcpy(dup, str);
    return dup;
}

void *memcpy(void *dest, const void *src, size_t n) {
	uint8_t *dp = dest;
	const uint8_t *sp = src;
	while (n--) {
		*dp++ = *sp++;
	}
	return dest;
}


void *memset(void *s, int c, uint32_t n) {
	uint8_t *p = s;
	while(n--) {
		*p++ = (uint8_t)c;
	}
	return s;
}


void *memmove(void *dst, const void *src, size_t n) {
	const uint8_t *_src = src;
	uint8_t       *_dst = dst;

	if(dst < src) {
		while(n--) {
			*_dst++ = *_src++;
		}
	} else {
		_dst += n;
		_src += n;

		while(n--) {
			*--_dst = *--_src;
		}
	}

	return dst;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *b1 = s1;
    const uint8_t *b2 = s2;

    for(unsigned i = 0; i < n; i++) {
        if(b1[i] != b2[i]) {
            return b2[i] - b1[i];
        }
    }

    return 0;
}

