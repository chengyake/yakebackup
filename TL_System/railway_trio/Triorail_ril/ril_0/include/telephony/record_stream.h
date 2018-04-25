/*
 * A simple utility for reading fixed records out of a stream fd
 */
#include <stdio.h>

#ifndef _CUTILS_RECORD_STREAM_H
#define _CUTILS_RECORD_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct RecordStream RecordStream;

extern RecordStream *record_stream_new(int fd, size_t maxRecordLen);
extern void record_stream_free(RecordStream *p_rs);

extern int record_stream_get_next (RecordStream *p_rs, void ** p_outRecord, 
                                    size_t *p_outRecordLen);

#ifdef __cplusplus
}
#endif


#endif /*_CUTILS_RECORD_STREAM_H*/

