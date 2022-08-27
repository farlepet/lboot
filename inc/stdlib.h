#ifndef LBOOT_STDLIB_H
#define LBOOT_STDLIB_H

/**
 * @brief Converts a string into an unsigned long
 *
 * @param nptr String to convert
 * @param endptr Optional pointer in which to store end of number
 * @param base Base to convert number in
 * @return Converted number
 */
unsigned long strtoul(const char *restrict nptr, char **restrict endptr, int base);

#endif

