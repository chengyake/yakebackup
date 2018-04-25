
#ifndef LIBRILUTILS_H
#define LIBRILUTILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return system time in nanos.
 *
 * This is a monotonicly increasing clock and
 * return the same value as System.nanoTime in java.
 */
uint64_t ril_nano_time();
int64_t elapsedRealtime();
void __log_print(int prio, const char *tag, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // LIBRILUTILS_H
