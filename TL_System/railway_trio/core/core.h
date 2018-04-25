#ifndef CORE_H
#define CORE_H

#include "../gps/gps.h"

//reference MTU
#define SOCKET_BUFFER_SIZE      (1400)
#define LOCAL_SOCKET            "hal-triorail-socket"



//#define debug

#ifndef debug
#define logdebug(...)   ((void)0)
#define loginfo(...)   syslog(LOG_INFO, __VA_ARGS__)
#define logerr(...)   syslog(LOG_ERR, __VA_ARGS__)
#else
#define logdebug(...)   syslog(LOG_DEBUG, __VA_ARGS__)
#define loginfo(...)   syslog(LOG_INFO, __VA_ARGS__)
#define logerr(...)   syslog(LOG_ERR, __VA_ARGS__)
#endif



enum system_status {
    STATUS_IDLE = 0,
    STATUS_SETUP,
    STATUS_RUN,
    STATUS_ERR,
    STATUS_STOP,
    STATUS_STATUS_MAX,
};

//module idx 
enum module_idx {
	IDX_FRAMEWORK_CORE=0,
	IDX_DEVICE_MISC,
    IDX_DEVICE_GPRS, 
    IDX_DEVICE_GPS,  
    IDX_DEVICE_TRIO, 
    IDX_USER_MONITOR,
    IDX_USER_CLIENT, //now
    IDX_USER_UI,     
    IDX_USER_DEBUG, 
    IDX_CLIENTS_MAX, 
};

//all cmds in a table
enum core_msg_t {
    CORE_MSG_GET_STATUS_CMD=0,
    CORE_MSG_GET_STATUS_RSP,
    CORE_MSG_REGISTER_CMD,
    CORE_MSG_REGISTER_RSP,
    CORE_MSG_UNREGISTER_CMD,
    CORE_MSG_UNREGISTER_RSP,
    CORE_MSG_RELEASE_CMD,
    CORE_MSG_RELEASE_RSP,
    CORE_MSG_MAX,
};

enum misc_msg_t {
    MISC_MSG_GET_STATUS_CMD = CORE_MSG_MAX,//8
    MISC_MSG_GET_STATUS_RSP,
    MISC_MSG_GET_TIME_CMD,
    MISC_MSG_GET_TIME_RSP,
    MISC_MSG_SET_TIME_CMD,
    MISC_MSG_SET_TIME_RSP,
    MISC_MSG_CHECK_SD_SIZE_CMD,
    MISC_MSG_CHECK_SD_SIZE_RSP,
    MISC_MSG_RELEASE_CMD,
    MISC_MSG_RELEASE_RSP,
    MISC_MSG_MAX,
};

enum gprs_msg_t {
    GPRS_MSG_GET_STATUS_CMD = MISC_MSG_MAX,//18
    GPRS_MSG_GET_STATUS_RSP,
    GPRS_MSG_RELEASE_CMD,
    GPRS_MSG_RELEASE_RSP,
    GPRS_MSG_MAX,
};

enum gps_msg_t {
    GPS_MSG_GET_STATUS_CMD = GPRS_MSG_MAX,//22
    GPS_MSG_GET_STATUS_RSP,
    GPS_MSG_GET_GPS_INFO_CMD,
    GPS_MSG_GET_GPS_INFO_RSP,
    GPS_MSG_RELEASE_CMD,
    GPS_MSG_RELEASE_RSP,
    GPS_MSG_MAX,
};

enum trio_msg_t {
    TRIO_MSG_0_TASK_CFG_CMD = GPS_MSG_MAX,//28
    TRIO_MSG_0_DIAL_REQ,//29
    TRIO_MSG_0_DIAL_RSP,
    TRIO_MSG_0_HANGUP_REQ, //31
    TRIO_MSG_0_HANGUP_RSP,
    TRIO_MSG_0_STATUS_REQ,//33 
    TRIO_MSG_0_STATUS_RSP,
    TRIO_MSG_0_NO_CAR_IND,//35
    TRIO_MSG_1_INCOMING_CALL_IND,//useless now
    TRIO_MSG_1_ANSWER_REQ, //useless now
    TRIO_MSG_1_ANSWER_RSP, //useless now yangwei process it autolly
    TRIO_MSG_1_STATUS_REQ,
    TRIO_MSG_1_STATUS_RSP,
    TRIO_MSG_0_TRACE_START_REQ, //41
    TRIO_MSG_0_TRACE_START_RSP,
    TRIO_MSG_0_TRACE_STOP_REQ,  //43
    TRIO_MSG_0_TRACE_STOP_RSP,
    TRIO_MSG_0_TRACE_STATUS_REQ,
    TRIO_MSG_0_TRACE_STATUS_RSP,
    TRIO_MSG_MAX,
};


enum monitor_msg_t {
    MONITOR_MSG_GET_STATUS_CMD = TRIO_MSG_MAX,
    MONITOR_MSG_GET_STATUS_RSP,
    MONITOR_MSG_REBOOT_SW_CMD,
    MONITOR_MSG_REBOOT_SW_RSP,
    MONITOR_MSG_REBOOT_HW_CMD,
    MONITOR_MSG_REBOOT_HW_RSP,
    MONITOR_MSG_RELEASE_CMD,
    MONITOR_MSG_RELEASE_RSP,
    MONITOR_MSG_MAX,
};


enum client_msg_t {
    CLIENT_MSG_GET_STATUS_CMD = MONITOR_MSG_MAX,
    CLIENT_MSG_GET_STATUS_RSP,
    CLIENT_MSG_RELEASE_CMD,
    CLIENT_MSG_RELEASE_RSP,
    CLIENT_MSG_MAX,
};

enum ui_msg_t {
    UI_MSG_GET_STATUS_CMD = CLIENT_MSG_MAX,
    UI_MSG_GET_STATUS_RSP,
    UI_MSG_RELEASE_CMD,
    UI_MSG_RELEASE_RSP,
    UI_MSG_MAX,
};

enum debug_msg_t {
    DEBUG_MSG_GET_STATUS_CMD = UI_MSG_MAX,
    DEBUG_MSG_GET_STATUS_RSP,
    DEBUG_MSG_GET_TIME_CMD,
    DEBUG_MSG_GET_TIME_RSP,
    DEBUG_MSG_SET_TIME_CMD,
    DEBUG_MSG_SET_TIME_RSP,
    DEBUG_MSG_CHECK_SD_SIZE_CMD,
    DEBUG_MSG_CHECK_SD_SIZE_RSP,
    DEBUG_MSG_RELEASE_CMD,
    DEBUG_MSG_RELEASE_RSP,
    DEBUG_MSG_MAX,
};




enum report_position_state {
	REPORT_POSITION = 500,
};

enum report_status_state {
	REPORT_STATUS = 400,
	REPORT_SUCCESS = 401,
	REPORT_NO_SPACE = -410,
	REPORT_USB_ERR = -411,
	REPORT_WRITE_ERR = -412,
	REPORT_READ_ERR = -413,
    
    //gsm
	REPORT_AT1_UNKNOWN_ERR = -421,
	REPORT_AT1_OPEN_ERR = -422,
	REPORT_AT1_NO_SIM = -423,
	REPORT_AT1_RESET = -424,

	REPORT_AT2_UNKNOWN_ERR = -431,
	REPORT_AT2_OPEN_ERR = -432,
	REPORT_AT2_NO_SIM = -433,
	REPORT_AT2_RESET = -434,

    //gprs
	REPORT_MODEM_UNKNOWN_ERR = -441,
	REPORT_MODEM_OPEN_ERR = -442,
	REPORT_MODEM_AT_ERR = -443,
	REPORT_MODEM_NO_SIM = -444,
	REPORT_MODEM_DO_PPP = -445,
	REPORT_MODEM_PPP_FAILED = -446,
	REPORT_MODEM_CONNECT_SERVER_ERR = -447,
	REPORT_MODEM_PPP_DISC = -448,
	REPORT_MODEM_TCPIP_DISC = -449,

	REPORT_GPS_UNKNOWN_ERR = -451,
	REPORT_GPS_OPEN_ERR = -452,
};




//note: Byte alignment
#pragma pack (1)
struct cmd_header {
	unsigned short host;
	unsigned short target;
	unsigned short cmd;
	unsigned short len;
    unsigned char data[0];
};
#pragma pack ()

#define CMD_HEADER_SIZE (sizeof(struct cmd_header))

#define TRAIN_NAME  "CRH380BJ-0301"
//#define TRAIN_NAME  "CRH2C-2150"






#endif
