#ifndef HAL_TRIORAIL_H
#define HAL_TRIORAIL_H

#include <stdlib.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#define SOCKET_NAME_HAL_TRIORAIL "hal-triorail-socket"


//DIAL REQ
typedef struct {
    uint32_t token; // 每次发送加1 
    uint32_t clir; // not use now, set to 0 always.
    uint32_t addressLen;
    char address[0]; // must include '\0' at the end.  
} RIL_dial_req;


//ALL RSP
typedef struct {
    uint32_t token; 
    uint32_t status;
} RIL_cmd_rsp;

/**
 * RIL_TRIO_0_HANGUP_REQ 
 * hangup a voice call
 */
typedef struct {
    uint32_t token; 
    uint32_t callID; // Hang up a specific line. we only concern 1 way call now, so set to 1 always.
} RIL_hangup_req;

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
    uint32_t status;
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
    char traceFileName[0]; // must include '\0' at the end.
    char railLineName[0];  // no '\0' include
    char railwayBureauName[0]; //no '\0' include
} RIL_trace_start_req;

/**
 * RIL_TRIO_0_TRACE_END_REQ 
 * request to close the trace file.
 */
typedef struct {
    uint32_t token; 
    uint32_t traceFileNameLen;
    char traceFileName[0]; // must include '\0' at the end.
} RIL_trace_end_req;


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

#ifdef __cplusplus
}
#endif

#endif /*HAL_TRIORAIL_H*/


