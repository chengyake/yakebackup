#ifndef HAL_TRIORAIL_H
#define HAL_TRIORAIL_H 1

#include <stdlib.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


/*
ͳһ����trioģ��0��ģ��1��trace��
ͨ��socket�������������RIL_msg�������Ҫ�ٲ��䡣socket��װ����cmd_header�������ʽ˵�����£�
*/ 

#define SOCKET_NAME_HAL_TRIORAIL "hal-triorail-socket"



typedef enum {
    MODULE_FRAMEWORK_CORE = 0,
    MODULE_DEVICE_MISC,
    MODULE_DEVICE_GPRS, 
    MODULE_DEVICE_GPS,
    MODULE_DEVICE_TRIO, 
    MODULE_USER_MONITOR,
    MODULE_USER_CLIENT,
    MODULE_USER_UI,
    MODULE_USER_DEBUG,
    MODULE_MAX
} module_ID;



/*
���漸����ģ��1��ص�cmd, HALֱ�ӽ������Ժ������������޸ġ�
    RIL_TRIO_1_INCOMING_CALL_IND,
    RIL_TRIO_1_ANSWER_REQ,
    RIL_TRIO_1_ANSWER_RSP,
������״̬��������RIL_TRIO_0_STATUS_REQ��������������Ĭ�϶�����������    
*/    
    
/* cmd of cmd_header, interface for uplayer. */
typedef enum {
    CORE_MSG_REGISTER_CMD = 2,
    CORE_MSG_REGISTER_RSP,

    GPS_MSG_GET_GPS_INFO_CMD = 25,

    RIL_TRIO_TASK_CFG_IND = 28,     
    RIL_TRIO_0_DIAL_REQ,
    RIL_TRIO_0_DIAL_RSP,
    RIL_TRIO_0_HANGUP_REQ, 
    RIL_TRIO_0_HANGUP_RSP,
    RIL_TRIO_0_STATUS_REQ, 
    RIL_TRIO_0_STATUS_RSP,
    RIL_TRIO_0_NO_CARRIER_IND,
    RIL_TRIO_1_INCOMING_CALL_IND,
    RIL_TRIO_1_ANSWER_REQ,
    RIL_TRIO_1_ANSWER_RSP,
    RIL_TRIO_1_STATUS_REQ,
    RIL_TRIO_1_STATUS_RSP,
    RIL_TRIO_0_TRACE_START_REQ, 
    RIL_TRIO_0_TRACE_START_RSP,
    RIL_TRIO_0_TRACE_END_REQ, 
    RIL_TRIO_0_TRACE_END_RSP,
    RIL_TRIO_0_TRACE_STATUS_REQ,
    RIL_TRIO_0_TRACE_STATUS_RSP,
    
    RIL_MSG_END
} RIL_msg;


typedef enum {
    TRIO_RIL_E_SUCCESS = 0,
    TRIO_RIL_E_RADIO_NOT_AVAILABLE = 1,     /* If radio did not start or is resetting */
    TRIO_RIL_E_GENERIC_FAILURE = 2,
    
    TRIO_RIL_E_TRIO_0_UNKNOW_ERR = -421, //��Ӧ -421	������ģ��1δ֪���� -431	������ģ��2δ֪����
    TRIO_RIL_E_TRIO_0_OPEN_PORT_ERR = -422,  //��Ӧ -422	������ģ��1���Զ˿ڴ�ʧ��
    TRIO_RIL_E_TRIO_0_NO_SIM = -423,  //��Ӧ -423	������ģ��1�Ҳ���SIM��
    TRIO_RIL_E_TRIO_0_RESET = -424, //��Ӧ -424	������ģ��1����

    TRIO_RIL_E_TRIO_1_UNKNOW_ERR = -431, //��Ӧ -431	������ģ��2δ֪���� -431	������ģ��2δ֪����
    TRIO_RIL_E_TRIO_1_OPEN_PORT_ERR = -432,  //��Ӧ -432	������ģ��2���Զ˿ڴ�ʧ��
    TRIO_RIL_E_TRIO_1_NO_SIM = -433,  //��Ӧ -433	������ģ��2�Ҳ���SIM��
    TRIO_RIL_E_TRIO_1_RESET = -434, //��Ӧ -434	������ģ��2����

    TRIO_RIL_E_MAX = -500
} RIL_triorail_Errno;





#pragma  pack (push,1)  
struct cmd_header {
    unsigned short host;
    unsigned short target;
    unsigned short cmd;
    unsigned short len;
    unsigned char data[0];//���ݻ����������cmd���������Լ�����ĸ�ʽ
};


//�����Ƕ�Ӧ������data[]

/**
 * RIL_TRIO_TASK_CFG_IND 
 */
typedef struct {
    uint8_t token; 
    uint8_t direction;   //������
    uint8_t Trio_0_taskConfig; //TRIORAIL1���������־
    uint8_t Trio_0_testType; //TRIORAIL1��������, ��������0x10/0x11/0x12/0x13
    uint8_t Trio_0_callType; //TRIORAIL1��������, ���У�0x01������:0x00 
    uint8_t Trio_0_modemType; //TRIORAIL1ģ������
    uint8_t Trio_0_callDuration; //������1���̺���0
    //TRIORAIL2ĿǰΪ0��HAL��ʱ������
    uint8_t Trio_1_taskConfig; //TRIORAIL2���������־
    uint8_t Trio_1_testType; //TRIORAIL2��������, ��������0x10/0x11/0x12/0x13
    uint8_t Trio_1_callType; //TRIORAIL2��������, ���У�0x01������:0x00 
    uint8_t Trio_1_modemType; //TRIORAIL2ģ������
    uint8_t Trio_1_callDuration; //������1���̺���0
    
} RIL_task_config_ind;

/**
 * RIL_TRIO_0_DIAL_REQ 
 * Initiate voice call
 */
typedef struct {
    uint32_t token; // ÿ�η��ͼ�1 
    /* (same as 'n' paremeter in TS 27.007 7.7 "+CLIR"
    * clir == 0 on "use subscription default value"
    * clir == 1 on "CLIR invocation" (restrict CLI presentation)
    * clir == 2 on "CLIR suppression" (allow CLI presentation)
    */
    uint32_t clir; // not use now, set to 0 always.
    uint32_t addressLen;
    char address[0]; // must include '\0' at the end.

} RIL_dial_req;

//Ŀǰ�õ���rsp��Ϣ�ṹ��һ�����Ժ�������ĵ����г���
/**
 * RIL_TRIO_0_DIAL_RSP 
 * result of outgoing voice call
 */
typedef struct {
    uint32_t token; //�����reqһ�£�������Ч��rsp��
    RIL_triorail_Errno status;
} RIL_cmd_rsp;

/**
 * RIL_TRIO_0_HANGUP_REQ 
 * hangup a voice call
 */
typedef struct {
    uint32_t token; 
    uint32_t callID; // Hang up a specific line. we only concern 1 way call now, so set to 1 always.
} RIL_hangup_req;



//����STATUS_REQ, STATUS_RSP�Ľṹһ����ͳһ����
/**
 * RIL_TRIO_0_STATUS_REQ 
 * get the status of the device.
 */
typedef struct {
    uint32_t token; 
} RIL_status_req;

/**
 * RIL_TRIO_0_STATUS_RSP 
 * result of the device status.
 */
typedef struct {
    uint32_t token; 
    RIL_triorail_Errno status;
} RIL_status_rsp;

/**
 * RIL_TRIO_0_TRACE_START_REQ 
 * start to write trace to a file.
 */
typedef struct {
    uint32_t token; 
    uint32_t traceFileNameLen;
    uint32_t railLineLen;
    uint32_t railwayBureauLen;
    char  traceFileName[0]; // must include '\0' at the end.
    char  railLineName[0]; // ��·��. 
    char  railwayBureauName[0]; //·����. 
} RIL_trace_start_req;



/**
 * RIL_TRIO_0_TRACE_END_REQ 
 * request to close the trace file.
 */
typedef struct {
    uint32_t token; 
    uint32_t traceFileNameLen;
    char  traceFileName[0]; // must include '\0' at the end.
} RIL_trace_end_req;


typedef struct  {
    double longitude;
    double latitude;
    uint64_t speed;
    uint64_t kmposition;
}gps_info_t;

#pragma pack(pop)


#ifdef __cplusplus
}
#endif

#endif /*HAL_TRIORAIL_H*/


