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

#define LOG_TAG "TRIO_TRACE_L2"
#include <log.h>


void WriteFile_L2(L2Data & L2Msg)
{
    uint8_t * destination;
    int index = 0;
    int frameLen = sizeof(TestFrame) + sizeof(L2Data) + L2Msg.msgLen- sizeof(uint8_t *);
    RLOGD("WriteFile_L2 frameLen: %d, msgLen: %d", frameLen, L2Msg.msgLen);

    destination = (uint8_t *)malloc(frameLen);

    destination[0] = 2;
    destination[1] = 11;
    destination[2] = 01;
    
    destination[5] = 0x62;

    //save time
    writeInt64(&destination[6], getRealtimeOfCS());

    //====data ===
    //frameID
    writeInt64(&destination[14], L2Msg.msgID);
    //frame time
    writeInt64(&destination[22], L2Msg.timestamp);
    destination[30] = L2Msg.msgChannelCode;
    
    destination[31] = L2Msg.msgDirection;
    destination[32] = L2Msg.msgLen;

    writeData(&destination[33], L2Msg.msgData, L2Msg.msgLen);

    writeData(&destination[33+L2Msg.msgLen], &(L2Msg.Speed), 32);
    
    writeInt16(&destination[3], 33+L2Msg.msgLen+32-5);
    
    destination[33+L2Msg.msgLen+32] = frameCheckSum(destination, L2Msg.msgLen+64, 1);
    destination[33+L2Msg.msgLen+33] = 3;


    writeFrame(destination, 33+L2Msg.msgLen+34);
    RLOGD("WriteFile_L2  free begin.");  
    free(destination);
    RLOGD("WriteFile_L2  free end.");   
}
