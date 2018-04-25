
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#ifdef HAVE_WINSOCK
#include <winsock2.h>   /* for ntohl */
#else
#include <netinet/in.h>
#endif
#include <stdbool.h>

#include <utils.h>
#include <Triorail_trace_msg.h>
#include <Triorail_trace_stream.h>

#define LOG_TAG "trace_stream"
#include <log.h>

#define debug_trace_stream__
#define STREAM_DEBUG 0


#define HEADER_SIZE 4


struct TraceStream {
    int fd;
    size_t maxRecordLen;

    unsigned char *buffer;

    unsigned char *unconsumed;
    unsigned char *read_end;
    unsigned char *buffer_end;
};


 TraceStream *trace_stream_new(int fd, size_t maxRecordLen)
{
    TraceStream *ret;
    RLOGI("trace_stream_new.");

    assert (maxRecordLen <= 0xffff);

    ret = (TraceStream *)calloc(1, sizeof(TraceStream));

    ret->fd = fd;
    ret->maxRecordLen = maxRecordLen;
    ret->buffer = (unsigned char *)malloc (maxRecordLen );

    ret->unconsumed = ret->buffer;
    ret->read_end = ret->buffer;
    ret->buffer_end = ret->buffer + maxRecordLen ;

    return ret;
}


 void trace_stream_free(TraceStream *rs)
{
    free(rs->buffer);
    free(rs);
}


/* returns NULL; if there isn't a full frame in the buffer */
static unsigned char * getEndOfFrame (unsigned char *p_begin,
                                            unsigned char *p_end, int & dummy_len)
{
    size_t inputLen;
    unsigned char * p_ret;
    bool ret = false;
    size_t i;
    size_t appMsgLen;
    unsigned char * p_frameBegin = p_begin;
    uint8_t xorValue = 0;

    dummy_len = 0;
    if (STREAM_DEBUG) RLOGD ("getEndOfFrame begin.");
    while (p_begin < p_end){
        if (p_end < p_begin + HEADER_SIZE) {
            return NULL;
        }
        inputLen = p_end - p_frameBegin;
		if (STREAM_DEBUG)RLOGD ("getEndOfFrame inputLen=%d.", inputLen);
        
        i = 0;
        ret = false;
        while (i < inputLen){
            if ((p_begin[i] == 0x02) && (p_begin[i+1] == 0x00)){
                //find the STX and app ID.
                ret = true;
                break;
            } else {
                i++;
            }
        }
        
        dummy_len += i;
        if (!ret){
            return NULL;
        }
        p_frameBegin = &p_begin[i];
        inputLen = p_end - p_frameBegin;
            
        p_begin = p_begin + i + 2;
        appMsgLen =  ((p_begin[0] & 0x1F)<<8) +  p_begin[1];  
		if (STREAM_DEBUG) RLOGD ("getEndOfFrame appMsgLen=%d", appMsgLen);
		
        if (inputLen < ((4 + appMsgLen) + 2))
        {   
            return NULL;
        }
        
        p_begin += 2;
        //find ETX (1 byte)
        if (p_begin[appMsgLen+1] == 0x03) {
			if (STREAM_DEBUG) RLOGD ("getEndOfFrame  get ETX");
            xorValue = frameCheckSum(p_frameBegin, appMsgLen + 3, 1);
            if (STREAM_DEBUG) RLOGD ("getEndOfFrame xorValue=0x%02x", xorValue);
            if (xorValue == p_begin[appMsgLen]) {
                //get a full frame.
                p_ret = &p_begin[appMsgLen+2];
                return p_ret;
            }
        }
        p_frameBegin++;
        p_begin = p_frameBegin;
    }
    return NULL;
}

static void *getNextFrame (TraceStream *p_rs, size_t *p_outRecordLen)
{
    unsigned char *record_start, *record_end;
    int dummy_len;
    record_end = getEndOfFrame (p_rs->unconsumed, p_rs->read_end, dummy_len);
  
    if (dummy_len>0) {
		RLOGE ("getNextFrame after getEndOfFrame dummy_len=%d", dummy_len);
#ifdef debug_trace_stream  		
        for (int i=0; i<dummy_len; i++) {
            printf("%02x", *(p_rs->unconsumed+i) );
        }
        fflush(stdout);
#endif            
    }

    if (record_end != NULL) {
        /* one full frame in the buffer */
        record_start = p_rs->unconsumed + dummy_len;
        p_rs->unconsumed = record_end;

        *p_outRecordLen = record_end - record_start;
		if (STREAM_DEBUG) RLOGD ("getNextFrame p_outRecordLen=%zu",  *p_outRecordLen);
        return record_start;
    }
	p_rs->unconsumed  += dummy_len;
    return NULL;
}

/**
 * Reads the next record from stream fd
 * Records are prefixed by a 16-bit big endian length value
 * Records may not be larger than maxRecordLen
 *
 * Doesn't guard against EINTR
 *
 * p_outRecord and p_outRecordLen may not be NULL
 *
 * Return 0 on success, -1 on fail
 * Returns 0 with *p_outRecord set to NULL on end of stream
 * Returns -1 / errno = EAGAIN if it needs to read again
 */
int trace_stream_get_next (TraceStream *p_rs, void ** p_outRecord,
                                    size_t *p_outRecordLen)
{
    void *ret;
	
    ssize_t countRead;
#if 0
	*p_outRecord = p_rs->buffer;
	*p_outRecordLen =  50;
	return 0;
#endif	
	
	
    /* is there one record already in the buffer? */
    ret = getNextFrame (p_rs, p_outRecordLen);

    if (ret != NULL) {
        *p_outRecord = ret;
        return 0;
    }

    // if the buffer is full and we don't have a full record
    if (p_rs->unconsumed == p_rs->buffer
        && p_rs->read_end == p_rs->buffer_end
    ) {
        // this should never happen
        //ALOGE("max record length exceeded\n");
        assert (0);
        errno = EFBIG;
        return -1;
    }

    if (p_rs->unconsumed != p_rs->buffer) {
        // move remainder to the beginning of the buffer
        size_t toMove;

        toMove = p_rs->read_end - p_rs->unconsumed;
        if (toMove) {
            memmove(p_rs->buffer, p_rs->unconsumed, toMove);
        }
		if (STREAM_DEBUG) RLOGI ("frame_stream_get_next  toMove: %d:\n", toMove);
        p_rs->read_end = p_rs->buffer + toMove;
        p_rs->unconsumed = p_rs->buffer;
    }

    countRead = read (p_rs->fd, p_rs->read_end, p_rs->buffer_end - p_rs->read_end);
    if (STREAM_DEBUG) RLOGI ("frame_stream_get_next  countRead: %zd:\n", countRead);
#ifdef debug_trace_stream        
    for (int i=0; i<countRead; i++) {
        printf("0x%02x,", *(p_rs->read_end+i) );
    }
    printf("\n");
    fflush(stdout);
#endif
    if (countRead <= 0) {
        /* note: end-of-stream drops through here too */
        *p_outRecord = NULL;
        return countRead;
    }

    p_rs->read_end += countRead;

    ret = getNextFrame (p_rs, p_outRecordLen);

    if (ret == NULL) {
        /* not enough of a buffer to for a whole command */
        errno = EAGAIN;
        return -1;
    }

    *p_outRecord = ret;
    return 0;
}
