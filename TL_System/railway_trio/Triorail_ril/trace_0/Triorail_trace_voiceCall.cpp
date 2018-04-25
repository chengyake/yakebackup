
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
#include <Triorail_trace_internal.h>
#include <utils.h>

#define LOG_TAG "TRIO_TRACE_VOICECALL"
#include <log.h>



/// <summary>
/// 将每次被叫的记录写入记录文件
/// </summary>
void WriteFile_IncomingCall(void * data, int len)
{
    uint8_t * destination;

    int frameLen = sizeof(TestFrame) + len;
    RLOGD("WriteFile_IncomingCall frameLen: %d", frameLen);
    
    destination = (uint8_t *)malloc(frameLen);
    destination[0] = 2;
    destination[1] = 11;
    destination[2] = 0x02;
    
    writeInt16(&destination[3], 9+len); //48

    destination[5] = 0x00; //MT call
    writeInt64(&destination[6], getRealtimeOfCS());

    writeData(&destination[14], data, len);

    destination[14+len] = frameCheckSum(destination, len+13, 1); //52
    destination[15+len] = 3;

    writeFrame(destination, frameLen); //55
    free(destination);
    return;

}


// write the frame of the short call each time.
void WriteFile_OutgoingShortCall(void * data, int len)
{
    uint8_t * destination;

    int frameLen = sizeof(TestFrame) + len;
    RLOGD("WriteFile_OutgoingShortCall frameLen: %d", frameLen);
    
    destination = (uint8_t *)malloc(frameLen);
    destination[0] = 2;
    destination[1] = 11;
    destination[2] = 0x01;
    
    writeInt16(&destination[3], 9+len);

    destination[5] = 0x10; //voice call
    writeInt64(&destination[6], getRealtimeOfCS());

    writeData(&destination[14], data, len);

    destination[14+len] = frameCheckSum(destination, len+13, 1);

    destination[15+len] = 3;

    writeFrame(destination, frameLen);
    free(destination);
    return;
}

// write the frame of short call statistics.
void WriteFile_ShortCallStatistic(void * data, int len)
{

    uint8_t * destination;

    int frameLen = sizeof(TestFrame) + len;
    RLOGD("WriteFile_ShortCallStatistic frameLen: %d", frameLen);
    
    destination = (uint8_t *)malloc(frameLen);
    destination[0] = 2;
    destination[1] = 11;
    destination[2] = 0x01;
    
    writeInt16(&destination[3], 9+len);

    destination[5] = 0x10; //voice call
    
    writeInt64(&destination[6], getRealtimeOfCS());

    writeData(&destination[14], data, len);

    destination[14+len] = frameCheckSum(destination, len+13, 1);

    destination[15+len] = 3;

    writeFrame(destination, frameLen);
    free(destination);
    return;

}

