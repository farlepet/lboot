#ifndef LBOOT_IO_OUTPUT_H
#define LBOOT_IO_OUTPUT_H

#include <stdint.h>

#ifdef CONFIG_WORKINGSTATUS
typedef enum working_status_e {
    WORKING_STATUS_NOTWORKING = 0, /**< Not actively working */
    WORKING_STATUS_WORKING,        /**< Actively working */
    WORKING_STATUS_ERROR           /**< Error occured */
} working_status_e;
#endif

typedef struct output_hand_struct output_hand_t;

/**
 * @brief Output device handle
 */
struct output_hand_struct {
    void *data; /**< Pointer to output-specific data */

    /**
     * @brief Write a buffer to the output device
     *
     * @param out Output handle
     * @param data Data to send
     * @param sz Number of bytes
     * @return < 0 on error, number of bytes written on success
     */
    ssize_t (*write)(output_hand_t *out, const void *data, size_t sz);

#ifdef CONFIG_STATUSBAR
    /**
     * @brief Update status bar message
     *
     * @param out Output handle
     * @param str Status string
     */
    void (*status)(output_hand_t *out, const char *str);

#  ifdef CONFIG_WORKINGSTATUS
    /**
     * @brief Update status bar working icon
     *
     * @parma out Output handle
     * @param status Working status
     */
    void (*working)(output_hand_t *out, working_status_e status);
#  endif
#endif
};

/**
 * @brief Set the currently selected output driver
 *
 * @param output Output driver to select
 */
void output_set(output_hand_t *output);

/**
 * @brief Sends a single character to the current output device
 *
 * @param ch Character to send
 */
void putchar(char ch);

/**
 * @brief Sends a (NULL-terminated) string of characters to the current
 * output device
 *
 * @param str NULL-terminated string
 */
void puts(const char *str);

/**
 * @brief Send formatted output to the current output device
 *
 * @param fmt Format string
 * @return int Number of characters sent
 */
int printf(const char *fmt, ...);

/**
 * @brief Send message and halt execution
 *
 * @param fmt Format string
 */
void _panic(const char *fmt, ...);

#ifdef CONFIG_VERBOSE_PANIC
#  define __panic_stringify1(X) #X
#  define __panic_stringify2(X) __panic_stringify1(X)

#  define panic(...) _panic("\nPANIC at " __FILE__ ":" __panic_stringify2(__LINE__) ":\n  " __VA_ARGS__)
#else
#  define panic _panic
#endif

/**
 * @brief Print data has hex
 *
 * @param data Data to print
 * @param len Length of data in bytes
 */
void print_hex(const void *data, size_t len);

/**
 * @brief Print a status message
 *
 * If FEATURE_STATUSBAR is enabled, this will update the statusbar message.
 * Otherwise, this will behave the same as printf() with a newline appended.
 *
 * @param fmt Format string
 */
void print_status(const char *fmt, ...);

#ifdef CONFIG_WORKINGSTATUS
/**
 * @brief Update status showing that work is in progress
 *
 * @param status Current working status
 */
void status_working(working_status_e status);
#else
#  define status_working(...)
#endif

#endif

