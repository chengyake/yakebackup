#ifndef TRIORAIL_TRACE_INTER_H
#define TRIORAIL_TRACE_INTER_H 1

#include <stdlib.h>
#include <stdint.h>
#include <Triorail_trace_msg.h>

#ifdef __cplusplus
extern "C" {
#endif

extern GSMUmParam g_GSMUmParam;
extern L1Data g_L1Msg; 
extern trace_GPS_Info_ind g_GPS_Info;
extern trace_task_config_ind g_taskConfig_Info;

typedef struct Trio_trace_frame  Trio_trace_frame;
struct Trio_trace_frame{
    Trio_trace_frame *next;
    Trio_trace_frame *prev;
    int  totalFrames;
    int64_t frameID;
    uint64_t timestamp;
    GPS_data gps_data;
    int dataLen;
    uint8_t *data;
};


extern void writeFrame(void * data, int len);
extern void writeTraceFileHead(char * railLineName, char * railwayBureauName);
extern void WriteFile_OutgoingShortCall(void * data, int len);
extern void WriteFile_IncomingCall(void * data, int len);
extern void WriteFile_ShortCallStatistic(void * data, int len);

extern void WriteFile_AT(void * data, int len);
extern void processOTRByTypes(Trio_trace_frame * pTraceFrame);
extern void WriteFile_L2(L2Data & L2Msg);
extern void WriteFile_L3(L3Data & L3Msg);
extern void WriteFile_L1Info(L1Data & L1Msg);
extern void PreprocessL3Message(Trio_trace_frame * pTraceFrame);
extern void WriteFile_CIGSM(uint8_t nARFCN, uint8_t * COverIList, int * RxLevelList, 
                int * strARFCNList, GSMUmParam * pUmParam, Trio_trace_frame * pTraceFrame);
extern void WriteFile_handover(Handover & handover);

#ifdef __cplusplus
}
#endif

#endif /*TRIORAIL_TRACE_INTER_H*/
