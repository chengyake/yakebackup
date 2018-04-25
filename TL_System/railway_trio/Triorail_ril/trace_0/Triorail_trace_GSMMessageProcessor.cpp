
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>
#include <string.h>

#include <Triorail_trace_msg.h>
#include <Triorail_trace.h>
#include <utils.h>
#include <Triorail_trace_internal.h>

#define LOG_TAG "TRIO_TRACE_GSMMsg"
#include <log.h>


#define GSM_GET_HANDOVER_FORM_L2

GSMUmParam g_GSMUmParam;
static bool s_GSMUmParam_init = false;

static bool s_bStartHandover = false;
static Handover s_Handover;


void GSMUmParamClear(GSMUmParam & UmParam)
{

    UmParam.m_wCell_Identity = -1;

    UmParam.m_LAI.nMCC = -1;
    UmParam.m_LAI.nMNC = -1;
    UmParam.m_LAI.nLAC = -1;


    UmParam.m_CtlCHDesc.bACC = false;
    UmParam.m_CtlCHDesc.nBS_AG_BLKS_RES = -1;
    UmParam.m_CtlCHDesc.nBS_PA_MFRMS = -1;
    UmParam.m_CtlCHDesc.nCCCH_CONF = -1;
    UmParam.m_CtlCHDesc.nT3212 = -1;

    UmParam.m_RachCtlParam.bRE = false;
    UmParam.m_RachCtlParam.bCELL_BAR_ACCESS = false;
    UmParam.m_RachCtlParam.nMax_Retrans = -1;
    UmParam.m_RachCtlParam.nTx_integer = -1;
    for (int i = 0; i < 16; i++)
    {
        UmParam.m_RachCtlParam.aAC[i] = false;
    }

    UmParam.m_CellSelectParameters.nCELL_RESELECT_HYSTERESIS = -1;
    UmParam.m_CellSelectParameters.nMS_TXPWR_MAX_CCH = -1;
    UmParam.m_CellSelectParameters.nLevel_Access_Min = -1;
    UmParam.m_CellSelectParameters.nACS = -1;
    UmParam.m_CellSelectParameters.bNECI = false;

    UmParam.m_CellDescriptionSACCH.nDTX = -1;
    UmParam.m_CellDescriptionSACCH.nPower_Control_Indicator = -1;
    UmParam.m_CellDescriptionSACCH.nRadio_Link_Timeout = -1;
}

void SetRachCtlParam(GSMUmParam & UmParam, RACH_Control_Param & rachCtlParam)
{
    UmParam.m_RachCtlParam.bCELL_BAR_ACCESS = rachCtlParam.bCELL_BAR_ACCESS;
    UmParam.m_RachCtlParam.bEC = rachCtlParam.bEC;
    UmParam.m_RachCtlParam.bRE = rachCtlParam.bRE;
    UmParam.m_RachCtlParam.nMax_Retrans = rachCtlParam.nMax_Retrans;
    UmParam.m_RachCtlParam.nTx_integer = rachCtlParam.nTx_integer;
    for (int i = 0; i < 16; i++)
    {
        UmParam.m_RachCtlParam.aAC[i] = rachCtlParam.aAC[i];
    }
}

void SetCtlCHDesc(GSMUmParam & UmParam,  Control_Channel_Description & ctlCHDesc)
{
    UmParam.m_CtlCHDesc.bACC = ctlCHDesc.bACC;
    UmParam.m_CtlCHDesc.nBS_AG_BLKS_RES = ctlCHDesc.nBS_AG_BLKS_RES;
    UmParam.m_CtlCHDesc.nBS_PA_MFRMS = ctlCHDesc.nBS_PA_MFRMS;
    UmParam.m_CtlCHDesc.nCCCH_CONF = ctlCHDesc.nCCCH_CONF;
    UmParam.m_CtlCHDesc.nT3212 = ctlCHDesc.nT3212;
}

void SetLAI(GSMUmParam & UmParam, Location_Area_Identification & LAI)
{
    UmParam.m_LAI.nMCC = LAI.nMCC;
    UmParam.m_LAI.nMNC = LAI.nMNC;
    UmParam.m_LAI.nLAC = LAI.nLAC;
}

void SetCellDescriptionSACCH(GSMUmParam & UmParam, Cell_Description_SACCH cellDesSACCH)
{
    UmParam.m_CellDescriptionSACCH.nDTX = cellDesSACCH.nDTX;
    UmParam.m_CellDescriptionSACCH.nPower_Control_Indicator = cellDesSACCH.nPower_Control_Indicator;
    UmParam.m_CellDescriptionSACCH.nRadio_Link_Timeout = cellDesSACCH.nRadio_Link_Timeout;
}

void SetCellSelectParameters(GSMUmParam & UmParam, Cell_Select_Parameters cellSelectPara)
{
    UmParam.m_CellSelectParameters.nCELL_RESELECT_HYSTERESIS = cellSelectPara.nCELL_RESELECT_HYSTERESIS;
    UmParam.m_CellSelectParameters.nMS_TXPWR_MAX_CCH = cellSelectPara.nMS_TXPWR_MAX_CCH;
    UmParam.m_CellSelectParameters.nACS = cellSelectPara.nACS;
    UmParam.m_CellSelectParameters.bNECI = cellSelectPara.bNECI;
    UmParam.m_CellSelectParameters.nLevel_Access_Min = cellSelectPara.nLevel_Access_Min;
}




void DecodeCtlCHDesc(uint8_t *  pCode, int n, Control_Channel_Description & ctlCHDesc)
{
    //IMSI结合分离允许
    if (pCode[n] & 0x40)
    {
        ctlCHDesc.bACC = true;
    }
    else
    {
        ctlCHDesc.bACC = false;
    }

    ctlCHDesc.nBS_AG_BLKS_RES = (pCode[n] & 0x38) >> 3;
    ctlCHDesc.nCCCH_CONF = pCode[n] & 7;
    ctlCHDesc.nBS_PA_MFRMS = (pCode[n + 1] & 7) + 2;
    ctlCHDesc.nT3212 = pCode[n + 2];
}

void DecodeLAI(uint8_t * pCode, int n, Location_Area_Identification & LAI)
{
    LAI.nMCC = (((pCode[n] & 15) * 100) + (((pCode[n] & 240) >> 4) * 10)) + (pCode[n + 1] & 15);
    LAI.nMNC = ((pCode[n + 2] & 15) * 10) + ((pCode[n + 2] & 240) >> 4);
    LAI.nLAC = (pCode[n + 3] * 0x100) + pCode[n + 4];
}

void DecodeRACHCtlParam(uint8_t * pCode, int n, RACH_Control_Param & rachCtlparam)
{
    //最大重传次数
    switch (((pCode[n] & 0xC0) >> 6))
    {
        case 0:
            rachCtlparam.nMax_Retrans = 1;
            break;

        case 1:
            rachCtlparam.nMax_Retrans = 2;
            break;

        case 2:
            rachCtlparam.nMax_Retrans = 4;
            break;

        case 3:
            rachCtlparam.nMax_Retrans = 7;
            break;
        default: 
            RLOGE("DecodeRACHCtlParam, nMax_Retrans default.");  
            break;
    }

    //发送分布时隙数
    int iTxinteger = (pCode[n] & 0x3C) >> 2;
    if (iTxinteger <= 9)
    {
        rachCtlparam.nTx_integer = iTxinteger + 3;
    }
    else
    {
        switch (iTxinteger)
        {
            case 10:
                rachCtlparam.nTx_integer = 14;
                break;

            case 11:
                rachCtlparam.nTx_integer = 16;
                break;

            case 12:
                rachCtlparam.nTx_integer = 20;
                break;

            case 13:
                rachCtlparam.nTx_integer = 25;
                break;

            case 14:
                rachCtlparam.nTx_integer = 32;
                break;

            case 15:
                rachCtlparam.nTx_integer = 50;
                break;
            default: 
                RLOGE("DecodeRACHCtlParam, iTxinteger default.");  
                break;
        }
    }

    //小区禁止接入, 0允许 1禁止
    rachCtlparam.bCELL_BAR_ACCESS = pCode[n] & 0x02;

    //重建链接允许 0允许 1禁止
    rachCtlparam.bRE = pCode[n] & 0x01;

    //紧急呼叫允许 0 所有用户允许 1 仅接入等级11-15级用户允许
    rachCtlparam.bEC = pCode[n + 1] & 0x04;

    //接入等级控制
    for (int i = 8; i < 16; i++)
    {
        rachCtlparam.aAC[i] = (pCode[n + 1] >> (i - 8)) & 0x01;
    }

    for (int i = 0; i < 8; i++)
    {
        rachCtlparam.aAC[i] = (pCode[n + 2] >> i) & 0x01;
    }
}

void DecodeCellSelectParam(uint8_t * pCode, int n, Cell_Select_Parameters & cellSelectParam)
{
    cellSelectParam.nCELL_RESELECT_HYSTERESIS = (int32_t)(((pCode[n] & 0xE0) >> 5) * 2);
    cellSelectParam.nMS_TXPWR_MAX_CCH = pCode[n] & 0x1F;
    cellSelectParam.nACS = (pCode[n + 1] & 0x80) >> 7;
    cellSelectParam.bNECI = ((pCode[n + 1] & 0x40) >> 6) == 1 ? true : false;
    cellSelectParam.nLevel_Access_Min = pCode[n + 1] & 0x3F;
}

void DecodeCellDescSACCH(uint8_t * pCode, int n, Cell_Description_SACCH & cellDescSACCH)
{
    cellDescSACCH.nDTX = ((pCode[n] & 0x80) >> 5) + ((pCode[n] & 0x30) >> 4);
    cellDescSACCH.nPower_Control_Indicator = pCode[n] & 0x40;
    cellDescSACCH.nRadio_Link_Timeout = ((pCode[n] & 0x0F) + 1) * 4;
}

void DecodeSystemInfo1(const Trio_trace_frame * pTraceFrame)
{
    if (pTraceFrame != NULL)
    {
        uint8_t * pTriorailMessage = pTraceFrame->data;
        uint8_t * pMsgCode = pTriorailMessage + 9;
        int nLen = (0x100 * pTriorailMessage[7]) + pTriorailMessage[8];

        if (nLen == 0x16)
        {
            RACH_Control_Param rachCtlparam;
            DecodeRACHCtlParam(pMsgCode, 18, rachCtlparam);
            SetRachCtlParam(g_GSMUmParam, rachCtlparam);
        }
    }
}

void DecodeSystemInfo2(const Trio_trace_frame * pTraceFrame)
{
    if (pTraceFrame != NULL)
    {
        uint8_t * pTriorailMessage = pTraceFrame->data;
        uint8_t * pMsgCode = pTriorailMessage + 9;
        int nLen = (0x100 * pTriorailMessage[7]) + pTriorailMessage[8];

        if (nLen == 0x16)
        {
            RACH_Control_Param rachCtlparam;
            DecodeRACHCtlParam(pMsgCode, 19, rachCtlparam);
            SetRachCtlParam(g_GSMUmParam, rachCtlparam);        
        }
    }
}

void DecodeSystemInfo2bis(const Trio_trace_frame * pTraceFrame)
{
    if (pTraceFrame != NULL)
    {
        uint8_t * pTriorailMessage = pTraceFrame->data;
        uint8_t * pMsgCode = pTriorailMessage + 9;
        int nLen = (0x100 * pTriorailMessage[7]) + pTriorailMessage[8];

        if (nLen == 0x16)
        {
            RACH_Control_Param rachCtlparam;
            DecodeRACHCtlParam(pMsgCode, 18, rachCtlparam);
            SetRachCtlParam(g_GSMUmParam, rachCtlparam);
        }
    }
}

void DecodeSystemInfo3(const Trio_trace_frame * pTraceFrame)
{
    if (pTraceFrame != NULL)
    {
        uint8_t * pTriorailMessage = pTraceFrame->data;
        uint8_t * pMsgCode = pTriorailMessage + 9;
        int nLen = (0x100 * pTriorailMessage[7]) + pTriorailMessage[8];
        
        if (nLen == 0x16)
        {
            //2、3两个字节
            g_GSMUmParam.m_wCell_Identity = (pMsgCode[2] * 0x100) + pMsgCode[3];

            //4、5、6、7、8五个字节
            Location_Area_Identification lAI;
            DecodeLAI(pMsgCode, 4,  lAI);
            SetLAI(g_GSMUmParam, lAI);

            //9、10、11三个字节
            Control_Channel_Description ctlCHDesc;
            DecodeCtlCHDesc(pMsgCode, 9, ctlCHDesc);
            SetCtlCHDesc(g_GSMUmParam, ctlCHDesc);

            //13、14两个字节
            Cell_Select_Parameters cellSelectParam;
            DecodeCellSelectParam(pMsgCode, 13, cellSelectParam);
            SetCellSelectParameters(g_GSMUmParam, cellSelectParam);

            //15、16两个字节
            RACH_Control_Param rachCtlparam;
            DecodeRACHCtlParam(pMsgCode, 15, rachCtlparam);
            SetRachCtlParam(g_GSMUmParam, rachCtlparam);
        }
    }
}

void DecodeSystemInfo4(const Trio_trace_frame * pTraceFrame)
{
    if (pTraceFrame != NULL)
    {
        uint8_t * pTriorailMessage = pTraceFrame->data;
        uint8_t * pMsgCode = pTriorailMessage + 9;
        int nLen = (0x100 * pTriorailMessage[7]) + pTriorailMessage[8];

        Location_Area_Identification lAI;
        DecodeLAI(pMsgCode, 2,  lAI);
        SetLAI(g_GSMUmParam, lAI);


        Cell_Select_Parameters cellSelectParam;
        DecodeCellSelectParam(pMsgCode, 7, cellSelectParam);
        SetCellSelectParameters(g_GSMUmParam, cellSelectParam);

        RACH_Control_Param rachCtlparam;
        DecodeRACHCtlParam(pMsgCode, 9, rachCtlparam);
        SetRachCtlParam(g_GSMUmParam, rachCtlparam);
    }
}

void DecodeSystemInfo6(const Trio_trace_frame * pTraceFrame)
{
    if (pTraceFrame != NULL)
    {
        uint8_t * pTriorailMessage = pTraceFrame->data;
        uint8_t * pMsgCode = pTriorailMessage + 9;
        int nLen = (0x100 * pTriorailMessage[7]) + pTriorailMessage[8];

        if (nLen == 0x12)
        {
            g_GSMUmParam.m_wCell_Identity = (pMsgCode[2] * 0x100) + pMsgCode[3];

            Location_Area_Identification lAI;
            DecodeLAI(pMsgCode, 4, lAI);
            SetLAI(g_GSMUmParam, lAI);

            Cell_Description_SACCH cellDescSACCH;
            DecodeCellDescSACCH(pMsgCode, 9, cellDescSACCH);
            SetCellDescriptionSACCH(g_GSMUmParam, cellDescSACCH);
        }
    }
}

void DecodeSystemInfo9(const Trio_trace_frame * pTraceFrame)
{
    uint8_t * pTriorailMessage = pTraceFrame->data;
    uint8_t * pMsgCode = pTriorailMessage + 9;
    int nLen = (0x100 * pTriorailMessage[7]) + pTriorailMessage[8];
    if (pTraceFrame != NULL)
    {
        if (nLen == 0x12)
        {
            RACH_Control_Param rachCtlparam;
            DecodeRACHCtlParam(pMsgCode, 9, rachCtlparam);
            SetRachCtlParam(g_GSMUmParam, rachCtlparam);
        }
    }
}

void DecordCCSetup(const Trio_trace_frame * pTraceFrame)
{
#if 0
    if ((m_nSpeechCallStatus == 1) && (m_pSpeechCall != null))
    {
        TriorailL3Message triorailRmm2320L3Msg = (TriorailL3Message)m_pTriorailL3MsgPool.GetMessageByID((long)MessageID);
        if (triorailRmm2320L3Msg != null)
        {
            byte[] pMsgCode = new byte[100];
            int iMsgLen = triorailRmm2320L3Msg.GetMessageCode(ref pMsgCode);
            int index = 0;
            while (index < iMsgLen)
            {
                if (pMsgCode[index] == 0x5e)
                {
                    break;
                }
                index++;
            }
            int num3 = pMsgCode[index + 1];
            if ((num3 > 2) && (((index + num3) + 2) <= iMsgLen))
            {
                int num4;
                int num5;
                string strCallNum = "";
                string str2 = "";
                for (int i = index + 3; i < ((index + num3) + 1); i++)
                {
                    num4 = (pMsgCode[i] & 240) >> 4;
                    num5 = pMsgCode[i] & 15;
                    if ((num4 > 9) || (num5 > 9))
                    {
                        return;
                    }
                    str2 = num5.ToString() + num4.ToString();
                    strCallNum = strCallNum + str2;
                }
                num4 = (pMsgCode[(index + num3) + 1] & 240) >> 4;
                num5 = pMsgCode[(index + num3) + 1] & 15;
                if (((num4 <= 9) || (num4 == 15)) && (num5 <= 9))
                {
                    if (num4 != 15)
                    {
                        str2 = num5.ToString() + num4.ToString();
                    }
                    else
                    {
                        str2 = num5.ToString();
                    }
                    strCallNum = strCallNum + str2;
                    m_pSpeechCall.SetCalledNum(strCallNum);
                }
            }
        }
    }
#endif    
}


void ProcessCCMessage(int64_t MessageTypeCode, const Trio_trace_frame * pTraceFrame)
{
    int64_t iMessageTypeCode = MessageTypeCode;

    if ((iMessageTypeCode <= 0x3033eL) && (iMessageTypeCode >= 0x30301L))
    {
        int iMessageType = (int)(iMessageTypeCode - 0x30300L);
        switch (iMessageType)
        {
            //以下12个为Call Establishment messages
            case 1://Alerting
            case 2://Call Proceeding
            case 3://Progress
                break;
            case 4://连接建立消息
                DecordCCSetup(pTraceFrame);
                break;
            case 5://Setup
                break;
            case 6://连接建立确认消息

                break;
            case 7://Connect
                //updateCCConnectMessageList(pMSMsg.GetKmPost());
                break;
            case 8://Call Confirmed
            case 9://Start CC
            case 11://Recall
            case 14://Emergency Setup
            case 15://Connect Acknowledge
                break;
            
            //以下10个为Call information phase messages
            case 0x10://User Information
            case 0x13://Modify Reject
            case 0x17://Modify
            case 0x18://Hold
            case 0x19://Hold Acknowledge
            case 0x1a://Hold Reject
            case 0x1c://Retrieve
            case 0x1d://Retreive Acknowledge
            case 0x1E://Retrieve Reject
            case 0x1f://Modify Complete
                break;

            //以下三个Call clearing message 呼叫清除消息
            case 0x25://Disconnect
            case 0x2a://Release Complete
            case 0x2d://Release
                //updateCCReleaseMessageList(pMSMsg.GetKmPost());
                break;
            
            //以下10个为Miscellaneous messages
            case 0x31://Stop DTMF
            case 0x32://Stop DTMF AckNowledge
            case 0x34://Status Enquiry
            case 0x35://Start DTMF
            case 0x36://Start DTMF Acknowledge
            case 0x37://Start DTMF reject
            case 0x39://Congestion Control
            case 0x3a://Facility
            case 0x3d://Status
            case 0x3e://Notify
                break;
            default: 
                RLOGE("ProcessCCMessage, default.");
                break;
        }
    }
}

void ProcessGPRSMMMessage(int64_t MessageTypeCode, const Trio_trace_frame * pTraceFrame)
{
    long num = MessageTypeCode;
    if ((num <= 0x30821L) && (num >= 0x30801L))
    {
        switch (((int) (num - 0x30801L)))
        {
            default: 
                RLOGE("ProcessGPRSMMMessage default."); 
                break;
        }
    }
}

void ProcessGPRSSMMessage(int64_t MessageTypeCode, const Trio_trace_frame * pTraceFrame)
{
    long num = MessageTypeCode;
    if ((num <= 0x30a55L) && (num >= 0x30a41L))
    {
        switch (((int) (num - 0x30a41L)))
        {
            default: 
                RLOGE("ProcessGPRSSMMessage default."); 
                break;
        }
    }
}

//处理移动性管理的消息信令
void ProcessMMMessage(int64_t MessageTypeCode, const Trio_trace_frame * pTraceFrame)
{
    long num = MessageTypeCode;
    if (num <= 0x30504L)
    {
        if (num < 0x30501L)
        {
            return;
        }
        switch (((int) (num - 0x30501L)))
        {
            case 0:
            case 1:
            case 2:
            case 3:
                return;
            default: 
                RLOGE("ProcessMMMessage default.");  
                break;
        }
    }
    if (((num != 0x30508L) && (num <= 0x30532L)) && (num >= 0x30511L))
    {
        switch (((int) (num - 0x30511L)))
        {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 20:
            case 0x15:
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x19:
            case 0x1a:
            case 0x1b:
            case 0x1c:
            case 0x1d:
            case 30:
            case 0x1f:
            case 0x20:
            case 0x21:
                return;
            default: 
                RLOGE("ProcessMMMessage default."); 
                break;
        }
    }
}
/// <summary>
/// 处理无线层消息
/// </summary>
/// <param name="MessageTypeCode"></param>
/// <param name="MessageID"></param>
/// <param name="TimeStap"></param>
void ProcessRRMessage(int64_t MessageTypeCode, const Trio_trace_frame * pTraceFrame)
{

    int64_t iMessageTypeCode = MessageTypeCode;
    uint8_t * pTriorailMessage = pTraceFrame->data;

    const uint8_t * gsmFrame = pTriorailMessage + 9;
    int iMsgCodeLen =(0x100 * pTriorailMessage[7]) + pTriorailMessage[8];
    RLOGI("ProcessRRMessage.");

    if ((iMessageTypeCode <= 0x3064eL) && (iMessageTypeCode >= 0x30600L))
    {
        int iMessageType = (int)(iMessageTypeCode - 0x30600L);
        switch (iMessageType)//MessageType
        {
            case 0://系统信息类型13
            case 1:
                break;
            case 2://系统信息类型2bis
                DecodeSystemInfo2bis(pTraceFrame);
                break;
            case 3://系统信息类型2ter
                break;
            case 4://系统消息类型9
                DecodeSystemInfo9(pTraceFrame);
                break;
            case 5://系统消息类型5bis
                break;
            case 6://系统信息类型5ter
                break;
            case 7://
            case 8://RR-Cell Change Order
            case 9://VGCS上行链路 Grant
            case 10://Partial Release
            case 11://Reserved
            case 12://Uplink Free for VGCS
                break;

            case 13://信道释放消息
                //Todo: for call state.
                //triorailFrame.CallClearTimeOutTimer.Enabled = false;
                //UpdateDisconnect(pTraceFrame);
                break;
            case 14://Uplink Release for VGCS
            case 15://Partial Release Complete
            case 0x10://信道模式改变
            case 0x11://Talker Indication
            case 0x12://RR Status
            case 0x13://Classmark Enquiry
            case 0x14://Frequency Redefinition
            case 0x15://Measurement Report
            case 0x16://Classmark Change
            case 0x17://信道模式改变确认Channel Mode Modify Acknowledge
            case 0x18://系统信息类型8
                break;
            case 0x19://系统信息类型1
                DecodeSystemInfo1(pTraceFrame);
                break;
            case 0x1a://系统信息类型2
                DecodeSystemInfo2(pTraceFrame);
                break;
            case 0x1b://收到系统消息类型3
                DecodeSystemInfo3(pTraceFrame);
                break;
            case 0x1c://系统信息类型4
                DecodeSystemInfo4(pTraceFrame);
                break;
            case 0x1d://系统信息类型5
                break;
            case 30://系统消息类型6
                DecodeSystemInfo6(pTraceFrame);
                break;
            case 0x1f://系统信息类型7
            case 0x20://Notification/NCH
            case 0x21://Paging Request Type 1
            case 0x22://Paging Request Type 2
            case 0x23://PDCH 指派命令
            case 0x24://Paging Request Type 3
            case 0x25://Notification/FACCH
            case 0x26://Notification Response
            case 0x27://Paging Response
                break;
            case 0x28://切换失败
#ifdef GSM_GET_HANDOVER_FORM_L3
                if (s_bStartHandover)
                {
                    s_bStartHandover = false;
                    
                    s_Handover.HandOverEndTime = getRealtimeOfCS();
                    s_Handover.HandOverEndTimestamp= pTraceFrame->timestamp;
                    s_Handover.nResult = 0;
                    s_Handover.Speed_End = pTraceFrame->gps_data.Speed;
                    s_Handover.nKmPost_End = pTraceFrame->gps_data.KmPosition;
                    s_Handover.Latitude_End = pTraceFrame->gps_data.latitude;
                    s_Handover.Longitude_End = pTraceFrame->gps_data.longitude;
                    
                    WriteFile_handover(s_Handover);
                    memset(&s_Handover, 0, sizeof(Handover));
                }
#endif                
                break;
            case 0x29://指派完成
            case 0x2a://Uplink Busy for VGCS
                break;
            case 0x2b://切换命令
#ifdef GSM_GET_HANDOVER_FORM_L3           
                if (s_bStartHandover)
                {
                    return;
                }

                s_bStartHandover = true;

                s_Handover.HandoverStartTime = getRealtimeOfCS();
                s_Handover.HandoverStartTimestamp = pTraceFrame->timestamp;
                s_Handover.Speed_Start = pTraceFrame->gps_data.Speed;
                s_Handover.nKmPost_Start = pTraceFrame->gps_data.KmPosition;
                s_Handover.Latitude_Start = pTraceFrame->gps_data.latitude;
                s_Handover.Longitude_Start= pTraceFrame->gps_data.longitude;
                s_Handover.nBCCH_start = g_L1Msg.nServingBCCH;
                s_Handover.nNcc_Start = g_L1Msg.nServingNcc;
                s_Handover.nBcc_Start = g_L1Msg.nServingBcc;


                if (iMsgCodeLen >= 12)
                {
                    s_Handover.nNcc_End = (gsmFrame[2] & 0x38) >> 3;
                    s_Handover.nBcc_End = gsmFrame[2] & 7;
                        
                    s_Handover.nBCCH_end = (gsmFrame[2] & 0xc0) * 4 + gsmFrame[3];
                }

                //提取切换到的TCH频点
                if ((gsmFrame[5] & 0x10) == 0)
                {
                    g_L1Msg.CurrentTCH = (gsmFrame[5] & 0x03) * 0x100 + gsmFrame[6];
                }
#endif                
                break;
            case 0x2c://切换完成
#ifdef GSM_GET_HANDOVER_FORM_L3           
                if (!s_bStartHandover)
                {
                    return;
                }
                s_bStartHandover = false;
               
                s_Handover.HandOverEndTime = getRealtimeOfCS();
                s_Handover.HandOverEndTimestamp= pTraceFrame->timestamp;

                s_Handover.nResult = 1;
               
                s_Handover.Speed_End = pTraceFrame->gps_data.Speed;
                s_Handover.nKmPost_End = pTraceFrame->gps_data.KmPosition;
                s_Handover.Latitude_End = pTraceFrame->gps_data.latitude;
                s_Handover.Longitude_End = pTraceFrame->gps_data.longitude;
                
                WriteFile_handover(s_Handover);
                memset(&s_Handover, 0, sizeof(Handover));

                g_L1Msg.nServingBCCH = s_Handover.nBCCH_end;
                g_L1Msg.nServingBcc = s_Handover.nBcc_End;
                g_L1Msg.nServingNcc = s_Handover.nNcc_End;
#endif                
                break;
            case 0x2d://物理信息
                break;
            case 0x2e://指派命令
                //提取TCH
                if ((gsmFrame[3] & 0x10) == 0)
                {
                    g_L1Msg.CurrentTCH = (gsmFrame[3] & 0x03) * 0x100 + gsmFrame[4];
                }
                break;
            case 0x2f://指派失败
            case 0x30:
            case 0x31:
            case 50:
            case 0x33:
            case 0x34://GPRS Suspension Request
            case 0x35:
            case 0x36://Extended Measurement Report
            case 0x37://Extended Measurement Order
            case 0x38://Application Information应用信息
            case 0x39:
            case 0x3a:
            case 0x3b:
            case 60:
            case 0x3d://系统信息类型16
            case 0x3e://系统消息类型17
            case 0x3f:
            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
            case 70:
            case 0x47:
            case 0x48:
            case 0x49:
            case 0x4a:
            case 0x4b:
            case 0x4c:
            case 0x4d:
            case 0x4e:
                break;

            default: 
                RLOGE("ProcessRRMessage default."); 
                break;
        }
    }
}


void SetCallParams(int nCallDuration, int nCallWindows, int nMaxAccess, int nMaxHandoverTime)
{
#if 0

    m_nCallDuration = nCallDuration;
    m_nCallWindows = nCallWindows;
    m_nMaxAccess = nMaxAccess;
    m_nMaxHandoverTime = nMaxHandoverTime;
#endif    
}

void UpdateDisconnect(const Trio_trace_frame * pTraceFrame)
{
#if 0
    if (pTraceFrame != null)
    {
        DateTime callEndTime = new DateTime();
        callEndTime = pL3Message.GetTime();
        
        if (m_nSpeechCallStatus != 0)
        {
            m_pSpeechCall.SetCallEndTime(callEndTime);
            m_pSpeechCall.SetCallEndTimeStamp(pL3Message.GetTimestamp());
            if (m_nSpeechCallStatus == 2)
            {
                m_pSpeechCall.SetResult(2);
            }
            else if (m_nSpeechCallStatus == 3)
            {
                m_pSpeechCall.SetResult(1);
            }
            else if (m_nSpeechCallStatus == 1)
            {
                m_pSpeechCall.SetResult(0);
            }
            m_pSpeechCall.SetCIEnd(g_GSMUmParam.GetCI());
            m_pSpeechCall.SetKmPostEnd(pL3Message.GetKmPost());
            m_pSpeechCall.SetSpeedEnd(pL3Message.GetSpeed());
            m_pSpeechCall.SetLATLONGEnd(pL3Message.GetLatitude(), pL3Message.GetLongitude());
            m_pSpeechCallPool.PushBack(m_pSpeechCall);
            RenewalSpeechCallList();
            m_nSpeechCallStatus = 0;
        }

        //triorailFrame.CallInterverTimeTimer.Enabled = true;
    }
#endif    
}


//处理真正的层三信令的原始16进制字节数组
void PreprocessL3Message(Trio_trace_frame * pTraceFrame)
{
    if (!s_GSMUmParam_init) {
        GSMUmParamClear(g_GSMUmParam);
        s_GSMUmParam_init = true;
    }
    
    //pMsgCode中存放的是纯粹的GSM层三信令，可参照GSM04.06进行解析
    uint8_t * pTriorailMessage = pTraceFrame->data;
    uint8_t * pMsgCode = pTriorailMessage + 9;
    int iMsgLen = (0x100 * pTriorailMessage[7]) + pTriorailMessage[8];
    
    if (iMsgLen >= 1)
    {
        int iMessageTypeCode;
        int iProtocolDiscriminator = pMsgCode[0] & 0x0F;
        int iMessageType = pMsgCode[1];
        RLOGI("PreprocessL3Message iProtocolDiscriminator=%d", iProtocolDiscriminator); 
        switch (iProtocolDiscriminator)
        {
            case 0:
            case 1:
            case 2:
            case 4:
            case 9:
            case 10://Session Management messages， transaction identifier
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
                return;

            case 3://Call Control;call related SS messages,transaction identifier
                //iMessageType最高两位肯定为0x，所以靠后六位区分
                iMessageType &= 0x3f;
                break;

            case 5://Mobility Management messages for non-GPRS services，pMsgCode[0]的高四位必须为零, skip indicator
                if (!(pMsgCode[0] & 0xF0))
                {
                    iMessageType &= 0x3f;
                    break;
                }
                return;

            case 6://Radio Resource Management messages，pMsgCode[0]的高四位必须为零,skip indicator
                if (iMsgLen == 1)
                {
                    break;
                }
                else if (!(pMsgCode[0] & 0xF0))
                {
                    break;
                }
                return;

            case 8://Mobility Management messages for GPRS services，pMsgCode[0]的高四位必须为零,skip indicator
                if (!(pMsgCode[0] & 0xF0))
                {
                    break;
                }
                return;

            default:
                if (iMsgLen != 1)
                {
                    return;
                }
                break;
        }

        if (iMsgLen == 1)
        {
            iMessageTypeCode = 0x30d01;
        }
        else
        {
            iMessageTypeCode = (0x30000 | (iProtocolDiscriminator << 8)) | iMessageType;
        }


        switch (((iMessageTypeCode & 0xf00) >> 8))
        {
            case 3://Call Control Messages. TODO: not use now.
                ProcessCCMessage((int64_t)iMessageTypeCode, pTraceFrame);
                break;

            case 5://Mobile Management(没处理)
                ProcessMMMessage((int64_t)iMessageTypeCode, pTraceFrame);
                break;

            case 6://处理了掉话记录视图的更新和参数显示视图,切换
                ProcessRRMessage((int64_t)iMessageTypeCode, pTraceFrame);
                break;

            case 8://GPRS mobility management (没处理)
                ProcessGPRSMMMessage((int64_t)iMessageTypeCode, pTraceFrame);
                break;

            case 10://GPRS session management(没处理)
                ProcessGPRSSMMessage((int64_t)iMessageTypeCode, pTraceFrame);
                break;
            default: 
                RLOGE("PreprocessL3Message, iMessageTypeCode default.");  
                break;
        }
    }
    else
    {
        RLOGE("PreprocessL3Message L3 msg len<1.");
    }
}



void PreprocessL2Message(Trio_trace_frame * pTraceFrame)
{
#ifdef GSM_GET_HANDOVER_FORM_L2

    uint8_t * pTriorailMessage = pTraceFrame->data;
    uint8_t * pMsgCode = pTriorailMessage + 9;
    int iMsgLen = (0x100 * pTriorailMessage[7]) + pTriorailMessage[8];
    RLOGI("PreprocessL2Message.");

    if ((((((pMsgCode[0] & 0x60) == 0) && ((pMsgCode[0] & 0x1c) == 0)) 
     && (((pMsgCode[0] & 1) == 1) && ((pMsgCode[1] & 1) == 0))) && ((pMsgCode[2] & 1) == 1)))
    {
     //命令
        if ((pMsgCode[0] & 2) == 2)
        {
            if ((pMsgCode[3] & 240) != 0)//11110000
            {
                return;
            }

             //00000110
            if (((pMsgCode[3] & 15) == 6) && (pMsgCode[4] == 0x2b))//00101011
            {
                if (s_bStartHandover)
                {   
                    int time_interval; //ms
                    RLOGE("PreprocessL2Message, s_bStartHandover is true.");
                    time_interval = (pTraceFrame->timestamp - s_Handover.HandoverStartTime)/10000;
                    RLOGE("PreprocessL2Message, time_interval=%d.", time_interval);
                    if (time_interval < 3000)
                    {
                        return;
                    }
                }

                s_bStartHandover = true;
                memset(&s_Handover, 0, sizeof(Handover));

                s_Handover.HandoverStartTime = pTraceFrame->timestamp;//getRealtimeOfCS();
                s_Handover.HandoverStartTimestamp = pTraceFrame->timestamp;
                s_Handover.Speed_Start = pTraceFrame->gps_data.Speed;
                s_Handover.nKmPost_Start = pTraceFrame->gps_data.KmPosition;
                s_Handover.Latitude_Start = pTraceFrame->gps_data.latitude;
                s_Handover.Longitude_Start= pTraceFrame->gps_data.longitude;
                s_Handover.nBCCH_start = g_L1Msg.nServingBCCH;
                s_Handover.nNcc_Start = g_L1Msg.nServingNcc;
                s_Handover.nBcc_Start = g_L1Msg.nServingBcc;

                if (iMsgLen >= 12)
                {
                    s_Handover.nNcc_End = (pMsgCode[5] & 0x38) >> 3;
                    s_Handover.nBcc_End = pMsgCode[5] & 7;

                    s_Handover.nBCCH_end = (pMsgCode[5] & 0xc0) * 4 + pMsgCode[6];
                }
            }
         }

        //Response
        if ((((pMsgCode[0] & 2) == 0) && ((pMsgCode[3] & 240) == 0)) && ((pMsgCode[3] & 15) == 6))//RR消息
        {
            //切换成功
            if (pMsgCode[4] == 0x2c)
            {
                if (!s_bStartHandover)
                {
                    return;
                }
                s_bStartHandover = false;
                
                s_Handover.HandOverEndTime = pTraceFrame->timestamp;
                s_Handover.HandOverEndTimestamp= pTraceFrame->timestamp;
                
                s_Handover.nResult = 1;
                
                s_Handover.Speed_End = pTraceFrame->gps_data.Speed;
                s_Handover.nKmPost_End = pTraceFrame->gps_data.KmPosition;
                s_Handover.Latitude_End = pTraceFrame->gps_data.latitude;
                s_Handover.Longitude_End = pTraceFrame->gps_data.longitude;
                
                WriteFile_handover(s_Handover);
                
                g_L1Msg.nServingBCCH = s_Handover.nBCCH_end;
                g_L1Msg.nServingBcc = s_Handover.nBcc_End;
                g_L1Msg.nServingNcc = s_Handover.nNcc_End;
             }

             //切换失败
             if (pMsgCode[4] == 0x28)
             {
                 if (s_bStartHandover)
                 {
                     s_bStartHandover = false;
                     
                     s_Handover.HandOverEndTime = pTraceFrame->timestamp;
                     s_Handover.HandOverEndTimestamp= pTraceFrame->timestamp;
                     s_Handover.nResult = 0;
                     s_Handover.Speed_End = pTraceFrame->gps_data.Speed;
                     s_Handover.nKmPost_End = pTraceFrame->gps_data.KmPosition;
                     s_Handover.Latitude_End = pTraceFrame->gps_data.latitude;
                     s_Handover.Longitude_End = pTraceFrame->gps_data.longitude;
                     
                     WriteFile_handover(s_Handover);
                 }
             }
        }
 }
 #endif
}


