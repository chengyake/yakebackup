
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

#define LOG_TAG "TRIO_TRACE_MSG"
#include <log.h>


L1Data g_L1Msg; 

static void processLayerMessageByCategory(Trio_trace_frame * pTraceFrame);
static void processTraceLayerMessageBySubTypes(Trio_trace_frame * pTraceFrame);
static void processL2Message(Trio_trace_frame * pTraceFrame);
static void processL3Message(Trio_trace_frame * pTraceFrame);


static void processQoSMessageByCategory(Trio_trace_frame * pTraceFrame);
static void processQoSMessageBySubTypes(Trio_trace_frame * pTraceFrame);
static void processCOverI(Trio_trace_frame * pTraceFrame);
static void processLayerStateAndMeasurementInfoMessageByCategory(Trio_trace_frame * pTraceFrame);
static void processLayerStateAndMeasurementInfoMessageBySubTypes(Trio_trace_frame * pTraceFrame);
static void processL1Info(Trio_trace_frame * pTraceFrame);
static void processL1InfoDedicated(Trio_trace_frame * pTraceFrame);
static void processL1InfoIdle(Trio_trace_frame * pTraceFrame);//解析层一消息，并且填充服务小区和6个邻小区的结构变量
static void processForcingMessageByCategory(Trio_trace_frame * pTraceFrame);
static void processForcingMessageBySubTypes(Trio_trace_frame * pTraceFrame);
static void processMobileInfoMessageByCategory(Trio_trace_frame * pTraceFrame);
static void processMobileInfoMessageBySubTypes(Trio_trace_frame * pTraceFrame);
static void processControlMessageByCategory(Trio_trace_frame * pTraceFrame);
static void processControlMessageBySubTypes(Trio_trace_frame * pTraceFrame);
static void processTraceStorageMessageByCategory(Trio_trace_frame * pTraceFrame);
static void processTraceStorageMessageBySubTypes(Trio_trace_frame * pTraceFrame);
static void processForcingMessage1ByCategory(Trio_trace_frame * pTraceFrame);
static void processForcingMessage1BySubTypes(Trio_trace_frame * pTraceFrame);
static void processGPSDataByCategory(Trio_trace_frame * pTraceFrame);
static void processTraceGPSData(Trio_trace_frame * pTraceFrame);
static double COverITranslate(uint8_t COverIDecimalValue);
static void FillNeighbourCellInfoDedicated(cellInfo & neighbourCellInfo, uint8_t * pTriorailMessage, int nStartIndex);
static void FillNeighbourCellInfoIdle(cellInfo & neighbourCellInfo, uint8_t * pTriorailMessage, int nStartIndex);


static int64_t getFrameID()
{
    static int64_t msgID = 0;
    msgID += 1LL;
    if (msgID == 0x7fffffffLL)
    {
        msgID = 1LL;
    }
    return msgID;
}



//分配所有消息的处理规则,pTriorailMessage[1]==0x00
void processOTRByTypes(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;
    //Type
    uint8_t iType = (pTriorailMessage[4] & 0xf0) >> 4;
    
    RLOGD("processOTRByTypes, nLen=%d.", nLen);  
    RLOGD("processOTRByTypes, iType=%d.", iType);  
    
    switch (iType)
    {
        case 0x00://Layer message trace
            processLayerMessageByCategory(pTraceFrame);
            break;
        case 0x01://Quality of service indicator
            processQoSMessageByCategory(pTraceFrame);
            break;
        case 0x02://Layer state and measurement information
            processLayerStateAndMeasurementInfoMessageByCategory(pTraceFrame);
            break;
        case 0x03://Forcing message
            processForcingMessageByCategory(pTraceFrame);
            break;
        case 0x04://Mobile Information message
            processMobileInfoMessageByCategory(pTraceFrame);
            break;
        case 0x05://Control message
            processControlMessageByCategory(pTraceFrame);
            break;
        case 0x06://Trace storage
            processTraceStorageMessageByCategory(pTraceFrame);
            break;
        case 0x07://Forcing message 1
            processForcingMessage1ByCategory(pTraceFrame);
            break;
        case 0x0F://GPS data
            processGPSDataByCategory(pTraceFrame);
            break;
        default: 
            RLOGE("processOTRByTypes, err OTR type.");  
            break;

    }
}

//Type为0x00时按不同的Category分发消息
static void processLayerMessageByCategory(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 4) {
        uint8_t iCategory = pTriorailMessage[4] & 7;
        switch (iCategory) {
            case 0x00://Command PC->MS
            case 0x01://Request PC->MS
            case 0x02://Reply PC<-MS
                break;
            case 0x03://此时为Trace消息, PC<-MS
                processTraceLayerMessageBySubTypes( pTraceFrame);
                break;
            case 0x04://Information, PC<-MS
            case 0x05://Error,PC<-MS
            case 0x06://Reserved
            case 0x07://Stored trace message,PC<-MS
                break;
            default: 
                RLOGE("processLayerMessageByCategory, unknow.");  
                break;
        }
    }
}

    
//Type为0x00(Layer Message),Category为0x03(Trace)时，按照不同的SubType分发消息, 层二层三消息
static void processTraceLayerMessageBySubTypes(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;
    RLOGI("processTraceLayerMessageBySubTypes.");  
    
    if (nLen >= 6)
    {
        uint8_t iSubType = pTriorailMessage[5] & 0x1f;
        switch (iSubType)
        {
            case 0x00://GSM L3 message
                processL3Message( pTraceFrame);
                break;
            case 0x01://GPRS RLC/MAC Control message
                break;
            case 0x02://GPRS GMM-SM message
                break;
            case 0x03://GSM L2 message
                processL2Message(pTraceFrame);
                break;
            case 0x04://(E)GPRS RLC/MAC Header
                break;
            case 0x05://RATSCCH trace messages
                break;
            case 0x06://LLC signalisation
                break;
            default: 
                RLOGE("processTraceLayerMessageBySubTypes, unknow.");  
                break;
        }
    }
}


//剥离层二消息
static void processL2Message(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;
    RLOGI("processL2Message.");          
    int iL2MsgLen = (0x100 * pTriorailMessage[7]) + pTriorailMessage[8];
    if (nLen <9) {
        RLOGE("processL2Message, nLen <9.");  
        return;
    }
    
    L2Data L2Msg;
    L2Msg.msgID = getFrameID();
    //L2Msg.msgID = pTraceFrame->frameID;
    
    L2Msg.msgLen = iL2MsgLen;
    L2Msg.msgData = pTriorailMessage + 9;
    L2Msg.timestamp = pTraceFrame->timestamp;
    L2Msg.KmPosition = pTraceFrame->gps_data.KmPosition;
    L2Msg.Speed= pTraceFrame->gps_data.Speed;
    L2Msg.latitude= pTraceFrame->gps_data.latitude;
    L2Msg.longitude= pTraceFrame->gps_data.longitude;
    
    if ((int)(pTriorailMessage[6] & 1))//U/D 0x00 Downlink, 0x01 Uplink
    {
        L2Msg.msgDirection = 1;
    }
    else
    {
        L2Msg.msgDirection = 0;
    }
    
    const char * strFormat;
    //Format 11110
    L2Msg.msgChannelCode = ((pTriorailMessage[6] & 0x1E) >> 1);
    switch (L2Msg.msgChannelCode)
    {
        case 0x00:
            strFormat = "NORMAL";
            break;

        case 0x01:
            strFormat = "RACH";
            break;

        case 0x02:
            strFormat = "ACCESS BURST";
            break;

        case 0x03:
            strFormat = "BCCH";
            break;

        case 0x04:
            strFormat = "PCH";
            break;

        case 0x05:
            strFormat = "CBCH";
            break;

        case 0x06:
            strFormat = "Reserved";
            break;

        case 0x07:
            strFormat = "SDCCH";
            break;

        case 0x08:
            strFormat = "SACCH";
            break;

        case 0x09:
            strFormat = "FACCH_F";
            break;

        case 0x0A:
            strFormat = "FACCH_H";
            break;

        case 0x0B:
            strFormat = "CCCH";
            break;

        case 0x0C:
            strFormat = "AGCH";
            break;

        case 0x0D:
            strFormat = "Other Channel";
            break;

        default: 
            RLOGE("processL2Message, unknow.");  
            break;
    }
    RLOGI("processTraceLayerMessageBySubTypes.  msgChannelCode=%d", 
                L2Msg.msgChannelCode);
    WriteFile_L2(L2Msg);
        
}

//剥离层三消息
static void processL3Message(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;
    if (nLen <9) {
        RLOGE("processL3Message, nLen <9.");  
        return;
    }
    int iL3MsgLen = (0x100 * pTriorailMessage[7]) + pTriorailMessage[8];
    L3Data L3Msg;
    //L3Msg.msgID = pTraceFrame->frameID;
    L3Msg.msgID = getFrameID();
    L3Msg.msgLen = iL3MsgLen;
    L3Msg.msgData = pTriorailMessage + 9;
    
    L3Msg.timestamp = pTraceFrame->timestamp;
    L3Msg.KmPosition = pTraceFrame->gps_data.KmPosition;
    L3Msg.Speed= pTraceFrame->gps_data.Speed;
    L3Msg.latitude= pTraceFrame->gps_data.latitude;
    L3Msg.longitude= pTraceFrame->gps_data.longitude;

    RLOGI("processL3Message.");  
    if ((int)(pTriorailMessage[6] & 1))//U/D 0x00 Downlink, 0x01 Uplink
    {
        L3Msg.msgDirection = 1;
    }
    else
    {
        L3Msg.msgDirection = 0;
    }

    WriteFile_L3(L3Msg);

    PreprocessL3Message(pTraceFrame);

}


//Type为0x01时按不同的Category分发消息 C/I
void processQoSMessageByCategory(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 4)
    {
        int iCategory = pTriorailMessage[4] & 7;
        switch (iCategory)
        {
            case 0x00:
            case 0x01:
            case 0x02:
                break;
            case 0x03://此时为Trace消息
                processQoSMessageBySubTypes(pTraceFrame);
                break;
            case 0x04:
            case 0x05:
            case 0x06://Reserved
            case 0x07://Stored trace message,PC<-MS
                break;
            default: 
                RLOGE("processTraceLayerMessageBySubTypes, unknow.");  
                break;
        }
    }
}


void processQoSMessageBySubTypes(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 6)
    {
        int iSubType = pTriorailMessage[5] & 0x1f;
        switch (iSubType)
        {
            case 0x00://Retransmitted RLC Block Rate
                break;
            case 0x01://RLC/MAC Data throughput
                break;
            case 0x02://DSC Counter QoS
                break;
            case 0x03://RLT Counter QoS
                break;
            case 0x04://FER
                break;
            case 0x05://EFR state
                break;
            case 0x06://DTX state
                break;
            case 0x07://RLP Resume Rate
                break;
            case 0x08://Handover Counter
                break;
            case 0x09://Reserved value
                break;
            case 0x0A://Retransmitted LLC Frame Rate
                break;
            case 0x0B://LLC Data throughput
                break;
            case 0x0C://Total RLC blocks transmitted
                break;
            case 0x0D://Total LLC frames transmitted
                break;
            case 0x0E://Downlink RLC BLER (Block Error Rate)
                break;
            case 0x0F://C/I GSM
                processCOverI(pTraceFrame);
                break;
            ////////////////////////////////
            case 0x10://AMR trace AMR trace (defined in ANNEX B AMR protocol specification)
                break;
            case 0x19://C/I GSM with high resolution of RX LEV (TTS75 or later)
                break;
            case 0x1A://C/I GSM dB Value with high resolution of RX LEV (TTS75 or later)
                break;
            case 0x1B://Frequency Correction with high resolution of RX LEV (TTS75 or later)
                break;
            case 0x1C://EGPRS Increml enta Tedundancy Memory
                break;
            case 0x1D://Frequency Correction (TTS65 and higher)
                break;
            case 0x1E://C/I GSM dB Value (TTS65 and Higher)
                break;
            default: 
                RLOGE("processQoSMessageBySubTypes, unknow.");  
                break;        }
    }
}

void processCOverI(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (pTriorailMessage[6] == 4)//headLength==4
    {
        uint8_t nNumofARFCN = pTriorailMessage[7] & 0x3f;//频点号的个数 00111111
        int nLength = pTriorailMessage[8];

        //9/10,reserved;
        int * nARFCNList;
        int * rxLevelList ;
        uint8_t * cOverIList;//0-255之间的数字

        if (nNumofARFCN > 0)
        {
            nARFCNList = (int *)malloc(nNumofARFCN * sizeof(int));
            rxLevelList = (int *)malloc(nNumofARFCN * sizeof(int));
            cOverIList = (uint8_t *)malloc(nNumofARFCN);//0-255之间的数字
            
            double * transferedCOverIList = (double *)malloc(nNumofARFCN * sizeof(double));//转换成double型的C/I值
            
            for (int i = 0; i < nNumofARFCN; i++)
            {
                int signedRxLev = (pTriorailMessage[11 + i * nLength] & 0xFC) >> 2;////11111100
                int currentARFCN = ((pTriorailMessage[11 + i * nLength] & 0x03) * 0x100)
                    + pTriorailMessage[12 + i * nLength];
                uint8_t bCOverI = pTriorailMessage[13 + i * nLength];

                rxLevelList[i] = signedRxLev;
                nARFCNList[i] = currentARFCN;
                cOverIList[i] = bCOverI;

                transferedCOverIList[i] = COverITranslate(bCOverI);
                
            }
            
        }

        WriteFile_CIGSM(nNumofARFCN, cOverIList, rxLevelList, nARFCNList,  &g_GSMUmParam, pTraceFrame);
        if (nNumofARFCN > 0) 
        {
            free(nARFCNList);
            free(rxLevelList);
            free(cOverIList);
        }
        
    }

}

double COverITranslate(uint8_t COverIDecimalValue)
{
    double num = 0.0;
    if (COverIDecimalValue > 250)
    {
        num = 20.0;
    }
    if (COverIDecimalValue < 10)
    {
        num = -6.0;
    }
    switch (COverIDecimalValue)
    {
        case 10:
            num = -4.9;
            break;

        case 11:
            num = -4.4;
            break;

        case 12:
            num = -3.9;
            break;

        case 13:
            num = -3.5;
            break;

        case 14:
            num = -3.1;
            break;

        case 15:
            num = -2.7;
            break;

        case 0x10:
            num = -2.3;
            break;

        case 0x11:
            num = -2.0;
            break;

        case 0x12:
            num = -1.7;
            break;

        case 0x13:
            num = -1.4;
            break;

        case 20:
            num = -1.1;
            break;

        case 0x15:
            num = -0.9;
            break;

        case 0x16:
            num = -0.6;
            break;

        case 0x17:
            num = -0.4;
            break;

        case 0x18:
            num = -0.2;
            break;

        case 0x19:
            num = 0.1;
            break;

        case 0x1a:
            num = 0.3;
            break;

        case 0x1b:
            num = 0.5;
            break;

        case 0x1c:
            num = 0.7;
            break;

        case 0x1d:
            num = 0.9;
            break;

        case 30:
            num = 1.1;
            break;

        case 0x1f:
            num = 1.25;
            break;

        case 0x20:
            num = 1.42;
            break;

        case 0x21:
            num = 1.59;
            break;

        case 0x22:
            num = 1.75;
            break;

        case 0x23:
            num = 1.91;
            break;

        case 0x24:
            num = 2.06;
            break;

        case 0x25:
            num = 2.21;
            break;

        case 0x26:
            num = 2.35;
            break;

        case 0x27:
            num = 2.5;
            break;

        case 40:
            num = 2.64;
            break;

        case 0x29:
            num = 2.77;
            break;

        case 0x2a:
            num = 2.9;
            break;

        case 0x2b:
            num = 3.03;
            break;

        case 0x2c:
            num = 3.15;
            break;

        case 0x2d:
            num = 3.25;
            break;

        case 0x2e:
            num = 3.39;
            break;

        case 0x2f:
            num = 3.51;
            break;

        case 0x30:
            num = 3.62;
            break;

        case 0x31:
            num = 3.74;
            break;

        case 50:
            num = 3.85;
            break;

        case 0x33:
            num = 3.95;
            break;

        case 0x34:
            num = 4.06;
            break;

        case 0x35:
            num = 4.16;
            break;

        case 0x36:
            num = 4.26;
            break;

        case 0x37:
            num = 4.37;
            break;

        case 0x38:
            num = 4.46;
            break;

        case 0x39:
            num = 4.56;
            break;

        case 0x3a:
            num = 4.65;
            break;

        case 0x3b:
            num = 4.75;
            break;

        case 60:
            num = 4.84;
            break;

        case 0x3d:
            num = 4.93;
            break;

        case 0x3e:
            num = 5.02;
            break;

        case 0x3f:
            num = 5.1;
            break;

        case 0x40:
            num = 5.19;
            break;

        case 0x41:
            num = 5.27;
            break;

        case 0x42:
            num = 5.36;
            break;

        case 0x43:
            num = 5.44;
            break;

        case 0x44:
            num = 5.52;
            break;

        case 0x45:
            num = 5.6;
            break;

        case 70:
            num = 5.68;
            break;

        case 0x47:
            num = 5.76;
            break;

        case 0x48:
            num = 5.83;
            break;

        case 0x49:
            num = 5.9;
            break;

        case 0x4a:
            num = 5.98;
            break;

        case 0x4b:
            num = 6.05;
            break;

        case 0x4c:
            num = 6.12;
            break;

        case 0x4d:
            num = 6.19;
            break;

        case 0x4e:
            num = 6.26;
            break;

        case 0x4f:
            num = 6.32;
            break;

        case 80:
            num = 6.39;
            break;

        case 0x51:
            num = 6.46;
            break;

        case 0x52:
            num = 6.52;
            break;

        case 0x53:
            num = 6.58;
            break;

        case 0x54:
            num = 6.65;
            break;

        case 0x55:
            num = 6.71;
            break;

        case 0x56:
            num = 6.78;
            break;

        case 0x57:
            num = 6.84;
            break;

        case 0x58:
            num = 6.91;
            break;

        case 0x59:
            num = 6.98;
            break;

        case 90:
            num = 7.05;
            break;

        case 0x5b:
            num = 7.12;
            break;

        case 0x5c:
            num = 7.2;
            break;

        case 0x5d:
            num = 7.27;
            break;

        case 0x5e:
            num = 7.35;
            break;

        case 0x5f:
            num = 7.42;
            break;

        case 0x60:
            num = 7.5;
            break;

        case 0x61:
            num = 7.58;
            break;

        case 0x62:
            num = 7.65;
            break;

        case 0x63:
            num = 7.72;
            break;

        case 100:
            num = 7.8;
            break;

        case 0x65:
            num = 7.87;
            break;

        case 0x66:
            num = 7.93;
            break;

        case 0x67:
            num = 7.99;
            break;

        case 0x68:
            num = 8.05;
            break;

        case 0x69:
            num = 8.11;
            break;

        case 0x6a:
            num = 8.17;
            break;

        case 0x6b:
            num = 8.23;
            break;

        case 0x6c:
            num = 8.28;
            break;

        case 0x6d:
            num = 8.33;
            break;

        case 110:
            num = 8.39;
            break;

        case 0x6f:
            num = 8.42;
            break;

        case 0x70:
            num = 8.5;
            break;

        case 0x71:
            num = 8.55;
            break;

        case 0x72:
            num = 8.6;
            break;

        case 0x73:
            num = 8.65;
            break;

        case 0x74:
            num = 8.7;
            break;

        case 0x75:
            num = 8.75;
            break;

        case 0x76:
            num = 8.8;
            break;

        case 0x77:
            num = 8.85;
            break;

        case 120:
            num = 8.9;
            break;

        case 0x79:
            num = 8.95;
            break;

        case 0x7a:
            num = 9.01;
            break;

        case 0x7b:
            num = 9.06;
            break;

        case 0x7c:
            num = 9.11;
            break;

        case 0x7d:
            num = 9.16;
            break;

        case 0x7e:
            num = 9.21;
            break;

        case 0x7f:
            num = 9.26;
            break;

        case 0x80:
            num = 9.31;
            break;

        case 0x81:
            num = 9.36;
            break;

        case 130:
            num = 9.4;
            break;

        case 0x83:
            num = 9.46;
            break;

        case 0x84:
            num = 9.5;
            break;

        case 0x85:
            num = 9.55;
            break;

        case 0x86:
            num = 9.6;
            break;

        case 0x87:
            num = 9.65;
            break;

        case 0x88:
            num = 9.7;
            break;

        case 0x89:
            num = 9.75;
            break;

        case 0x8a:
            num = 9.79;
            break;

        case 0x8b:
            num = 9.84;
            break;

        case 140:
            num = 9.89;
            break;

        case 0x8d:
            num = 9.94;
            break;

        case 0x8e:
            num = 9.99;
            break;

        case 0x8f:
            num = 10.04;
            break;

        case 0x90:
            num = 10.09;
            break;

        case 0x91:
            num = 10.14;
            break;

        case 0x92:
            num = 10.19;
            break;

        case 0x93:
            num = 10.24;
            break;

        case 0x94:
            num = 10.29;
            break;

        case 0x95:
            num = 10.34;
            break;

        case 150:
            num = 10.39;
            break;

        case 0x97:
            num = 10.44;
            break;

        case 0x98:
            num = 10.49;
            break;

        case 0x99:
            num = 10.54;
            break;

        case 0x9a:
            num = 10.59;
            break;

        case 0x9b:
            num = 10.64;
            break;

        case 0x9c:
            num = 10.69;
            break;

        case 0x9d:
            num = 10.74;
            break;

        case 0x9e:
            num = 10.79;
            break;

        case 0x9f:
            num = 10.84;
            break;

        case 160:
            num = 10.88;
            break;

        case 0xa1:
            num = 10.93;
            break;

        case 0xa2:
            num = 10.98;
            break;

        case 0xa3:
            num = 11.02;
            break;

        case 0xa4:
            num = 11.07;
            break;

        case 0xa5:
            num = 11.11;
            break;

        case 0xa6:
            num = 11.16;
            break;

        case 0xa7:
            num = 11.2;
            break;

        case 0xa8:
            num = 11.24;
            break;

        case 0xa9:
            num = 11.28;
            break;

        case 170:
            num = 11.33;
            break;

        case 0xab:
            num = 11.37;
            break;

        case 0xac:
            num = 11.41;
            break;

        case 0xad:
            num = 11.45;
            break;

        case 0xae:
            num = 11.5;
            break;

        case 0xaf:
            num = 11.54;
            break;

        case 0xb0:
            num = 11.58;
            break;

        case 0xb1:
            num = 11.62;
            break;

        case 0xb2:
            num = 11.66;
            break;

        case 0xb3:
            num = 11.7;
            break;

        case 180:
            num = 11.75;
            break;

        case 0xb5:
            num = 11.79;
            break;

        case 0xb6:
            num = 11.83;
            break;

        case 0xb7:
            num = 11.89;
            break;

        case 0xb8:
            num = 11.92;
            break;

        case 0xb9:
            num = 11.96;
            break;

        case 0xba:
            num = 12.0;
            break;

        case 0xbb:
            num = 12.05;
            break;

        case 0xbc:
            num = 12.09;
            break;

        case 0xbd:
            num = 12.13;
            break;

        case 190:
            num = 12.18;
            break;

        case 0xbf:
            num = 12.22;
            break;

        case 0xc0:
            num = 12.27;
            break;

        case 0xc1:
            num = 12.31;
            break;

        case 0xc2:
            num = 12.35;
            break;

        case 0xc3:
            num = 12.4;
            break;

        case 0xc4:
            num = 12.44;
            break;

        case 0xc5:
            num = 12.49;
            break;

        case 0xc6:
            num = 12.53;
            break;

        case 0xc7:
            num = 12.58;
            break;

        case 200:
            num = 12.64;
            break;

        case 0xc9:
            num = 12.67;
            break;

        case 0xca:
            num = 12.72;
            break;

        case 0xcb:
            num = 12.76;
            break;

        case 0xcc:
            num = 12.81;
            break;

        case 0xcd:
            num = 12.86;
            break;

        case 0xce:
            num = 12.91;
            break;

        case 0xcf:
            num = 12.96;
            break;

        case 0xd0:
            num = 13.01;
            break;

        case 0xd1:
            num = 13.06;
            break;

        case 210:
            num = 13.11;
            break;

        case 0xd3:
            num = 13.17;
            break;

        case 0xd4:
            num = 13.22;
            break;

        case 0xd5:
            num = 13.27;
            break;

        case 0xd6:
            num = 13.33;
            break;

        case 0xd7:
            num = 13.38;
            break;

        case 0xd8:
            num = 13.44;
            break;

        case 0xd9:
            num = 13.5;
            break;

        case 0xda:
            num = 13.56;
            break;

        case 0xdb:
            num = 13.62;
            break;

        case 220:
            num = 13.68;
            break;

        case 0xdd:
            num = 13.74;
            break;

        case 0xde:
            num = 13.81;
            break;

        case 0xdf:
            num = 13.87;
            break;

        case 0xe0:
            num = 13.93;
            break;

        case 0xe1:
            num = 14.0;
            break;

        case 0xe2:
            num = 14.07;
            break;

        case 0xe3:
            num = 14.14;
            break;

        case 0xe4:
            num = 14.21;
            break;

        case 0xe5:
            num = 14.27;
            break;

        case 230:
            num = 14.35;
            break;

        case 0xe7:
            num = 14.42;
            break;

        case 0xe8:
            num = 14.5;
            break;

        case 0xe9:
            num = 14.58;
            break;

        case 0xea:
            num = 14.66;
            break;

        case 0xeb:
            num = 14.75;
            break;

        case 0xec:
            num = 14.84;
            break;

        case 0xed:
            num = 14.94;
            break;

        case 0xee:
            num = 15.06;
            break;

        case 0xef:
            num = 15.2;
            break;

        case 240:
            num = 15.4;
            break;

        case 0xf1:
            num = 15.67;
            break;

        case 0xf2:
            num = 15.96;
            break;

        case 0xf3:
            num = 16.2;
            break;

        case 0xf4:
            num = 16.45;
            break;

        case 0xf5:
            num = 16.75;
            break;

        case 0xf6:
            num = 17.08;
            break;

        case 0xf7:
            num = 17.54;
            break;

        case 0xf8:
            num = 18.12;
            break;

        case 0xf9:
            num = 18.8;
            break;

        case 250:
            num = 19.99;
            break;
    }
    return num;
}

//Type为0x02时按不同的Category分发消息
void processLayerStateAndMeasurementInfoMessageByCategory(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 4)
    {
        int iCategory = pTriorailMessage[4] & 7;
        RLOGD("processLayerStateAndMeasurementInfoMessageByCategory, iCategory=%d.", iCategory);  
        switch (iCategory)
        {
            case 0x00:
            case 0x01:
            case 0x02:
                break;
            case 0x03://此时为Trace消息
                processLayerStateAndMeasurementInfoMessageBySubTypes(pTraceFrame);
                break;
            case 0x04:
            case 0x05:
            case 0x06://Reserved
            case 0x07://Stored trace message,PC<-MS
                break;
            default: 
                RLOGE("processLayerStateAndMeasurementInfoMessageByCategory, err Category.");  
                break;            
        }
    }
}

//Layer State and Measurement Infomation, Trace Message
void processLayerStateAndMeasurementInfoMessageBySubTypes(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 6)
    {
        int iSubType = pTriorailMessage[5] & 0x1f;
        RLOGD("processLayerStateAndMeasurementInfoMessageBySubTypes, iSubType=%d.", iSubType);  
        switch (iSubType)
        {
            case 0x00://Layer 1 information
                processL1Info(pTraceFrame);
                break;
            case 0x01://Service state
                break;
            case 0x02://Reserved for future use
                break;
            case 0x03://MAC information
                break;
            case 0x04://RLC information
                break;
            case 0x05://RR information
                break;
            case 0x06://LLC information
                break;
            case 0x07://MM information
                break;
            case 0x08://GMM information
                break;
            case 0x09://SM information
                break;
            case 0x0A://SNDCP information
                break;
            case 0x1A://Layer 1 information with high resolution for RX LEV
                break;
            case 0x1B://Timing Information
                break;
            case 0x1C://ASCI-L1/RR/MM information
                break;
            case 0x1D://ASCI-GCC/BCC information
                break;
            case 0x1E://EGPRS information
                break;
            default: 
                RLOGE("processLayerStateAndMeasurementInfoMessageBySubTypes,  unknow subtype.");  
                break;            
        }
    }
}


void processL1Info(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 8)
    {
        int m_iMode = pTriorailMessage[6] & 0x0F;
        RLOGD("processL1Info, m_iMode=%d.", m_iMode);  
        switch (m_iMode)
        {
            case 0x00://Idle模式
                if (nLen >= 139)//此处应为139 STX(1)+AppID(1)+Length(2) + Identification(2) 
                // + FormatID(1)+ServingLen(1)+ServingContent(14)+NeighborLen(1)+6Neighbor(19*6)
                // + FCS(1) + ETX(1)
                {
                    processL1InfoIdle(pTraceFrame);
                }
                break;

            case 0x01://Dedicated模式 占用模式
                if (nLen >= 73)//此处应为73
                //此处应为73 STX(1)+AppID(1)+Length(2) + Identification(2) 
                // + FormatID(1)+ServingLen(1)+ServingContent(9)+NeighborLen(1)+6Neighbor(9*6)
                // + FCS(1) + ETX(1)
                {
                    processL1InfoDedicated(pTraceFrame);
                }
                break;
            case 0x02://Packet mode;  
                break;
            case 0x03://Additional NBs
                break;
            default:
                RLOGE("processL1Info, unknow mode.");
                break;
        }
    }
}


void processL1InfoDedicated(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    g_L1Msg.mode = 1;

    int index = 8;
    int servingCellLength = pTriorailMessage[7];
    int neighborCellLength = pTriorailMessage[index + servingCellLength];

    int vTATX = (int)pTriorailMessage[index] & 0x80;
    int vTXL = (int)pTriorailMessage[index] & 0x40;

    if ((vTATX != 0) && (vTXL!= 0))
    {
        //TA
        g_L1Msg.servingCellInfo.nTA = pTriorailMessage[index] & 0x3f;//00111111

        //服务小区电平值Full
        g_L1Msg.servingCellInfo.nRxLevel = pTriorailMessage[index + 2] & 0x3f;//01111111


        //Sub
        g_L1Msg.servingCellInfo.nRxLevel= pTriorailMessage[index + 3] & 0x3f;//01111111


        //FULL语音质量
        g_L1Msg.servingCellInfo.nRxQual = (pTriorailMessage[index + 4] & 0x70) >> 4;//01110000


        //Sub语音质量
        g_L1Msg.servingCellSubInfo.nRxQual = pTriorailMessage[index + 4] & 7;//00000111

        //BCCH
        int nBCCH = (pTriorailMessage[index + 6] & 3);//00000011
        nBCCH = (nBCCH * 0x100) + pTriorailMessage[index + 7];
        g_L1Msg.servingCellSubInfo.nBCCH = nBCCH;
        g_L1Msg.servingCellSubInfo.nBCCH = nBCCH;
        g_L1Msg.nServingBCCH = nBCCH;


        //解析出BSIC码
        int vBSIC = (pTriorailMessage[index + 8] & 0x40) >> 6;
        if (vBSIC==1)
        {
            int nNCC = (pTriorailMessage[index + 8] & 0x38) >> 3;
            int nBCC = pTriorailMessage[index + 8] & 7;
            g_L1Msg.servingCellInfo.nNCC = nNCC;
            g_L1Msg.servingCellSubInfo.nNCC = nNCC;
            g_L1Msg.servingCellInfo.nBCC = nBCC;
            g_L1Msg.servingCellSubInfo.nBCC = nBCC;
            
            g_L1Msg.nServingNcc = nNCC;
            g_L1Msg.nServingBcc = nBCC;
        }
        else
        {
            g_L1Msg.servingCellInfo.nNCC = g_L1Msg.nServingNcc;
            g_L1Msg.servingCellSubInfo.nNCC = g_L1Msg.nServingNcc;
            g_L1Msg.servingCellInfo.nBCC = g_L1Msg.nServingBcc;
            g_L1Msg.servingCellSubInfo.nBCC = g_L1Msg.nServingBcc;
        }


        FillNeighbourCellInfoDedicated(g_L1Msg.neighbour1CellInfo, pTriorailMessage, (index + servingCellLength) + 1);
        FillNeighbourCellInfoDedicated(g_L1Msg.neighbour2CellInfo, pTriorailMessage, ((index + servingCellLength) + 1) + neighborCellLength);
        FillNeighbourCellInfoDedicated(g_L1Msg.neighbour3CellInfo, pTriorailMessage, ((index + servingCellLength) + 1) + (neighborCellLength * 2));
        FillNeighbourCellInfoDedicated(g_L1Msg.neighbour4CellInfo, pTriorailMessage, ((index + servingCellLength) + 1) + (neighborCellLength * 3));
        FillNeighbourCellInfoDedicated(g_L1Msg.neighbour5CellInfo, pTriorailMessage, ((index + servingCellLength) + 1) + (neighborCellLength * 4));
        FillNeighbourCellInfoDedicated(g_L1Msg.neighbour6CellInfo, pTriorailMessage, ((index + servingCellLength) + 1) + (neighborCellLength * 5));

        WriteFile_L1Info(g_L1Msg);
    }
}

void FillNeighbourCellInfoDedicated(cellInfo & neighbourCellInfo, uint8_t *  pTriorailMessage, int nStartIndex)
{
    if ((pTriorailMessage[nStartIndex] & 0x80) && (pTriorailMessage[nStartIndex] & 0x40))
    {
        neighbourCellInfo.nNCC = (pTriorailMessage[nStartIndex] & 0x38) >> 3;
        neighbourCellInfo.nBCC = pTriorailMessage[nStartIndex] & 7;

        int nBCCH = pTriorailMessage[nStartIndex + 1] & 3;
        nBCCH = (nBCCH * 0x100) + pTriorailMessage[nStartIndex + 2];
        neighbourCellInfo.nBCCH = nBCCH;
        neighbourCellInfo.nRxLevel = pTriorailMessage[nStartIndex + 3] & 0x3f;
    }
}

    
void processL1InfoIdle(Trio_trace_frame * pTraceFrame)//解析层一消息，并且填充服务小区和6个邻小区的结构变量
{
    
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;
    
    int index = 8;
    int servingCellLength = pTriorailMessage[7];
    int neighborCellLength = pTriorailMessage[index + servingCellLength];
    int vBCCH = (int)pTriorailMessage[index] & 0x80;
    int vSCH = (int)pTriorailMessage[index] & 0x40;
    g_L1Msg.mode = 0;
	
    if ((vBCCH != 0) && (vSCH != 0))
    {
        //解析出BSIC码
        g_L1Msg.servingCellInfo.nNCC = (pTriorailMessage[index] & 0x38) >> 3;
        g_L1Msg.servingCellInfo.nBCC = pTriorailMessage[index] & 7;


        //解析出BCCH绝对频点号
        int nBCCH = pTriorailMessage[index + 1] & 3;
        nBCCH = (nBCCH * 0x100) + pTriorailMessage[index + 2];
        g_L1Msg.servingCellInfo.nBCCH = (uint16_t)nBCCH;

        //电平值
        g_L1Msg.servingCellInfo.nRxLevel = pTriorailMessage[index + 3] & 0x3f;

        //话音质量
        //pLayer1InfoPool.SaveRxQualFull(DateTime.Now, kmPost, 0xff);


        //解出C1
        int VC1 = (int)pTriorailMessage[index + 4] & 0x80;
        if (VC1 != 0)
        {
            int C1 = pTriorailMessage[index + 5];
            C1 = (C1 < 0x7f) ? C1 : (C1 - 0x100);
            g_L1Msg.servingCellInfo.nC1 = (int16_t)C1;
        }

        //解出C2
        int VC2 = (int)pTriorailMessage[index + 4] & 0x40;
        if (VC2 != 0)
        {
            int C2 = pTriorailMessage[index + 6];
            C2 = (C2 < 0x7f) ? C2 : (C2 - 0x100);
            g_L1Msg.servingCellInfo.nC2 = (int16_t)C2;
        }

        //填充6个邻小区对应的结构变量
        FillNeighbourCellInfoIdle(g_L1Msg.neighbour1CellInfo, pTriorailMessage, (index + servingCellLength) + 1);
        FillNeighbourCellInfoIdle(g_L1Msg.neighbour2CellInfo, pTriorailMessage, ((index + servingCellLength) + 1) + neighborCellLength);
        FillNeighbourCellInfoIdle(g_L1Msg.neighbour3CellInfo, pTriorailMessage, ((index + servingCellLength) + 1) + (neighborCellLength * 2));
        FillNeighbourCellInfoIdle(g_L1Msg.neighbour4CellInfo, pTriorailMessage, ((index + servingCellLength) + 1) + (neighborCellLength * 3));
        FillNeighbourCellInfoIdle(g_L1Msg.neighbour5CellInfo, pTriorailMessage, ((index + servingCellLength) + 1) + (neighborCellLength * 4));
        FillNeighbourCellInfoIdle(g_L1Msg.neighbour6CellInfo, pTriorailMessage, ((index + servingCellLength) + 1) + (neighborCellLength * 5));

        g_L1Msg.timestamp = pTraceFrame->timestamp;
        g_L1Msg.KmPosition = pTraceFrame->gps_data.KmPosition;
        g_L1Msg.Speed= pTraceFrame->gps_data.Speed;
        g_L1Msg.latitude= pTraceFrame->gps_data.latitude;
        g_L1Msg.longitude= pTraceFrame->gps_data.longitude;
        g_L1Msg.direction= pTraceFrame->gps_data.direction;

        WriteFile_L1Info(g_L1Msg);
    }
}

    
void FillNeighbourCellInfoIdle(cellInfo & neighbourCellInfo, uint8_t * pTriorailMessage, int nStartIndex)
{
    if ((pTriorailMessage[nStartIndex] & 0x80) && (pTriorailMessage[nStartIndex] & 0x40))
    {
        neighbourCellInfo.nNCC = (pTriorailMessage[nStartIndex] & 0x38) >> 3;
        neighbourCellInfo.nBCC = pTriorailMessage[nStartIndex] & 7;

        int nBCCH = pTriorailMessage[nStartIndex + 1] & 3;
        nBCCH = (nBCCH * 0x100) + pTriorailMessage[nStartIndex + 2];
        neighbourCellInfo.nBCCH = (uint16_t)nBCCH;

        neighbourCellInfo.nRxLevel = pTriorailMessage[nStartIndex + 3] & 0x3f;

        if (pTriorailMessage[nStartIndex + 4] & 0x80)//字节的最高位VC1
        {
            int C1 = pTriorailMessage[nStartIndex + 5];
            C1 = (C1 < 0x7f) ? C1 : (C1 - 0x100);
            neighbourCellInfo.nC1 = (uint16_t)C1;
        }

        if (pTriorailMessage[nStartIndex + 4] & 0x40)//字节的次高位VC2
        {
            int C2 = pTriorailMessage[nStartIndex + 6];
            C2 = (C2 < 0x7f) ? C2 : (C2 - 0x100);
            neighbourCellInfo.nC1 = (uint16_t)C2;
        }
    }
}

//Type为0x03时按不同的Category分发消息
void processForcingMessageByCategory(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 4)
    {
        int iCategory = pTriorailMessage[4] & 7;
        switch (iCategory)
        {
            case 0x00:
            case 0x01:
            case 0x02:
                break;
            case 0x03://此时为Trace消息
                processForcingMessageBySubTypes(pTraceFrame);
                break;
            case 0x04:
            case 0x05:
            case 0x06://Reserved
            case 0x07://Stored trace message,PC<-MS
                break;
            default: 
                RLOGE("processForcingMessageByCategory, err iCategory.");  
                break;
        }
    }
}
void processForcingMessageBySubTypes(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 6)
    {
        int iSubType = pTriorailMessage[5] & 0x1f;
        switch (iSubType)
        {
            case 0x00://Cell reselection forcing
                break;
            case 0x01://Suppressing cell reselection
                break;
            case 0x02://GPRS Multi-Slot class forcing
                break;
            case 0x03://Downlink Coding Scheme forcing
                break;
            case 0x04://Reserved for future use
                break;
            case 0x05://Forcing a BCCH in Idle mode
                break;
            case 0x06://Forcing a Handover
                break;
            case 0x07://Suppressing a Handover
                break;
            case 0x08://Modifying cell barring access list
                break;
            case 0x09://Overriding Path Loss
                break;
            case 0x0A://Power classes forcing
                break;
            case 0x0B://Full Rate forcing
                break;
            case 0x0C://Frequency band forcing
                break;
            case 0x0D://Localisation Updating forcing
                break;
            case 0x0E://Modifying L3 Uplink frames
                break;
            case 0x0F://Set GPRS auto-attach
                break;
            ////////////////////////////////
            case 0x10://MS GPRS class forcing
                break;
            case 0x11://Layer message forcing
                break;
            case 0x12://AMR Disable Class forcing AMR disable forcing
                break;
            case 0x13://Preferred voice codec forcing
                break;
            case 0x14://SMS push capture
                break;
            case 0x16://Feature Enable forcing
                break;
            case 0x17://Reattach forcing
                break;
            case 0x18://Dedicated Timeslot forcing
                break;
            case 0x19://Feature Disable forcing
                break;
            case 0x1A://Allowed Channel forcing
                break;
            case 0x1B://ASCI Group ID forcing
                break;
            case 0x1C://Mobile shut down forcing
                break;
            case 0x1D://GPRS Uplink coding scheme forcing
                break;
            case 0x1E://EGPRS Uplink coding scheme forcing
                break;
            default: 
                RLOGE("processForcingMessageByCategory, err iCategory.");  
                break;        }
    }
}

//Type为0x04时按不同的Category分发消息
void processMobileInfoMessageByCategory(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 4)
    {
        int iCategory = pTriorailMessage[4] & 7;
        switch (iCategory)
        {
            case 0x00:
            case 0x01:
            case 0x02:
                break;
            case 0x03://此时为Trace消息
                processMobileInfoMessageBySubTypes(pTraceFrame);
                break;
            case 0x04:
            case 0x05:
            case 0x06://Reserved
            case 0x07://Stored trace message,PC<-MS
                break;
            default: 
                break;

        }
    }
}

void processMobileInfoMessageBySubTypes(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 6)
    {
        int iSubType = pTriorailMessage[5] & 0x1f;
        switch (iSubType)
        {
            case 0x00://Product Name
                break;
            case 0x01://IMEI
                break;
            case 0x02://Software Version
                break;
            case 0x03://Protocol Version
                break;
            case 0x04://MS GPRS Class
                break;
            case 0x05://Mobile capabilities
                break;
            case 0x06://Suported Mask Bits
                break;
            default:
                break;
        }
    }
}

//Type为0x05时按不同的Category分发消息
void processControlMessageByCategory(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 4)
    {
        int iCategory = pTriorailMessage[4] & 7;
        switch (iCategory)
        {
            case 0x00:
            case 0x01:
            case 0x02:
                break;
            case 0x03://此时为Trace消息
                processControlMessageBySubTypes(pTraceFrame);
                break;
            case 0x04:
            case 0x05:
            case 0x06://Reserved
            case 0x07://Stored trace message,PC<-MS
                break;
            default: 
                break;
        }
    }
}

void processControlMessageBySubTypes(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 6)
    {
        int iSubType = pTriorailMessage[5] & 0x1f;
        switch (iSubType)
        {
            case 0x00://Serial Link Mode Setting
                break;
            case 0x01://TTS Mode Setting
                break;
            default:
                break;
        }
    }
}

//Type为0x06时按不同的Category分发消息
void processTraceStorageMessageByCategory(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 4)
    {
        int iCategory = pTriorailMessage[4] & 7;
        switch (iCategory)
        {
            case 0x00:
            case 0x01:
            case 0x02:
                break;
            case 0x03://此时为Trace消息
                processTraceStorageMessageBySubTypes(pTraceFrame);
                break;
            case 0x04:
            case 0x05:
            case 0x06://Reserved
            case 0x07://Stored trace message,PC<-MS
                break;
            default:
                break;

        }
    }
}


void processTraceStorageMessageBySubTypes(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 6)
    {
        int iSubType = pTriorailMessage[5] & 0x1f;
        switch (iSubType)
        {
            case 0x01://Trace recording
                break;
            case 0x06://Marker
                break;
            case 0x0F://Recording on startup
                break;
            case 0x10://Layer Messages trace
                break;
            case 0x11://Quality of Service trace
                break;
            case 0x12://Layer state trace
                break;
            case 0x13://Scanning
                break;
            case 0x1D://GPS Data
                break;
            case 0x1F://Error
                break;
            default: 
                break;
        }
    }
}

//Type为0x07时按照不同的Category分发消息
void processForcingMessage1ByCategory(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 4)
    {
        int iCategory = pTriorailMessage[4] & 7;
        switch (iCategory)
        {
            case 0x00:
                break;
            case 0x01:
                break;
            case 0x02:
                break;
            case 0x03://MS->PC trace
                processForcingMessage1BySubTypes(pTraceFrame);
                break;
            case 0x04:
            case 0x05:
            case 0x06://Reserved
            case 0x07://Stored trace message,PC<-MS
                break;
            default: 
                break;
        }
    }
}
    
void processForcingMessage1BySubTypes(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 6)
    {
        int iSubType = pTriorailMessage[5] & 0x1f;
        switch (iSubType)
        {
            case 0x00://Uplink Bler forcing
                break;
            case 0x01://Downlink Bler forcing
                break;
            case 0x02://Limited Fequency forcing
                break;
            default: 
                break;
        }
    }
}

void processGPSDataByCategory(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 4)
    {
        int iCategory = pTriorailMessage[4] & 7;
        switch (iCategory)
        {
            case 0x00:
            case 0x01:
            case 0x02:
                break;
            case 0x03://
                processTraceGPSData(pTraceFrame);
                break;
            case 0x04:
            case 0x05:
            case 0x06://Reserved
            case 0x07://Stored trace message,PC<-MS
                break;
            default: 
                break;
        }
    }
}

       
void processTraceGPSData(Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    int nLen = pTraceFrame->dataLen;

    if (nLen >= 6)
    {
        int iSubType = pTriorailMessage[5] & 0x1f;
        switch (iSubType)
        {
            case 0x00://GPS status messages
                break;
            case 0x01://GPS data
                break;
            case 0x1E://GPS debug outputs
                break;
            default:
                break;
        }
    }
}





