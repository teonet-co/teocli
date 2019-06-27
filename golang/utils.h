/*
 * File:   utils.h
 * Author: Kirill Scherba
 *
 * Created on April 10, 2015, 6:13 PM
 */

#ifndef UTILS_H
#define	UTILS_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

char *ksnet_formatMessage(const char *fmt, ...);
char *ksnet_sformatMessage(char *str_to_free, const char *fmt, ...);
//char *ksnet_vformatMessage(const char *fmt, va_list ap);
void *memdup(const void* d, size_t s);
char *trim(char *str);
char *trimlf(char *str);
int calculate_lines(char *str);

#ifdef	__cplusplus
}
#endif

#endif	/* UTILS_H */
