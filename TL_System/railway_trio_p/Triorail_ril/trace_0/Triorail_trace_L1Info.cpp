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

#define LOG_TAG "TRIO_TRACE_L1"
#include <log.h>



static uint64_t getL1FrameID()
{
    static uint64_t msgID = 0;
    msgID += 1LL;
    if (msgID == 0x7fffffffLL)
    {
        msgID = 1LL;
    }
    return msgID;
}

void WriteIdleInfo(cellInfo pLayer1Info, int & Index,  uint8_t * RefBuffer)
{
    
    writeInt16(&RefBuffer[Index], pLayer1Info.nRxLevel);
    writeInt16(&RefBuffer[Index+2], pLayer1Info.nBCCH);

    RefBuffer[Index + 4] = pLayer1Info.nNCC;
    RefBuffer[Index + 5] = pLayer1Info.nBCC;

    writeInt16(&RefBuffer[Index+6], pLayer1Info.nC1);
    writeInt16(&RefBuffer[Index+8], pLayer1Info.nC2);

    Index += 10;
}

void WriteDedicatedInfo(cellInfo pLayer1Info, int & Index,  uint8_t * RefBuffer)
{
    writeInt16(&RefBuffer[Index], pLayer1Info.nRxLevel);
    writeInt16(&RefBuffer[Index+2], pLayer1Info.nBCCH);

    RefBuffer[Index + 4] = pLayer1Info.nNCC;
    RefBuffer[Index + 5] = pLayer1Info.nBCC;

    Index += 6;
}

void WriteFile_L1Info(L1Data & L1Msg)
{
    uint8_t * destination;
    int index = 0;
    int frameLen = sizeof(TestFrame) + sizeof(L1Data);
    RLOGD("WriteFile_L1Info ");

    destination = (uint8_t *)malloc(frameLen);

    destination[0] = 2;
    destination[1] = 11;
    destination[2] = 01;
    destination[5] = 0x60;

    
    //save time
    writeInt64(&destination[6], getRealtimeOfCS());

    //====data ===
    //frameID
    writeInt64(&destination[14], getL1FrameID());

    destination[22] = L1Msg.mode;

    if (L1Msg.mode == 0) {
        index = 23;
        WriteIdleInfo(L1Msg.servingCellInfo, index, destination);
        WriteIdleInfo(L1Msg.neighbour1CellInfo, index, destination);
        WriteIdleInfo(L1Msg.neighbour2CellInfo, index, destination);
        WriteIdleInfo(L1Msg.neighbour3CellInfo, index, destination);
        WriteIdleInfo(L1Msg.neighbour4CellInfo, index, destination);
        WriteIdleInfo(L1Msg.neighbour5CellInfo, index, destination);
        WriteIdleInfo(L1Msg.neighbour6CellInfo, index, destination);
    } else if (L1Msg.mode == 1) {
        destination[23] = L1Msg.servingCellInfo.nRxQual;
        writeInt16(&destination[24], L1Msg.servingCellInfo.nRxLevel);

        destination[26] = L1Msg.servingCellSubInfo.nRxQual;
        writeInt16(&destination[27], L1Msg.servingCellSubInfo.nRxLevel);
        writeInt16(&destination[29], L1Msg.servingCellSubInfo.nBCCH);
        destination[31] = L1Msg.servingCellSubInfo.nNCC;
        destination[32] = L1Msg.servingCellSubInfo.nBCC;

        destination[33] = L1Msg.servingCellInfo.nTA;

        index = 34;
        WriteDedicatedInfo(L1Msg.neighbour1CellInfo, index, destination);
        WriteDedicatedInfo(L1Msg.neighbour2CellInfo, index, destination);
        WriteDedicatedInfo(L1Msg.neighbour3CellInfo, index, destination);
        WriteDedicatedInfo(L1Msg.neighbour4CellInfo, index, destination);
        WriteDedicatedInfo(L1Msg.neighbour5CellInfo, index, destination);
        WriteDedicatedInfo(L1Msg.neighbour6CellInfo, index, destination);
    }

    
    writeInt64(&destination[index], L1Msg.KmPosition);

    //date len
    writeInt16(&destination[3], ((index + 8) - 5));

    destination[index + 8] = frameCheckSum(destination, (index + 8) - 1, 1);
    destination[index + 9] = 3;

    //writeFrame(destination, frameLen);
    writeFrame(destination, index + 10);
    free(destination);

}


