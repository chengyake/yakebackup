
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>
#include <string.h>

#include <Triorail_trace.h>
#include <Triorail_trace_msg.h>
#include <Triorail_trace_internal.h>
#include <utils.h>

#define LOG_TAG "TRIO_TRACE"
#include <log.h>

static Trace_file_head s_tra_file_head;




 void writeTraceFileHead(char * railLineName, char * railwayBureauName) {
    RLOGI("writeTraceFileHead");   
    int trio_count = 0;
    uint8_t * destination;
    uint8_t * index;
    int len = 0;
    
    if (g_taskConfig_Info.Trio_0_taskConfig == 1) {
        trio_count++;
    }
    if (g_taskConfig_Info.Trio_1_taskConfig == 1) {
        trio_count++;
    }

    if (trio_count == 0) {
        trio_count = 1;
        
        s_tra_file_head.triorail_data[0].Triorail_ID = 01;
        s_tra_file_head.triorail_data[0].TriorailTestType = 0x10;
        s_tra_file_head.triorail_data[0].TriorailSetType = 0x01;
        s_tra_file_head.triorail_data[0].TriorailCallType = 0x01;
    }
    int frameLen = sizeof(Trace_file_head) - 
        (TRIORAIL_COUNT_MAX-trio_count) * sizeof(Triorail_config);
    RLOGD("writeTraceFileHead frameLen: %d", frameLen);

    destination = (uint8_t *)malloc(frameLen);

    s_tra_file_head.headLen = 74+trio_count*4;
    s_tra_file_head.timestamp = getRealtimeOfCS();
    s_tra_file_head.ESPI1 = 0;
    s_tra_file_head.ESPI1Config = 0;
    s_tra_file_head.ESPI2 = 1;
    s_tra_file_head.ESPI2Config = 0;
    s_tra_file_head.OT498_890_Flag = 2;
    s_tra_file_head.OT498_890_Num = 0;
    s_tra_file_head.SELEX_Flag = 3;
    s_tra_file_head.SELEX_Num = 0;
    s_tra_file_head.SAGEM_Flag = 4;
    s_tra_file_head.SAGEM_Num = 0;
    s_tra_file_head.Triorail_Flag = 5;
    s_tra_file_head.Triorail_Num = trio_count;

    if (g_taskConfig_Info.Trio_0_taskConfig == 1) {
        s_tra_file_head.triorail_data[0].Triorail_ID = 01;
        s_tra_file_head.triorail_data[0].TriorailTestType = g_taskConfig_Info.Trio_0_testType;
        s_tra_file_head.triorail_data[0].TriorailSetType = g_taskConfig_Info.Trio_0_modemType;
        s_tra_file_head.triorail_data[0].TriorailCallType = g_taskConfig_Info.Trio_0_callType;
    }
    
    if (g_taskConfig_Info.Trio_1_taskConfig == 1) {
        s_tra_file_head.triorail_data[1].Triorail_ID = 02;
        s_tra_file_head.triorail_data[1].TriorailTestType = g_taskConfig_Info.Trio_1_testType;
        s_tra_file_head.triorail_data[1].TriorailSetType = g_taskConfig_Info.Trio_1_modemType;
        s_tra_file_head.triorail_data[1].TriorailCallType = g_taskConfig_Info.Trio_1_callType;
    }
    

    s_tra_file_head.OT290_Flag = 6;
    s_tra_file_head.OT290_Num = 0;
    s_tra_file_head.GPRSPingFlag = 7;
    s_tra_file_head.GPRSPingArg[0] = 0;
    s_tra_file_head.GPRSPingArg[1] = 0;
    s_tra_file_head.GPRSPingArg[2] = 0;
    
    s_tra_file_head.GPRS_UDP_flag = 8;
    s_tra_file_head.GPRS_UDP_Arg[0] = 0;
    s_tra_file_head.GPRS_UDP_Arg[1] = 0;
    s_tra_file_head.GPRS_UDP_Arg[2] = 0;
    
    s_tra_file_head.GPRS_FTP_flag = 9;
    s_tra_file_head.GPRS_FTP_Arg = 0;

    s_tra_file_head.controlOrderFlag= 0x0A; //new
    s_tra_file_head.controlOrderConfigFlag= 0;//new
    s_tra_file_head.CIRTFlag = 0x0B;
    s_tra_file_head.CIRTArg = 0;
    s_tra_file_head.baseStationFlag = 0x10;
    s_tra_file_head.baseStationNum = 0;
    s_tra_file_head.rangeEquipmentFlag = 0x11;
    s_tra_file_head.rangeEquipmentNum = 0;

    s_tra_file_head.railLineLen = 0x09;
    memset(s_tra_file_head.railLineName, 0, sizeof(s_tra_file_head.railLineName ));
    s_tra_file_head.railwayBureauLen = 0x09;
    memset(s_tra_file_head.railwayBureauName, 0, sizeof(s_tra_file_head.railwayBureauName ));

//not use now.    
#if 0    
    if (strlen(railLineName) > s_tra_file_head.railLineLen){
        RLOGE("writeTraceFileHead err, railLineName is too long."); 
        memcpy(s_tra_file_head.railLineName, railLineName, s_tra_file_head.railLineLen);
    }else {
        memcpy(s_tra_file_head.railLineName, railLineName, strlen(railLineName));
        s_tra_file_head.railLineLen = strlen(railLineName);
    }
    
    if (strlen(railwayBureauName) > s_tra_file_head.railwayBureauLen){
        RLOGE("writeTraceFileHead err, railwayBureauLen is too long."); 
        memcpy(s_tra_file_head.railwayBureauName, railwayBureauName, s_tra_file_head.railwayBureauLen);
    }else {
        memcpy(s_tra_file_head.railwayBureauName, railwayBureauName, strlen(railwayBureauName));
        s_tra_file_head.railwayBureauLen = strlen(railwayBureauName);
    }
#endif

    s_tra_file_head.rfu[0] = 01;
    s_tra_file_head.rfu[1] = 01;
    
    s_tra_file_head.GPS_map_flag = 0x12;
    s_tra_file_head.toLoad = 0;

    len = (size_t)&(s_tra_file_head.triorail_data) - (size_t)&(s_tra_file_head.headLen);
    RLOGD("writeTraceFileHead len: %d", len);
    memcpy(destination, &(s_tra_file_head.headLen), len);

    index = destination + len;
    len = trio_count* sizeof(Triorail_config);
    memcpy(index, &s_tra_file_head.triorail_data, len);

    index = index + len;
    len = (size_t)&(s_tra_file_head.CRC) - (size_t)&(s_tra_file_head.OT290_Flag);
    memcpy(index, &s_tra_file_head.OT290_Flag, len);

    index = index + len;
    uint8_t crc[2] = {0};
    CRC16(destination, frameLen -2, crc);
    s_tra_file_head.CRC = ((uint16_t)crc[0]<<8) + crc[1];
    writeInt16(index, s_tra_file_head.CRC);
    index += 2;

    len = index - destination;
    RLOGD("writeTraceFileHead end len: %d", len);
    
    writeFrame(destination, frameLen);     
    free(destination);
}

