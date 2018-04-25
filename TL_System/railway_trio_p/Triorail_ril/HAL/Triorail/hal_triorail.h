#ifndef HAL_TRIORAIL_H
#define HAL_TRIORAIL_H 1

#include <stdlib.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


/*
统一管理trio模块0，模块1，trace。
通过socket发送下面的命令RIL_msg，如果需要再补充。socket封装按照cmd_header，具体格式说明如下：
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
下面几个和模块1相关的cmd, HAL直接接听。以后根据情况再做修改。
    RIL_TRIO_1_INCOMING_CALL_IND,
    RIL_TRIO_1_ANSWER_REQ,
    RIL_TRIO_1_ANSWER_RSP,
几个查状态的命令像RIL_TRIO_0_STATUS_REQ，可以做心跳，默认都返回正常。    
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
    
    TRIO_RIL_E_TRIO_0_UNKNOW_ERR = -421, //对应 -421	特优洛模块1未知错误， -431	特优洛模块2未知错误
    TRIO_RIL_E_TRIO_0_OPEN_PORT_ERR = -422,  //对应 -422	特优洛模块1测试端口打开失败
    TRIO_RIL_E_TRIO_0_NO_SIM = -423,  //对应 -423	特优洛模块1找不到SIM卡
    TRIO_RIL_E_TRIO_0_RESET = -424, //对应 -424	特优洛模块1重启

    TRIO_RIL_E_TRIO_1_UNKNOW_ERR = -431, //对应 -431	特优洛模块2未知错误， -431	特优洛模块2未知错误
    TRIO_RIL_E_TRIO_1_OPEN_PORT_ERR = -432,  //对应 -432	特优洛模块2测试端口打开失败
    TRIO_RIL_E_TRIO_1_NO_SIM = -433,  //对应 -433	特优洛模块2找不到SIM卡
    TRIO_RIL_E_TRIO_1_RESET = -434, //对应 -434	特优洛模块2重启

    TRIO_RIL_E_MAX = -500
} RIL_triorail_Errno;





#pragma  pack (push,1)  
struct cmd_header {
    unsigned short host;
    unsigned short target;
    unsigned short cmd;
    unsigned short len;
    unsigned char data[0];//数据或参数，属于cmd命令下面自己定义的格式
};


//下面是对应命令是data[]

/**
 * RIL_TRIO_TASK_CFG_IND 
 */
typedef struct {
    uint8_t token; 
    uint8_t direction;   //上下行
    uint8_t Trio_0_taskConfig; //TRIORAIL1配置任务标志
    uint8_t Trio_0_testType; //TRIORAIL1测试类型, 语音呼叫0x10/0x11/0x12/0x13
    uint8_t Trio_0_callType; //TRIORAIL1呼叫类型, 主叫：0x01，被叫:0x00 
    uint8_t Trio_0_modemType; //TRIORAIL1模块类型
    uint8_t Trio_0_callDuration; //长呼：1，短呼：0
    //TRIORAIL2目前为0，HAL暂时不处理。
    uint8_t Trio_1_taskConfig; //TRIORAIL2配置任务标志
    uint8_t Trio_1_testType; //TRIORAIL2测试类型, 语音呼叫0x10/0x11/0x12/0x13
    uint8_t Trio_1_callType; //TRIORAIL2呼叫类型, 主叫：0x01，被叫:0x00 
    uint8_t Trio_1_modemType; //TRIORAIL2模块类型
    uint8_t Trio_1_callDuration; //长呼：1，短呼：0
    
} RIL_task_config_ind;

/**
 * RIL_TRIO_0_DIAL_REQ 
 * Initiate voice call
 */
typedef struct {
    uint32_t token; // 每次发送加1 
    /* (same as 'n' paremeter in TS 27.007 7.7 "+CLIR"
    * clir == 0 on "use subscription default value"
    * clir == 1 on "CLIR invocation" (restrict CLI presentation)
    * clir == 2 on "CLIR suppression" (allow CLI presentation)
    */
    uint32_t clir; // not use now, set to 0 always.
    uint32_t addressLen;
    char address[0]; // must include '\0' at the end.

} RIL_dial_req;

//目前用到的rsp消息结构都一样，以后有特殊的单独列出。
/**
 * RIL_TRIO_0_DIAL_RSP 
 * result of outgoing voice call
 */
typedef struct {
    uint32_t token; //必须和req一致，丢掉无效的rsp。
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



//几个STATUS_REQ, STATUS_RSP的结构一样，统一定义
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
    char  railLineName[0]; // 线路名. 
    char  railwayBureauName[0]; //路局名. 
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


