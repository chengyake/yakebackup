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

#define LOG_TAG "TRIO_TRACE_CIGSM"
#include <log.h>



//�����߲���д���ļ�
void WriteFile_CIGSM(uint8_t nARFCN, uint8_t * COverIList, int * RxLevelList, 
                int * strARFCNList, GSMUmParam * pUmParam, Trio_trace_frame * pTraceFrame)
{
    uint8_t * destination;
    int index = 0;
    int frameLen = sizeof(TestFrame) + sizeof(GSMUmParam) + sizeof(uint8_t)*4*nARFCN + sizeof(GPS_data);
    RLOGD("WriteFile_CIGSM frameLen: %d", frameLen);
    
    destination = (uint8_t *)malloc(frameLen);

    destination[0] = 2;//��ͷ
    destination[1] = 11;//������
    destination[2] = 01;//ģ��������
    
    destination[5] = 0x64;//��������
    
    //save time
    writeInt64(&destination[6], getRealtimeOfCS());

    //�ƶ������롢�ƶ������롢λ�����롢С�����
    if ((pUmParam->m_LAI.nMCC != -1) && (pUmParam->m_LAI.nMNC != -1) 
        && (pUmParam->m_LAI.nLAC != -1) && (pUmParam->m_wCell_Identity != -1))
    {
        writeInt32(&destination[14], pUmParam->m_LAI.nMCC);
        writeInt32(&destination[18], pUmParam->m_LAI.nMNC);
        writeInt32(&destination[22], pUmParam->m_LAI.nLAC);
        writeInt32(&destination[26], pUmParam->m_wCell_Identity);
    }
    else
    {
        writeInt32(&destination[14], -1);
        writeInt32(&destination[18],-1);
        writeInt32(&destination[22], -1);
        writeInt32(&destination[26], -1);
    }
    destination[30] = nARFCN;

    for (int i = 0; i < nARFCN; i++)
    {
        destination[31 + (i * 4)] = (uint8_t)((int)(strARFCNList[i] & 0xff00) >> 8);
        destination[32 + (i * 4)] = (uint8_t)((int)(strARFCNList[i] & 0xff));
        destination[33 + (i * 4)] = (uint8_t)((int)(RxLevelList[i]));
        destination[34 + (i * 4)] = COverIList[i];
    }
    
    destination[31 + (nARFCN * 4)] = pTraceFrame->gps_data.direction;

    index = 32 + (nARFCN * 4);
    writeInt64(&destination[index], pTraceFrame->gps_data.Speed);
    writeInt64(&destination[index+8], pTraceFrame->gps_data.KmPosition);
    writeInt64(&destination[index+16], pTraceFrame->gps_data.latitude);
    writeInt64(&destination[index+32], pTraceFrame->gps_data.longitude);

    writeInt16(&destination[3], ((64 + (nARFCN * 4)) - 5));


    destination[64 + (nARFCN * 4)] = frameCheckSum(destination, (64 + (nARFCN * 4)) - 1, 1);
    destination[65 + (nARFCN * 4)] = 3;

    writeFrame(destination, 66 + (nARFCN * 4));
    free(destination);
    RLOGI("WriteFile_CIGSM end.");  
    
}

