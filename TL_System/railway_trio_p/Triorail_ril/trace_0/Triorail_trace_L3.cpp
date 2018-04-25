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

#define LOG_TAG "TRIO_TRACE_L3"
#include <log.h>


void WriteFile_L3(L3Data & L3Msg)
{
    uint8_t * destination;
    int index = 0;
    int frameLen = sizeof(TestFrame) + sizeof(L3Data) + L3Msg.msgLen - sizeof(uint8_t *);
    RLOGD("WriteFile_L3 frameLen: %d, msgLen: %d", frameLen, L3Msg.msgLen);
    
    destination = (uint8_t *)malloc(frameLen);
    
    destination[0] = 2;
    destination[1] = 11;
    destination[2] = 01;

    destination[5] = 0x63;
        
    //save time
    writeInt64(&destination[6], getRealtimeOfCS());

    //====data ===
    //frameID
    writeInt64(&destination[14], L3Msg.msgID);
    //frame time
    writeInt64(&destination[22], L3Msg.timestamp);

    destination[30] = L3Msg.msgDirection;
    destination[31] = L3Msg.msgLen;
    writeData(&destination[32], L3Msg.msgData, L3Msg.msgLen);
    writeData(&destination[32+L3Msg.msgLen], &(L3Msg.Speed), 32);
    
    writeInt16(&destination[3], 32+L3Msg.msgLen+32-5);

    destination[32+L3Msg.msgLen+32] =  frameCheckSum(destination, L3Msg.msgLen+63, 1);
    destination[32+L3Msg.msgLen+33] = 3;

    //writeFrame(destination, frameLen);
    writeFrame(destination, 32+L3Msg.msgLen+33+1);
    free(destination);
    RLOGD("WriteFile_L3  end.");  
    return;
}
