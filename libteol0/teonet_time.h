/**
 * @file teonet_time.h
 * @brief Utility functions for working with time.
 */

#ifndef TEONET_TIME_H
#define TEONET_TIME_H

#include <stdint.h>

/**
 * Get current time.
 *
 * @return current time in milliseconds since Unix Epoch.
 *
 * @note On 32-bit linux systems return value is limited to maximum value of 32-bit signed integer.
 */

int64_t teotimeGetCurrentTime();

/**
 * Get time between saved moment of time and current time.
 *
 * @param time_value Saved moment of time.
 *
 * @return Time in milliseconds between time_value and current time.
 *
 * @note Return value can be negative if time_value is in the future.
 */
int64_t teotimeGetTimePassed(int64_t time_value);

#endif
