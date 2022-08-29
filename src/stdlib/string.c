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

char *strchr(const char *s, int c) {
	while(*s) {
		if(*s == (char)c) return (char *)s;
		s++;
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

