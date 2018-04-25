#ifndef TRIORAIL_TRACE_H
#define TRIORAIL_TRACE_H 1

#include <stdlib.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#pragma  pack (push,1)  
typedef struct  {
	uint32_t token;
	uint32_t cmd;
	uint32_t len;
	uint8_t data[0];
}trace_cmd_frame_req;

typedef struct  {
	uint32_t token;
	uint32_t cmd;
	uint32_t status;
}trace_cmd_frame_rsp;


enum trace_cmd {
    TRACE_GET_STATUS,
    TRACE_ON,
    TRACE_OFF,
    TRACE_CONFIG,
    TRACE_SHORT_CALL_IND, //send from HAL
    TRACE_SHORT_INCOMING_CALL_IND, //send from HAL
    TRACE_SHORT_STATISTIC_IND, //send from HAL
    TRACE_AT_DATA_IND, //send from HAL
    TRACE_GPS_IND,
    TRACE_TASK_CONFIG_IND,
    TRACE_TEST,
};

enum trace_status {
    TRACE_OK,
    TRACE_ERR_OPEN_DEV,
    TRACE_ERR_OPEN_FILE,
    TRACE_ERR_1,
    TRACE_ERR_2,
};

typedef struct {
    uint32_t dummy;
} trace_get_status;


typedef struct {
    uint32_t traceFileNameLen;
    uint32_t railLineLen;
    uint32_t railwayBureauLen;
    char  traceFileName[0]; // must include '\0' at the end.
    char  railLineName[0]; // ��·��.
    char  railwayBureauName[0]; //·����.  
} trace_start_req;

typedef struct {
    uint32_t traceFileNameLen;
    char  traceFileName[0]; // must include '\0' at the end.
} trace_end_req;


enum call_state_cmd {
    TRIORAIL_CALL_START,
    TRIORAIL_CALL_CONNECT_OK,
    TRIORAIL_CALL_CONNECT_ERR,
    TRIORAIL_CALL_END_OK,
    TRIORAIL_CALL_END_ERR,    
};

typedef struct {
    uint8_t callType;  //fixed, 0x00
    uint64_t callStartTimestamp;
    uint64_t callEndTimestamp;
    
    uint64_t callStartKmPosition;
    uint64_t callEndKmPosition;
    uint64_t callStartSpeed;
    uint64_t callEndSpeed;
    uint64_t callStartLatitude;
    uint64_t callStartLongitude;
    uint64_t callEndLatitude;
    uint64_t callEndLongitude;
    
    uint8_t establishResult;
    uint32_t establishDelay;
    uint8_t lossResult;
    uint8_t direction;
} trace_short_call_ind;

typedef struct {
    uint8_t callType;  //fixed, 0x00
    uint32_t incomingCallTimes;
    uint8_t callStatus;
    
    uint64_t callKmPosition;
    uint64_t callSpeed;
    uint64_t callLatitude;
    uint64_t callLongitude;
    uint8_t direction;
} trace_short_incoming_call_ind;

typedef struct {
    uint32_t voiceCallLess10or5SecTimes; 
    uint32_t voiceCallLess15or75SecTimes; 
    uint32_t voiceCallDelayTimeAll;

    uint8_t callType;  //fixed, 0x01
    uint32_t voiceCallTimes;
    uint64_t voiceCallSuccessRadio;//���гɹ���     
    uint32_t voiceCallFailureTimes;//����ʧ�ܴ���       
    uint32_t voiceCallDelayTime_Max;//��������ʱ   
    uint32_t voiceCallDelayTime_Min;//��С������ʱ  
    uint32_t voiceCallDelayTime_Ave;//ƽ��������ʱ  
    uint32_t voiceCallDelayTime_95;//95%���ʺ�����ʱ                                  
    uint32_t voiceCallDelayTime_99;//99%���ʺ�����ʱ       
    uint64_t voiceCallLess10or5SecRatio;//С��10or5��ĸ���    
    uint64_t voiceCallLess15or75SecRatio;//С��15or7.5��ĸ��ʣ�  
    uint64_t callKmPosition;
    uint64_t callSpeed;
    uint64_t callLatitude;
    uint64_t callLongitude;
    uint8_t direction;
} trace_short_call_statistic_ind;


typedef struct {
    uint8_t AT_dir;
    int64_t KmPosition;
    int64_t Speed;
    int64_t latitude;
    int64_t longitude;
    uint8_t direction;
    uint8_t AT_len;
    char  AT_str[0];    
} trace_AT_data_ind;


typedef struct {
    int64_t KmPosition;
    int64_t Speed;
    uint64_t latitude;
    uint64_t longitude;
    int8_t direction;
}trace_GPS_Info_ind;

typedef struct {
    uint8_t direction;   //������
    uint8_t Trio_0_taskConfig; //TRIORAIL1���������־
    uint8_t Trio_0_testType; //TRIORAIL1��������, ��������0x10/0x11/0x12/0x13
    uint8_t Trio_0_callType; //TRIORAIL1��������, ���У�0x01������:0x00 
    uint8_t Trio_0_modemType; //TRIORAIL1ģ������ ������:0x01���������0x00 
    uint8_t Trio_0_callDuration; //������1���̺���0
    //TRIORAIL2ĿǰΪ0��HAL��ʱ������
    uint8_t Trio_1_taskConfig; //TRIORAIL2���������־
    uint8_t Trio_1_testType; //TRIORAIL2��������, ��������0x10/0x11/0x12/0x13
    uint8_t Trio_1_callType; //TRIORAIL2��������, ���У�0x01������:0x00 
    uint8_t Trio_1_modemType; //TRIORAIL2ģ������ ������:0x01���������0x00 
    uint8_t Trio_1_callDuration; //������1���̺���0
}trace_task_config_ind;


#pragma pack(pop)



#ifdef __cplusplus
}
#endif

#endif /*TRIORAIL_TRACE_H*/

