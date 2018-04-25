
#ifndef  TRIORAIL_TRACE_STREAM_H
#define  TRIORAIL_TRACE_STREAM_H 1


#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct TraceStream TraceStream;

extern  TraceStream *trace_stream_new(int fd, size_t maxRecordLen);
extern void trace_stream_free( TraceStream *p_rs);

extern int trace_stream_get_next ( TraceStream *p_rs, void ** p_outRecord, 
                                    size_t *p_outRecordLen);

#ifdef __cplusplus
}
#endif


#endif /*TRIORAIL_TRACE_STREAM_H*/

