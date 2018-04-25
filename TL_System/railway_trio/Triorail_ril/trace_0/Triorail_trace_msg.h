#ifndef TRIORAIL_TRACE_MSG_H
#define TRIORAIL_TRACE_MSG_H 1

#include <stdlib.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int nMCC;
    int nMNC;
    int nLAC;
}Location_Area_Identification;

typedef struct {
    int nMax_Retrans;
    int nTx_integer;
    bool bCELL_BAR_ACCESS;
    bool bRE;
    bool bEC;
    bool aAC[0x10];
}RACH_Control_Param;

typedef struct {
    bool bACC;
    int nBS_AG_BLKS_RES;
    int nCCCH_CONF;
    int nBS_PA_MFRMS;
    int nT3212;
}Control_Channel_Description;

typedef struct {
    int nCELL_RESELECT_HYSTERESIS; //n*2
    int nMS_TXPWR_MAX_CCH; // 0 -31
    int nACS;
    bool bNECI;
    int nLevel_Access_Min;
}Cell_Select_Parameters;


typedef struct {
    int nPower_Control_Indicator; //0 PWRC is not set;1 PWRC is set
    int nDTX;// 0:The MSs may use uplink discontinuous transmission;
    //1:The MSs shall use uplink discontinuous transmission;1:The MSs shall not use uplink discontinuous transmission
    int nRadio_Link_Timeout; //(n+1)*4
}Cell_Description_SACCH;


typedef struct {
    Control_Channel_Description m_CtlCHDesc;
    Location_Area_Identification m_LAI;
    RACH_Control_Param m_RachCtlParam;
    Cell_Select_Parameters m_CellSelectParameters;
    Cell_Description_SACCH m_CellDescriptionSACCH;
    int m_wCell_Identity;
    int nNcc;
    int nBcc;
    double dKmPost;
    int nTA;
} GSMUmParam;




//=====================================
// struct used to write file.

#define TRIORAIL_COUNT_MAX 2

#pragma  pack (push,1)  

typedef struct {
    uint8_t STX;
    uint8_t appID;
    uint16_t appLen; //use 13 bits, max len is 8191 bytes.
    uint8_t appData[0];
    uint8_t FCS;
    uint8_t ETX;
} Triorail_traceFrame;



typedef struct {
    uint8_t Triorail_ID;
//TestType=0x10、0x11、0x12、0x13或者0x00，即个呼移动端、个呼固定端、紧急呼叫、组呼或者被叫    
    uint8_t TriorailTestType;
    uint8_t TriorailSetType;
    uint8_t TriorailCallType;
    
} Triorail_config;


typedef struct {
    int32_t headLen;
    uint64_t timestamp; 
    int8_t ESPI1;
    int8_t ESPI1Config;
    int8_t ESPI2;
    int8_t ESPI2Config;
    int8_t OT498_890_Flag;
    int8_t OT498_890_Num;
    int8_t SELEX_Flag;
    int8_t SELEX_Num;
    int8_t SAGEM_Flag;
    int8_t SAGEM_Num;
    int8_t Triorail_Flag;
    int8_t Triorail_Num;
    Triorail_config    triorail_data[TRIORAIL_COUNT_MAX];
    int8_t OT290_Flag;
    int8_t OT290_Num;
    int8_t GPRSPingFlag;
    uint8_t GPRSPingArg[3];
    int8_t GPRS_UDP_flag;
    uint8_t GPRS_UDP_Arg[3];
    int8_t GPRS_FTP_flag;
    int16_t GPRS_FTP_Arg;
    int8_t controlOrderFlag; //new
    int8_t controlOrderConfigFlag; //new
    int8_t CIRTFlag;
    int8_t CIRTArg;
    int8_t baseStationFlag;
    int16_t baseStationNum;
    int8_t rangeEquipmentFlag;
    int16_t rangeEquipmentNum;
    uint16_t railLineLen;
    uint8_t railLineName[9];
    uint16_t railwayBureauLen;
    uint8_t railwayBureauName[9];
    int8_t rfu[2];
    int8_t direction;
    int8_t GPS_map_flag;
    int8_t toLoad;
    uint16_t CRC;

} Trace_file_head;

typedef struct {
    int8_t STX;
    int8_t testSetType;
    int8_t testSetID;
    int16_t frameLen;
    int8_t testType;
    uint64_t timestamp;
    int8_t data[0];
    int8_t XOR; 
    int8_t ETX; 
    
} TestFrame;


typedef struct {
    int64_t KmPosition;
    int64_t Speed;
    int64_t latitude;
    int64_t longitude;
    int8_t direction;
} GPS_data;

typedef struct {
    int8_t AT_dir;
    int8_t AT_len;
    int8_t AT_str[0];
    int64_t KmPosition;
    int64_t Speed;
    int64_t latitude;
    int64_t longitude;
    int8_t direction;
    
} AT_data;

typedef struct {
    int64_t msgID;
    int64_t timestamp;
    int8_t msgChannelCode;
    int8_t msgDirection;
    uint8_t msgLen;
    uint8_t * msgData;
    
    int64_t Speed;
    int64_t KmPosition;
    int64_t latitude;
    int64_t longitude;
    
} L2Data;


typedef struct {
    int64_t msgID;
    int64_t timestamp;
    int8_t msgDirection;
    uint8_t msgLen;
    uint8_t * msgData;
    
    int64_t Speed;
    int64_t KmPosition;
    int64_t latitude;
    int64_t longitude;
} L3Data;


typedef struct {
    uint8_t nBCC;
    uint16_t nBCCH;
    int16_t nC1;
    int16_t nC2;
    uint8_t nNCC;
    int16_t nRxLevel;
    
    uint8_t nRxQual;
    uint8_t nTA;
} cellInfo;


typedef struct {
    int64_t msgID;
    int64_t timestamp;
    uint8_t msgLen;
    
    int nServingNcc;
    int nServingBcc;
    
    int nServingBCCH;    
    uint16_t CurrentTCH;
    uint8_t mode;
    cellInfo servingCellInfo;
    cellInfo servingCellSubInfo;
    cellInfo neighbour1CellInfo;
    cellInfo neighbour2CellInfo;
    cellInfo neighbour3CellInfo;
    cellInfo neighbour4CellInfo;
    cellInfo neighbour5CellInfo;
    cellInfo neighbour6CellInfo;
    
    int64_t KmPosition;
    int64_t Speed;
    int64_t latitude;
    int64_t longitude;
    int8_t direction;
    
} L1Data;

typedef struct {
    uint64_t nKmPost_Start;
    uint64_t nKmPost_End;
    uint64_t Latitude_Start;
    uint64_t Longitude_Start;
    uint64_t Latitude_End;
    uint64_t Longitude_End;
    uint64_t Speed_Start;
    uint64_t Speed_End;
    
    uint64_t HandOverEndTime;
    uint64_t HandoverStartTime;
    uint64_t HandOverEndTimestamp;
    uint64_t HandoverStartTimestamp;
    int nBCCH_end;
    int nBCCH_start;

    uint8_t nResult;
    uint8_t nType;
    uint8_t nNcc_End;
    uint8_t nBcc_End;
    uint8_t nNcc_Start;
    uint8_t nBcc_Start;

}Handover;


#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /*TRIORAIL_TRACE_MSG_H*/

