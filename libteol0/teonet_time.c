#include "teonet_time.h"

#include <stdint.h>
#include <string.h>

#include "teonet_platform.h"

#if defined(TEONET_OS_WINDOWS)
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

// Get current time in milliseconds.
int64_t teotimeGetCurrentTime() {
    int64_t current_time;

#if defined(TEONET_OS_WINDOWS)
    struct __timeb64 time_value;
    memset(&time_value, 0, sizeof(time_value));

    _ftime64_s(&time_value);

    current_time = time_value.time * 1000 + time_value.millitm;
#else
    struct timeval time_value;
    memset(&time_value, 0, sizeof(time_value));

    gettimeofday(&time_value, 0);

    // Cast to int64_t is needed on 32-bit unix systems.
    current_time = (int64_t)time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif

    return current_time;
}

// Get amount of time between saved moment of time and current time.
int64_t teotimeGetTimePassed(int64_t time_value) {
    int64_t current_time = teotimeGetCurrentTime();

    return current_time - time_value;
}
