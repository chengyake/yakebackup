
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>

#include <Triorail_trace_msg.h>
#include <Triorail_trace.h>
#include <utils.h>
#include <Triorail_trace_internal.h>

#define LOG_TAG "TRIO_TRACE_AT"
#include <log.h>

// write the frame of the all the AT string.
void WriteFile_AT(void * data, int len)
{
    trace_AT_data_ind * pAT_ind;
    uint8_t * destination;
    int AT_len;

    pAT_ind = (trace_AT_data_ind *)data;
    
    int frameLen = sizeof(TestFrame) + len;
    
    RLOGD("WriteFile_AT frameLen: %d", frameLen);

    
    destination = (uint8_t *)malloc(frameLen);

    destination[0] = 2;
    destination[1] = 11;
    destination[2] = 01;
    writeInt16(&destination[3], 9+len);
    
    destination[5] = 0x50;

    writeInt64(&destination[6], getRealtimeOfCS());
    
    destination[14] = pAT_ind->AT_dir;
    
    AT_len = pAT_ind->AT_len;
    destination[15] = pAT_ind->AT_len;
    writeData(&destination[16], pAT_ind->AT_str, AT_len);

    writeData(&destination[16+AT_len], &(pAT_ind->KmPosition), 33);

    destination[16+AT_len+33] =  frameCheckSum(destination, len+13, 1);

    destination[16+AT_len+34] = 3;

    writeFrame(destination, frameLen);
    free(destination);
    return;

}





