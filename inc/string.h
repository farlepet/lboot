#ifndef LBOOT_STRING_H
#define LBOOT_STRING_H

#include <stdint.h>

/**
 * Calculates the length of a string.
 * 
 * @param str the string to calculate the length of
 * @return the length of the string
 */
size_t strlen(const char *str);

/**
 * Checks to see if two strings are identical
 *
 * @param str1 first string
 * @param str2 second string
 * @return 0 on equal, else str1-str2 of the offending byte pair
 */
int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t num);

/**
 * Checks to see if two strings are identical, case-insensitive
 *
 * @param str1 first string
 * @param str2 second string
 * @return 0 on equal, else str1-str2 of the offending byte pair
 */
int strcasecmp(const char *str1, const char *str2);

/**
 * Checks to see if two strings are identical, case-insensitive with length
 *
 * @param str1 first string
 * @param str2 second string
 * @param num Number of characters to check
 * @return 0 on equal, else str1-str2 of the offending byte pair
 */
int strncasecmp(const char *str1, const char *str2, size_t num);

char *strchr(const char *s, int c);

char *strcpy(char *dest, const char *src);

char *strncpy(char *dest, const char *src, size_t n);

/**
 * @brief Duplicate string in allocated memory
 *
 * @param str String to duplicate
 * @return Pointer to new string.
 */
char *strdup(const char *str);

void *memcpy(void *dest, const void *src, size_t n);

/**
 * @brief Compare data in two memory regions
 *
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to check
 * @return 0 on equal, else s2-s1 of the offending byte pair
 */
int memcmp(const void *s1, const void *s2, size_t n);

void *memset(void *s, int c, size_t n);

void *memmove(void *dst, const void *src, size_t n);

#endif

