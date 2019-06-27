#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define KSN_BUFFER_SM_SIZE 256

char *ksnet_vformatMessage(const char *fmt, va_list ap) {

    int size = KSN_BUFFER_SM_SIZE; /* Guess we need no more than 100 bytes */
    char *p, *np;
    va_list ap_copy;
    int n;

    if((p = malloc(size)) == NULL)
        return NULL;

    while(1) {

        // Try to print in the allocated space
        va_copy(ap_copy,ap);
        n = vsnprintf(p, size, fmt, ap_copy);
        va_end(ap_copy);

        // Check error code
        if(n < 0)
            return NULL;

        // If that worked, return the string
        if(n < size)
            return p;

        // Else try again with more space
        size = n + KSN_BUFFER_SM_SIZE; // Precisely what is needed
        if((np = realloc(p, size)) == NULL) {
            free(p);
            return NULL;
        }
        else {
            p = np;
        }
    }
}

/**
 * Create formated message in new null terminated string
 *
 * @param fmt Format string like in printf function
 * @param ... Parameter
 *
 * @return Null terminated string, should be free after using or NULL on error
 */
char *ksnet_formatMessage(const char *fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    char *p = ksnet_vformatMessage(fmt, ap);
    va_end(ap);

    return p;
}
/**
 * Create formated message in new null terminated string, and free str_to_free
 *
 * @param str_to_free
 * @param fmt Format string like in printf function
 * @param ... Parameter
 *
 * @return Null terminated string, should be free after using or NULL on error
 */
char *ksnet_sformatMessage(char *str_to_free, const char *fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    char *p = ksnet_vformatMessage(fmt, ap);
    va_end(ap);

    if(str_to_free != NULL) free(str_to_free);

    return p;
}

/**
 * Remove trailing line feeds characters from null
 * terminated string
 *
 * @param str
 * @return
 */
char *trimlf(char *str) {

    // Trim at the end of string
    int len = strlen(str);
    while(len && (str[len - 1] == '\n' || str[len - 1] == '\r'))
        str[(--len)] = '\0';

    return str;
}

/**
 * Remove trailing or leading space
 *
 * Remove trailing or leading space or tabulation characters in
 * input null terminated string
 *
 * @param str Null terminated input string to remove trailing and leading
 *            spaces in
 * @return Pointer to input string (the same as input string)
 */
char *trim(char *str) {

    int i, j = 0, stop = 0;

    // Trim at begin of string
    for(i = 0; str[i] != '\0'; i++) {
        if(stop || (str[i] != ' ' && str[i] != '\t')) {
            str[j++] = str[i];
            stop = 1;
        }
    }
    str[j] = '\0';

    // Trim at the end of string
    int len = strlen(str);
    while(len && (str[len - 1] == ' ' || str[len - 1] == '\t'))
        str[(len--) - 1 ] = '\0';

    return str;
}

/**
 * Duplicate memory \todo move this function to teonet library
 *
 * Allocate memory and copy selected value to it
 *
 * @param d Pointer to value to copy
 * @param s Length of value
 * @return
 */
void *memdup(const void* d, size_t s) {

    void* p;
    return ((p = malloc(s))?memcpy(p, d, s):NULL);
}

/**
 * Calculate number of lines in string
 *
 * Calculate number of line with line feed \\n at the end in null
 * input terminated string
 *
 * @param str Input null terminated string
 * @return Number of line with linefeed \\n at the end
 */
int calculate_lines(char *str) {

    int i, num = 0;
    for(i = 0; str[i] != 0; i++)
        if(str[i] == '\n') num++;

    return num;
}
