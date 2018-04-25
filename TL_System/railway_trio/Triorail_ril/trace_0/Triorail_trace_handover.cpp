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

#define LOG_TAG "TRIO_TRACE_HANDOVER"
#include <log.h>



void WriteFile_handover(Handover & handover)
{
    uint8_t * destination;
    int index = 0;
    int frameLen = sizeof(TestFrame) + sizeof(Handover);
    RLOGD("WriteFile_handover frameLen: %d", frameLen);
    
    destination = (uint8_t *)malloc(frameLen);

    destination[0] = 2;
    destination[1] = 11;
    destination[2] = 01;
    
    destination[5] = 0x70;

    //save time
    writeInt64(&destination[6], getRealtimeOfCS());

    //====data ===

    writeInt64(&destination[14], handover.HandOverEndTime);
    writeInt64(&destination[22], handover.HandoverStartTime);

    writeInt64(&destination[30], handover.HandoverStartTimestamp);
    writeInt64(&destination[38], handover.HandOverEndTimestamp);
    
    //switch time used, ms.
    writeInt16(&destination[46], (handover.HandOverEndTime - handover.HandoverStartTime)/10000);

    destination[48] = handover.nType;
    destination[49] = handover.nResult;

    writeInt16(&destination[50], handover.nBCCH_start);
    writeInt16(&destination[52], handover.nBCCH_end);


    destination[54] = handover.nNcc_Start;
    destination[55] = handover.nBcc_Start;

    destination[56] = handover.nNcc_End;
    destination[57] = handover.nBcc_End;


    //gps data.
    writeData(&destination[58], &(handover.nKmPost_Start), 64);

    writeInt16(&destination[3], 107);

    destination[112] = frameCheckSum(destination, 111, 1);
    destination[113] = 3;

    writeFrame(destination, 114);
    free(destination);

}


