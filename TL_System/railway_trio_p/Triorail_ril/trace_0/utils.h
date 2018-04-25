#ifndef TRIORAIL_TRACE_UTILS_H
#define TRIORAIL_TRACE_UTILS_H 1

#include <stdlib.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

extern void  writeInt16(void * p, int data);
extern void  writeInt32(void * p, int data);
extern void  writeInt64 (void * p, int64_t data);
extern void  writeData(void * p, const void *s, int len);
extern uint8_t frameCheckSum(uint8_t * ptr, int len, int offset);
extern void CRC16(uint8_t * ptr, int size, uint8_t * crc_result);

extern uint64_t ril_nano_time();
extern int64_t elapsedRealtime();
extern uint64_t getRealtimeOfCS();
extern void __log_print(int prio, const char *tag, const char *fmt, ...);
#ifdef __cplusplus
}
#endif

#endif /*TRIORAIL_TRACE_UTILS_H*/
