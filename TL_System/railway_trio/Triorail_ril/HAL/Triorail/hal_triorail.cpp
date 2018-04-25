
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/un.h>  
#include <netinet/in.h>
#include <stdio.h>

#include <pthread.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>


#include <ril.h>
#include <Errors.h>
#include <record_stream.h>
#include <hal_triorail.h>
#include <Triorail_trace.h>

#define LOG_TAG "HAL_TRIO"
#include "../liblog/log.h"

#ifndef _GNU_SOURCE
#define     _GNU_SOURCE
#endif


#define SOCKET_NAME_RIL_0 "trio-rild-0"	/* from ril.cpp */
#define SOCKET_NAME_RIL_1 "trio-rild-1"	/* from ril.cpp */
#define SOCKET_NAME_TRACE_0 "trio-trace-0"  /* from Triorail_trace.cpp*/

// match with constant in ril.cpp.
#define MAX_COMMAND_BYTES (1 * 1024)

static int DIAL_DELAY_MSEC = 5*1000;

const  int RESPONSE_SOLICITED = 0;
const  int RESPONSE_UNSOLICITED = 1;
    
int s_fdRild_0 = 0;
int s_fdRild_1 = 0;
int s_fdTrace_0 = 0;
int s_fdTrioHalCmd = 0;

static trace_GPS_Info_ind s_GPS_Info = {0,0,0,0,0};

RIL_triorail_Errno s_rild_0_status = TRIO_RIL_E_SUCCESS;
RIL_triorail_Errno s_rild_1_status = TRIO_RIL_E_SUCCESS;
RIL_triorail_Errno s_trace_0_status = TRIO_RIL_E_SUCCESS;
    
int token = 0; //  ++ before send each request.

#define MAX_FD_EVENTS 8
static fd_set readFds;
static int nfds = 0;
static int fd_readSet[MAX_FD_EVENTS] = {0};
enum socketFdID {
    RILD_0 = 0,
    RILD_1,
    TRACE_0,
    HAL_CMD,
};


static pthread_mutex_t s_commandmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_commandcond = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t s_writeMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_fdMutex = PTHREAD_MUTEX_INITIALIZER;;

static trace_task_config_ind s_taskConfig_Info;
static trace_short_call_ind s_trace_short_call_msg;
static trace_short_call_statistic_ind s_trace_call_statistic_msg;

enum options {
    RADIO_RESET,
    RADIO_OFF,
    RADIO_ON,
    SIM_STATUS,
    VOICE_REGISTRATION_STATE,
    SIGNAL_STRENGTH,
    DIAL_CALL,
    ANSWER_CALL,
    END_CALL,
};

static const char * callResponsesError[] = {
    "NO CARRIER", /* sometimes! */
    "NO ANSWER",
    "NO DIALTONE",
    "BUSY",
};

static void print_usage() {
    perror("Usage: radiooptions [option] [extra_socket_args]\n\
           0 - RADIO_RESET, \n\
           1 - RADIO_OFF, \n\
           2 - RADIO_ON, \n\
           3 - GET_SIM_STATUS, \n\
           4 - REQUEST_VOICE_REGISTRATION_STATE, \n\
           5 - REQUEST_SIGNAL_STRENGTH, \n\
           6 number - DIAL_CALL number, \n\
           7 - ANSWER_CALL, \n\
           8 - END_CALL \n");
}

static int error_check(int argc, char * argv[]) {
    if (argc < 2) {
        return -1;
    }
    const int option = atoi(argv[1]);
    if (option < 0 || option > 10) {
        return 0;
    } else if ((option == DIAL_CALL ) && argc == 3) {
        return 0;
    } else if ((option != DIAL_CALL) && argc == 2) {
        return 0;
    }
    return -1;
}

static int get_number_args(char *argv[]) {
    const int option = atoi(argv[1]);
    if (option != DIAL_CALL) {
        return 1;
    } else {
        return 2;
    }
}

uint64_t getRealtimeOfCS() {
/*c# datetime ToBinary() begin   0001-1-1 0:0:0
     1 day=86400 s
     1 year =  31536000  
     int64_t secTo1970 = 31536000*1969 + 86400*(492 -15) = 62135596800 1296000
     
*/
    uint64_t secTo1970 = 62135596800;
    uint64_t withKind;

    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    withKind =  (now.tv_sec + secTo1970) * 10000000LL + now.tv_nsec/100; // + 0x8000000000000000LL;
    return  withKind;
}

static int
blockingWrite(int fd, const void *buffer, size_t len) {
    size_t writeOffset = 0;
    const uint8_t *toWrite;

    toWrite = (const uint8_t *)buffer;

    while (writeOffset < len) {
        ssize_t written;
        do {
            written = write (fd, toWrite + writeOffset,
                                len - writeOffset);
        } while (written < 0 && ((errno == EINTR) || (errno == EAGAIN)));

        if (written >= 0) {
            writeOffset += written;
        } else {   // written < 0
            RLOGE ("RIL Response: unexpected error on write errno:%d", errno);
            close(fd);
            return -1;
        }
    }

    return 0;
}

static int
sendRequestRaw (const void *data, size_t dataSize, int fd) {
    int ret;
    uint32_t header;

    if (fd <= 0) {
        return -1;
    }

    if (dataSize > MAX_COMMAND_BYTES) {
        RLOGE("RIL: packet larger than %u (%u)",
                MAX_COMMAND_BYTES, (unsigned int )dataSize);
        return -1;
    }

    pthread_mutex_lock(&s_writeMutex);

    header = htonl(dataSize);
    RLOGD("sendRequestRaw   header %d = %d", dataSize, header);

    ret = blockingWrite(fd, (void *)&header, sizeof(header));

    if (ret < 0) {
        pthread_mutex_unlock(&s_writeMutex);
        return ret;
    }

    ret = blockingWrite(fd, data, dataSize);

    if (ret < 0) {
        pthread_mutex_unlock(&s_writeMutex);
        return ret;
    }

    pthread_mutex_unlock(&s_writeMutex);

    return 0;
}
static int sendRequest (Parcel &p) {
    int fd = 0;
    if ( p.rildID == RILD_0) {
        fd = s_fdRild_0;
    }else if ( p.rildID == RILD_1){
        fd = s_fdRild_1;
    }
    return sendRequestRaw(&p, sizeof (p), fd);
}



static int
sendTraceReq (int cmd, void * data, int len) {
    int ret;
    static int token = 0;
    token++;
    
/*
struct trace_cmd_frame_req {
	uint32_t token;
	uint32_t cmd;
	uint32_t len;
	uint8_t data[0];
};
*/  
    uint32_t cmdHead[3] = {0};
    uint8_t *  pSendData;
    pSendData = (uint8_t *)malloc(sizeof(cmdHead) + len);

    if (s_fdTrace_0 <= 0) {
        RLOGE("sendTraceReq: s_fdTrace_0=%d",s_fdTrace_0);
        return -1;
    }
    if (cmd != TRACE_GPS_IND) {
        RLOGI("sendTraceReq: token=%d, cmd=%d, len=%d", token, cmd, len);
    }

    cmdHead[0] = (uint32_t)token;
    cmdHead[1] = (uint32_t)cmd;
    cmdHead[2] = (uint32_t)len;
    memcpy(pSendData, cmdHead, sizeof(cmdHead));
    memcpy(pSendData+sizeof(cmdHead), data, len);
    
    pthread_mutex_lock(&s_writeMutex);

    ret = blockingWrite(s_fdTrace_0, (void *)pSendData, sizeof(cmdHead)+len);

    if (ret < 0) {
        pthread_mutex_unlock(&s_writeMutex);
        free(pSendData);
        return ret;
    }

    pthread_mutex_unlock(&s_writeMutex);
	free(pSendData);
    return 0;
}

static int
sendHALResponse (unsigned short target, int cmd, RIL_triorail_Errno status) {
/*
    unsigned short host;
    unsigned short target;
    unsigned short cmd;
    unsigned short len;
    uint32_t token; 
    RIL_Errno status;
*/    
    unsigned short cmdHead[4] = {0};
    int ret;
    RIL_cmd_rsp rsp;
    
    unsigned char *  pSendData;
    pSendData = (unsigned char *)malloc(sizeof(cmdHead) + sizeof(RIL_cmd_rsp));

    if (s_fdTrioHalCmd <= 0) {
        RLOGE("sendHALResponse: s_fdTrioHalCmd=%d",s_fdTrioHalCmd);
        return -1;
    }
    RLOGI("sendHALResponse: cmd=%d, status=%d", cmd, status);

    cmdHead[0] = (unsigned short)MODULE_DEVICE_TRIO;
    cmdHead[1] = (unsigned short)target;
    cmdHead[2] = (unsigned short)cmd;
    if (status == TRIO_RIL_E_MAX) {
        cmdHead[3] = 0;
        memcpy(pSendData, cmdHead, sizeof(cmdHead));
        pthread_mutex_lock(&s_writeMutex);
        ret = blockingWrite(s_fdTrioHalCmd, (void *)pSendData, sizeof(cmdHead));
        RLOGI("sendHALResponse: cmd0=%d, len=%d", cmdHead[0] , sizeof(cmdHead));
        pthread_mutex_unlock(&s_writeMutex);
        free(pSendData);
        if (ret < 0) {
            return ret;
        } else {
			return 0;
		}
    } 
    cmdHead[3] = sizeof(RIL_cmd_rsp);
    //Todo, should add list to save token.
    rsp.token = 0; 
    rsp.status = status;
    memcpy(pSendData, cmdHead, sizeof(cmdHead));
    memcpy(pSendData+sizeof(cmdHead), &rsp, sizeof(RIL_cmd_rsp));
	
    pthread_mutex_lock(&s_writeMutex);

    ret = blockingWrite(s_fdTrioHalCmd, (void *)pSendData, sizeof(cmdHead)+sizeof(RIL_cmd_rsp));

    if (ret < 0) {
        pthread_mutex_unlock(&s_writeMutex);
        free(pSendData);
        return ret;
    }

    pthread_mutex_unlock(&s_writeMutex);
    free(pSendData);
    return 0;
}


static int readInt32FromParcel (Parcel &p) {
    int32_t * pInt = (int32_t *)p.index;
    p.index = p.index +  sizeof(int32_t);
    return (int) * pInt;
}

//must free char* string after not used.
static char* readStringFromParcel(Parcel &p) {
    int len = readInt32FromParcel(p);
    char * string = NULL;
    if (len<=0) {
        //error;
    } else {
        string = (char *)malloc(sizeof(char)*len);
        memcpy(string, p.index, len);
        p.index = p.index +  sizeof(char) * len;
    }
    return string;
}

static status_t readFromParcel(Parcel &p, void* outData, size_t len) {
    if (len<=0) {
        return BAD_VALUE;
    } else {
        memcpy(outData, p.index, len);
        p.index = p.index +  sizeof(char) * len;
    }
    return NO_ERROR;
}

static const void* readInplaceFromParcel(Parcel &p, size_t len) {
    if (len<=0) {
        return NULL;
    } else {
        const void* data = p.index;
        p.index = p.index +  sizeof(char) * len;
        return data;
    }
}

static void writeInt32ToParcel (Parcel &p, int data) {
    uint8_t * pIndex = (uint8_t *)p.index;
    int32_t * pInt = (int32_t *)pIndex;
    
    * pInt = (int32_t)data;
    RLOGD("writeInt32ToParcel   data   = %d", data);
    p.index = p.index + sizeof(int32_t);
}
static void writeInt64ToParcel (Parcel &p, int64_t data) {
    uint8_t * pIndex =  (uint8_t *) p.index;
    int64_t * pInt64 = (int64_t *)pIndex;
    
    * pInt64 = data;
    p.index = p.index + sizeof(int64_t);
}

static void writeStringToParcel(Parcel &p, const char *s) {

    int len = 0;
    if (s == NULL) {
        writeInt32ToParcel(p, 0);
        return;
    }
    len = strlen(s);
    if (len<=0) {
        writeInt32ToParcel(p, 0);
    } else {
        writeInt32ToParcel(p, len+1); // include the null at the end.
        memcpy(p.index, s, len);
        p.index = p.index + sizeof(char) * len;
        *(p.index) = (uint8_t)'\0';
        p.index++;
    }
}

static void writeDataToParcel(Parcel &p, const void *s, int len) {
    uint8_t * pIndex = (uint8_t *) p.index;
    if (len<=0) {
        writeInt32ToParcel(p, 0);
    } else {
        writeInt32ToParcel(p, len);
        memcpy(pIndex, s, len);
        p.index = p.index +  sizeof(char) * len;
    }
}
static void traceStart(RIL_trace_start_req * pReq) {
/*
typedef struct {
    uint32_t token; 
    uint32_t direction;   
    uint32_t traceFileNameLen;
    uint32_t railLineLen;
    uint32_t railwayBureauLen;
    char  traceFileName[0]; // must include '\0' at the end.
    char  railLineName[0]; // 线路名. must include '\0' at the end.
    char  railwayBureauName[0]; //路局名.  must include '\0' at the end.
} RIL_trace_start_req;

*/
    uint32_t len = pReq->traceFileNameLen;
    if (len != strlen(pReq->traceFileName)+1){
        RLOGE("traceStart, file name len err!");
        return;
    }
    RLOGI("processTraceCmd traceFileNameLen=%d railLineLen=%d railwayBureauLen=%d ", 
    	pReq->traceFileNameLen, pReq->railLineLen, pReq->railwayBureauLen);
						
    int32_t frameLen = sizeof(trace_start_req) + pReq->traceFileNameLen
        + pReq->railLineLen + pReq->railwayBureauLen;
    uint8_t * data = (uint8_t * )malloc(frameLen);
    RLOGI("traceStart   frameLen = %d", frameLen);
    memcpy(data, &(pReq->traceFileNameLen), frameLen);
    
    sendTraceReq (TRACE_ON, data, frameLen);
    free(data);
}

static void traceEnd(RIL_trace_end_req * pReq) {
/*
typedef struct {
    uint32_t traceFileNameLen;
    char  traceFileName[0]; // must include '\0' at the end.
} trace_end_req;

*/
    uint32_t len = pReq->traceFileNameLen;
    if (len != strlen(pReq->traceFileName)+1){
        RLOGE("traceEnd, file name len err!");
        return;
    }
    uint8_t * data =  (uint8_t * )malloc(sizeof(uint32_t) + len);
    
    memcpy(data, &len, sizeof(uint32_t));
    memcpy(data+sizeof(uint32_t), pReq->traceFileName, len);
    
    sendTraceReq (TRACE_OFF, data, sizeof(uint32_t)+len);
    free(data);
}

static void traceGetStatus(void) {
/*
typedef struct {
    uint32_t dummy;
} trace_get_status;

*/
    uint32_t dummy = 0;

    sendTraceReq (TRACE_GET_STATUS, &dummy, sizeof(uint32_t));
}

static void traceSetATString(int dir, const char * str) {

    trace_AT_data_ind * pAT_ind;
    uint32_t len = sizeof(trace_AT_data_ind) + strlen(str);

    pAT_ind = (trace_AT_data_ind *)malloc(len);
    memset(pAT_ind, 0, len);
    
    pAT_ind->AT_dir = dir;
    pAT_ind->AT_len = strlen(str);
    memcpy(pAT_ind->AT_str, str, pAT_ind->AT_len);
/*
    pAT_ind->KmPosition
    pAT_ind->Speed;
    pAT_ind->latitude;
    pAT_ind->longitude;
    pAT_ind->direction;
*/    
    memcpy(&(pAT_ind->KmPosition), &s_GPS_Info, sizeof(trace_GPS_Info_ind));

    RLOGI("traceSetATString AT_len = %d", pAT_ind->AT_len);
    
    sendTraceReq (TRACE_AT_DATA_IND, pAT_ind, len);
    free(pAT_ind);
}

static void traceSetIncomingShortVoiceCallMsg(int callStatus, int trioID) {
    RLOGI("traceSetIncomingShortVoiceCallMsg: callState=%d", callStatus);
    static int64_t callTimes = 0;
    trace_short_incoming_call_ind msg;
    if ((s_taskConfig_Info.Trio_0_callType == 1) && (trioID == RILD_0)) {
        //Calling Line does not handle this msg.
        RLOGE("traceSetIncomingShortVoiceCallMsg: Trio_0_callType == 1");
        return;
    }

    callTimes++;
    if (callTimes == 0x7fffffffLL)
    {
        callTimes = 1LL;
    }
    
    msg.callType = 0;

    msg.incomingCallTimes = callTimes;
    msg.callStatus = callStatus;
    msg.callKmPosition = s_GPS_Info.KmPosition;
    msg.callSpeed = s_GPS_Info.Speed;
    msg.callLongitude = s_GPS_Info.longitude;
    msg.callLatitude = s_GPS_Info.latitude;
    msg.direction = s_GPS_Info.direction;
    sendTraceReq (TRACE_SHORT_INCOMING_CALL_IND, &msg, sizeof(trace_short_incoming_call_ind));

}
static void traceSetShortVoiceCallMsg(call_state_cmd cmd, trace_short_call_ind &msg) {
    
    static bool call_start = false;
    if (s_taskConfig_Info.Trio_0_callDuration == 1) {
        //long call need not save frame to file.
        RLOGI("traceSetShortVoiceCallMsg:  long call");
        return;
    }
    RLOGI("traceSetShortVoiceCallMsg: cmd=%d, call_start=%d",cmd, call_start);
    switch (cmd) {
        case TRIORAIL_CALL_START:
            call_start = true;
            memset(&msg, 0, sizeof(trace_short_call_ind));
            msg.callType = 0;
            msg.callStartTimestamp = getRealtimeOfCS();

            msg.callStartKmPosition = s_GPS_Info.KmPosition;
            msg.callStartSpeed = s_GPS_Info.Speed;
            msg.callStartLongitude = s_GPS_Info.longitude;
            msg.callStartLatitude = s_GPS_Info.latitude;
            msg.direction = s_GPS_Info.direction;

            s_trace_call_statistic_msg.voiceCallTimes++;
            s_trace_call_statistic_msg.callType = 1;
            break;
        case TRIORAIL_CALL_CONNECT_OK:
        {
            msg.establishResult = 1;
            msg.establishDelay = (getRealtimeOfCS() - msg.callStartTimestamp)/10000;

            s_trace_call_statistic_msg.voiceCallDelayTimeAll += msg.establishDelay;
            
            uint32_t successTimes = s_trace_call_statistic_msg.voiceCallTimes - s_trace_call_statistic_msg.voiceCallFailureTimes;
            double ratio;

            if (msg.establishDelay > s_trace_call_statistic_msg.voiceCallDelayTime_Max){
                s_trace_call_statistic_msg.voiceCallDelayTime_Max = msg.establishDelay;
            }
            
            if (msg.establishDelay < s_trace_call_statistic_msg.voiceCallDelayTime_Min){
                s_trace_call_statistic_msg.voiceCallDelayTime_Min = msg.establishDelay;
            }
            ratio = successTimes / (double)s_trace_call_statistic_msg.voiceCallTimes;
            memcpy(&(s_trace_call_statistic_msg.voiceCallSuccessRadio), &ratio, sizeof(double));
            
            if (successTimes > 0){
                s_trace_call_statistic_msg.voiceCallDelayTime_Ave = s_trace_call_statistic_msg.voiceCallDelayTimeAll/ successTimes;            
            }
            else {
                s_trace_call_statistic_msg.voiceCallDelayTime_Ave = 0.0;
            }            
            //
            if (s_taskConfig_Info.Trio_0_testType == 0x10){ //固定端
                if (msg.establishDelay < 5000){
                    s_trace_call_statistic_msg.voiceCallLess10or5SecTimes++;

                    if (successTimes > 0){
                        ratio = s_trace_call_statistic_msg.voiceCallLess10or5SecTimes / (double)successTimes;
                    }
                    else {
                        ratio = 0.0;
                    }
                    memcpy(&(s_trace_call_statistic_msg.voiceCallLess10or5SecRatio), &ratio, sizeof(double));
                }
            
                if (msg.establishDelay < 7500){
                    s_trace_call_statistic_msg.voiceCallLess15or75SecTimes++;
                    if (successTimes > 0){
                        ratio = s_trace_call_statistic_msg.voiceCallLess15or75SecTimes / (double)successTimes;
                    }
                    else {
                        ratio = 0.0;
                    }
                    memcpy(&(s_trace_call_statistic_msg.voiceCallLess15or75SecRatio), &ratio, sizeof(double));
                }
                
            } else{
                if (msg.establishDelay < 10000){
                    s_trace_call_statistic_msg.voiceCallLess10or5SecTimes++;

                    if (successTimes > 0){
                        ratio = s_trace_call_statistic_msg.voiceCallLess10or5SecTimes / (double)successTimes;
                    }
                    else {
                        ratio = 0.0;
                    }
                    memcpy(&(s_trace_call_statistic_msg.voiceCallLess10or5SecRatio), &ratio, sizeof(double));

                }
            
                if (msg.establishDelay < 15000) {
                    s_trace_call_statistic_msg.voiceCallLess15or75SecTimes++;
                    if (successTimes > 0){
                        ratio = s_trace_call_statistic_msg.voiceCallLess15or75SecTimes / (double)successTimes;
                    }
                    else {
                        ratio = 0.0;
                    }
                    memcpy(&(s_trace_call_statistic_msg.voiceCallLess15or75SecRatio), &ratio, sizeof(double));
                }
            }
            //Todo, need a list to store all the delay times of each call.
            s_trace_call_statistic_msg.voiceCallDelayTime_95 = s_trace_call_statistic_msg.voiceCallDelayTime_Max;
            s_trace_call_statistic_msg.voiceCallDelayTime_99 = s_trace_call_statistic_msg.voiceCallDelayTime_Max;

            memcpy(&(s_trace_call_statistic_msg.callKmPosition), &s_GPS_Info, sizeof(trace_GPS_Info_ind));
        }
            break;
        case TRIORAIL_CALL_CONNECT_ERR:
            if (!call_start){
                return;
            }
            call_start = false;
            msg.establishResult = 0;
            msg.establishDelay  = 0;
            msg.callEndTimestamp = getRealtimeOfCS();
            
            msg.callEndKmPosition = s_GPS_Info.KmPosition;
            msg.callEndSpeed = s_GPS_Info.Speed;
            msg.callEndLongitude = s_GPS_Info.longitude;
            msg.callEndLatitude = s_GPS_Info.latitude;
            msg.direction = s_GPS_Info.direction;

            sendTraceReq (TRACE_SHORT_CALL_IND, &msg, sizeof(trace_short_call_ind));

            s_trace_call_statistic_msg.voiceCallFailureTimes++;
            memcpy(&(s_trace_call_statistic_msg.callKmPosition), &s_GPS_Info, sizeof(trace_GPS_Info_ind));
            sendTraceReq (TRACE_SHORT_STATISTIC_IND, &(s_trace_call_statistic_msg.callType), 
                sizeof(trace_short_call_statistic_ind) - 12);
            break;
        case TRIORAIL_CALL_END_OK:
            if (!call_start){
                return;
            }
            call_start = false;

            msg.callEndTimestamp = getRealtimeOfCS();
            msg.lossResult = 1;
            msg.callEndKmPosition = s_GPS_Info.KmPosition;
            msg.callEndSpeed = s_GPS_Info.Speed;
            msg.callEndLongitude = s_GPS_Info.longitude;
            msg.callEndLatitude = s_GPS_Info.latitude;
            msg.direction = s_GPS_Info.direction;

            
            sendTraceReq (TRACE_SHORT_CALL_IND, &msg, sizeof(trace_short_call_ind));
            RLOGI("TRIORAIL_CALL_END_OK:  TRACE_SHORT_CALL_IND");
            sendTraceReq (TRACE_SHORT_STATISTIC_IND, &(s_trace_call_statistic_msg.callType), 
                sizeof(trace_short_call_statistic_ind) - 12);
            RLOGI("TRIORAIL_CALL_END_OK:  TRACE_SHORT_STATISTIC_IND");                
            break;
        case TRIORAIL_CALL_END_ERR:
            if (!call_start){
                return;
            }
            call_start = false;
            msg.callEndTimestamp = getRealtimeOfCS();
            msg.lossResult = 0;

            msg.callEndKmPosition = s_GPS_Info.KmPosition;
            msg.callEndSpeed = s_GPS_Info.Speed;
            msg.callEndLongitude = s_GPS_Info.longitude;
            msg.callEndLatitude = s_GPS_Info.latitude;
            msg.direction = s_GPS_Info.direction;

            sendTraceReq (TRACE_SHORT_CALL_IND, &msg, sizeof(trace_short_call_ind));

            sendTraceReq (TRACE_SHORT_STATISTIC_IND, &(s_trace_call_statistic_msg.callType), 
                sizeof(trace_short_call_statistic_ind) - 12);
            break;
            
        default:
            break;
    }
}

static Parcel s_dialParcel;
static bool s_pendingDialing = false;
static bool s_existCall = false;
/* we support only 1 way call now.*/
static RIL_Call * s_pCurrentCall_0 = NULL;

static void * dialThread(void *arg) {
	RLOGI("dialThread, s_pendingDialing=%d", s_pendingDialing);
    usleep(DIAL_DELAY_MSEC * 1000);
    RLOGI("dialThread  after sleep.");
    
    if (s_pendingDialing){
        sendRequest(s_dialParcel);
    }
    s_pendingDialing = false;
    s_existCall = true;
}



void requestDial(char * address, int clirMode, RIL_UUS_Info *  uusInfo, int trioID) {
    int ret = 0;
    pthread_t tid_dial;
	pthread_attr_t attr;
	
    s_dialParcel.request = RIL_REQUEST_DIAL;
    s_dialParcel.token = token;
    s_dialParcel.rildID= trioID;
    s_dialParcel.index = s_dialParcel.data;
    memset(s_dialParcel.data, 0, sizeof(s_dialParcel.data));
    
    RLOGI("requestDial   address   = %s", address);
    writeStringToParcel(s_dialParcel, address);
    writeInt32ToParcel(s_dialParcel, clirMode);

    if (uusInfo == NULL) {
       writeInt32ToParcel(s_dialParcel, 0); // UUS information is absent
    } else {
       writeInt32ToParcel(s_dialParcel, 1);// UUS information is present
       writeInt32ToParcel(s_dialParcel, uusInfo->uusType);
       writeInt32ToParcel(s_dialParcel, uusInfo->uusDcs);
       writeDataToParcel(s_dialParcel, uusInfo->uusData, uusInfo->uusLength);
    }
    s_pendingDialing = true;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&tid_dial, &attr, dialThread, NULL);
    if (ret < 0) {
        RLOGE ("dialThread err. \n");
    }
}


// ril has delayed 500ms after CLCC cmd, then to send RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
static int POLL_DELAY_MSEC = 1000; 
static Parcel s_getCurrentCallsParcel_0;
static bool s_needsPollDelay_0 = false;

static void * pollCurrentCallThread(void *arg) {
	RLOGI("pollCurrentCallThread  s_existCall=%d", s_existCall);
    usleep(POLL_DELAY_MSEC * 200);
    RLOGI("pollCurrentCallThread  after sleep");
    if (s_existCall) {
		sendRequest (s_getCurrentCallsParcel_0);
	}
}

static void requrstGetCurrentCalls (int trioID) {
    int ret = 0;
    pthread_t tid_getCalls;
	pthread_attr_t attr;
	
    s_getCurrentCallsParcel_0.request = RIL_REQUEST_GET_CURRENT_CALLS;
    s_getCurrentCallsParcel_0.token = token;
    s_getCurrentCallsParcel_0.rildID= trioID;
    
    s_getCurrentCallsParcel_0.index = s_getCurrentCallsParcel_0.data;
    memset(s_getCurrentCallsParcel_0.data, 0, sizeof(s_getCurrentCallsParcel_0.data));
    
    RLOGI("requrstGetCurrentCalls");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&tid_getCalls, &attr, pollCurrentCallThread, NULL);
    if (ret < 0) {
        RLOGE ("pollCurrentCallThread err. \n");
    }
}

static void resetCallstateRild_0 () {
    RLOGI("resetCallstateRild_0");
    free(s_pCurrentCall_0);
    s_pCurrentCall_0 = NULL;
}

static void processCallStateChange (RIL_Call * p_calls, int trioID) {
    RLOGI("processCallStateChange");
    
    if (trioID == RILD_0){
        RIL_CallState   oldState;
        RIL_CallState   newState;
        
        if (s_pCurrentCall_0 == NULL) {
            //after dial, first state.
            traceSetShortVoiceCallMsg(TRIORAIL_CALL_START, s_trace_short_call_msg);
            return;
        } else {
            oldState = s_pCurrentCall_0->state;
        }
        RLOGD("parserCallList oldState=%d", oldState);
        
        if (p_calls == NULL) {
            if ((oldState == RIL_CALL_ACTIVE) || (oldState == RIL_CALL_HOLDING)) {
                traceSetShortVoiceCallMsg(TRIORAIL_CALL_END_ERR, s_trace_short_call_msg);
                sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_0_NO_CARRIER_IND, TRIO_RIL_E_SUCCESS);
            } else if ((oldState == RIL_CALL_DIALING) || (oldState == RIL_CALL_ALERTING)) {
                traceSetShortVoiceCallMsg(TRIORAIL_CALL_CONNECT_ERR, s_trace_short_call_msg);
                sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_0_DIAL_RSP, TRIO_RIL_E_GENERIC_FAILURE);
            }
            return;
        } else {
            newState = p_calls->state;
        }
        RLOGD("parserCallList newState=%d", newState);
        
        if ((newState == RIL_CALL_DIALING) || (newState == RIL_CALL_ALERTING)){
            s_needsPollDelay_0 = true;
        } else if (newState == RIL_CALL_ACTIVE){
            //sometimes we can not get alerting msg.
            if (oldState != RIL_CALL_ALERTING) {
                RLOGE("parserCallList missing RIL_CALL_ALERTING!");
            }
            s_needsPollDelay_0 = false;
            traceSetShortVoiceCallMsg(TRIORAIL_CALL_CONNECT_OK, s_trace_short_call_msg);
            sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_0_DIAL_RSP, TRIO_RIL_E_SUCCESS);
        }
    } else if (trioID == RILD_1) {
        //RFU
    }
}

/* only conside the case of 1 way call, other cases will be ignored.*/
static void parserCallList(Parcel p) {
    int num;
    RIL_Call call[7];
    RIL_Call *p_calls = &call[0];
    RIL_CallState   state;
    
    num =  readInt32FromParcel(p);
    RLOGI("parserCallList num=%d", num);
    
    if (num == 0) {
        //no call.
        if (p.rildID == RILD_0){
            processCallStateChange(NULL, p.rildID);
            resetCallstateRild_0();
        } else if (p.rildID = RILD_1){
            //
        }        
    } else if (num == 1) {
        p_calls->state = (RIL_CallState)readInt32FromParcel(p);
        p_calls->index = readInt32FromParcel(p);
        p_calls->toa = readInt32FromParcel(p);
        p_calls->isMpty = readInt32FromParcel(p);    
        p_calls->isMT = readInt32FromParcel(p);
        p_calls->als = readInt32FromParcel(p);
        p_calls->isVoice = readInt32FromParcel(p);
        
        if (p.rildID == RILD_0) {
            if (s_pCurrentCall_0 == NULL) {
                processCallStateChange(p_calls, p.rildID);
                s_pCurrentCall_0 = (RIL_Call *)malloc(sizeof(RIL_Call));
                memcpy(s_pCurrentCall_0, p_calls, sizeof(RIL_Call));
            }
            if (p_calls->state != s_pCurrentCall_0->state) {
                processCallStateChange(p_calls, p.rildID);
                memcpy(s_pCurrentCall_0, p_calls, sizeof(RIL_Call));
            }
            if (s_needsPollDelay_0){
                //requrstGetCurrentCalls(RILD_0);
            }
        } else if (p.rildID == RILD_1) {
            //RFU
        }
        
    } else if (num > 1) {
        //RFU
        RLOGE("parserCallList num=%d", num);
        for (int i = 0 ; i < num ; i++) {
            //save the state of all the calls.
        };
    }

}


static void acceptCall (int trioID) {
    Parcel parcel;
    parcel.request = RIL_REQUEST_ANSWER;
    parcel.token = token;
    parcel.rildID= trioID;
    
    parcel.index = parcel.data;
    memset(parcel.data, 0, sizeof(parcel.data));
    
    RLOGI("acceptCall");
    sendRequest (parcel);
}


static void hangupConnection (int callID, int trioID) {
    Parcel parcel;
    parcel.request = RIL_REQUEST_HANGUP;
    parcel.token = token;
    parcel.rildID= trioID;
    
    parcel.index = parcel.data;
    memset(parcel.data, 0, sizeof(parcel.data));
    
    RLOGI("hangupConnection: gsmIndex= %d" , callID);

    writeInt32ToParcel(parcel, 1);
    writeInt32ToParcel(parcel, callID);

    sendRequest (parcel);
    s_pendingDialing = false;
    s_existCall = false;
}

static void getModemStatus (int trioID) {
    Parcel parcel;
    parcel.request = RIL_REQUEST_GET_SIM_STATUS;
    parcel.token = token;
    parcel.rildID= trioID;
    
    parcel.index = parcel.data;
    memset(parcel.data, 0, sizeof(parcel.data));
    
    RLOGI("getModemStatus: trioID= %d" , trioID);

    sendRequest (parcel);
}


static int 
setParcelData(Parcel &p, void *buffer, size_t buflen) {
    if (buflen != sizeof(Parcel)) {
        return -1;
    }
    memset(p.data, 0, PARCEL_LENGTH);
    memcpy(&p, buffer, buflen);
    p.index = p.data;
    return  0;   
}

static void addLinstenFd(int fd, socketFdID id)
{
    pthread_mutex_lock(&s_fdMutex);
    if (fd <= 0){
        RLOGE("addLinstenFd fd<=0.");
        pthread_mutex_unlock(&s_fdMutex);
        return;
    }
    FD_SET(fd, &readFds);
    if (fd >= nfds){
        nfds = fd+1;
    } 
    fd_readSet[id] = fd;
    
    pthread_mutex_unlock(&s_fdMutex);
    RLOGD("addLinstenFd end.");
}

static void removeLinstenFd(int fd, socketFdID id)
{
    if (fd <= 0){
        RLOGE("removeLinstenFd fd<=0.");
        return;
    }
    FD_CLR(fd, &readFds);
    fd_readSet[id] = 0;

    int n = 0;
    for (int i = 0; i < MAX_FD_EVENTS; i++) {
        if (fd_readSet[i] > n) {
            n = fd_readSet[i];
        } 
    }
    nfds = n + 1;
    RLOGI("removeLinstenFd  nfds = %d", nfds);
}

static void
processUnsolicited (Parcel &p) {
    int response;
   
    response = p.unsolResponse;
    RLOGI("processUnsolicited response: %d", response);
    switch(response) {
        case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED:
/*
RIL_CALL_RING = 0,
RIL_CALL_NO_CARRIER,
RIL_CALL_CCWA
*/		
        {
            int num;
            int callState;
            num =  readInt32FromParcel(p);
            callState = readInt32FromParcel(p);
            RLOGI("RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED  num: %d,  callState: %d", num, callState);

            if (p.rildID == RILD_0){
                switch (callState) {
                    RLOGI("RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, callState = %d", callState);
                    case RIL_CALL_NO_CARRIER:
                    case RIL_CALL_RING:
                    case RIL_CALL_CCWA:
                        break;
                    default:
                        break;
                }
                requrstGetCurrentCalls(RILD_0);
            }else if (p.rildID == RILD_1) {
                switch (callState) {
                    case RIL_CALL_NO_CARRIER:
                        break;
                    case RIL_CALL_RING:
                        //sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_1_INCOMING_CALL_IND, RIL_E_SUCCESS);
                        acceptCall(RILD_1);
                        break;
                    case RIL_CALL_CCWA:
                        break;
                    default:
                        break;
                }
            } else {
                //error.
            }
        }
            break;
        case  RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED:
            RLOGI("RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED  ");
            break;
        case RIL_UNSOL_AT_STRING_IND:
            RLOGI("RIL_UNSOL_AT_STRING_IND  ");
            if (p.rildID == RILD_0){
                int dir = readInt32FromParcel(p);
                char * str = readStringFromParcel(p);
                RLOGI("RIL_UNSOL_AT_STRING_IND: dir=%d, AT=%s", dir, str);
                traceSetATString(dir, str);
                free(str);
            }else if (p.rildID == RILD_1) {
            }
            break;
            
        default:
            /* not concern other UNSOL msg now.*/
            break;
    }
}

typedef enum{
    CARDSTATE_ABSENT,
    CARDSTATE_PRESENT,
    CARDSTATE_ERROR,
    CARDSTATE_UNKNOW,
}CardState;

static CardState getCardState(int state) {
    CardState cardState;
     switch(state) {
     case 0:
         cardState = CARDSTATE_ABSENT;
         break;
     case 1:
         cardState = CARDSTATE_PRESENT;
         break;
     case 2:
         cardState = CARDSTATE_ERROR;
         break;
     default:
         cardState = CARDSTATE_UNKNOW;
     }
 }


static void
processSolicited (Parcel &p) {
    int serial, error, request;

    serial = p.token;
    error = p.err;
    request = p.request;
/*    
    if (serial != token) {
        RLOGE("Unexpected solicited response! sn: %d,  error: %d", serial , error);
        return;
    }
*/    
    RLOGD("processSolicited  sn: %d,  request: %d", serial , request);
      
    switch (request) {
        case RIL_REQUEST_RADIO_POWER:
            break;
        case RIL_REQUEST_SIGNAL_STRENGTH:
            break;
        case RIL_REQUEST_VOICE_REGISTRATION_STATE:
            break;
        case RIL_REQUEST_GET_SIM_STATUS:
        {
            int state = readInt32FromParcel(p);
            CardState simstate = getCardState(state);
            RLOGI("RIL_REQUEST_GET_SIM_STATUS rildID=%d, simstate=%d.", p.rildID, simstate);
            
            if (p.rildID == RILD_0){
                if (simstate == CARDSTATE_PRESENT) {
                    sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_0_STATUS_RSP, TRIO_RIL_E_SUCCESS);
                } else {
                    sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_0_STATUS_RSP, TRIO_RIL_E_TRIO_0_NO_SIM);
                }
            } else if (p.rildID == RILD_1){
                if (simstate == CARDSTATE_PRESENT) {
                    sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_1_STATUS_RSP, TRIO_RIL_E_SUCCESS);
                } else {
                    sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_1_STATUS_RSP, TRIO_RIL_E_TRIO_1_NO_SIM);
                }
            }
        }
            break;
        case RIL_REQUEST_DIAL:
            if (p.rildID == RILD_0){
                if (error ==  TRIO_RIL_E_SUCCESS) {
                    //
                    RLOGI("RIL_REQUEST_DIAL OK.");
                    requrstGetCurrentCalls(RILD_0);
                } else {
                    RLOGE("RIL_REQUEST_DIAL  failed!");
                    error =  TRIO_RIL_E_GENERIC_FAILURE;
                    traceSetShortVoiceCallMsg(TRIORAIL_CALL_START, s_trace_short_call_msg);
                    traceSetShortVoiceCallMsg(TRIORAIL_CALL_CONNECT_ERR, s_trace_short_call_msg);
                    sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_0_DIAL_RSP, (RIL_triorail_Errno)error);
                }
                
            } else if (p.rildID == RILD_1){
                //
            } else {
                RLOGE("RIL_REQUEST_DIAL  RILD ID is error!");
            }
            break;
        case RIL_REQUEST_GET_CURRENT_CALLS:
            if (p.rildID == RILD_0){
                if (error ==  TRIO_RIL_E_SUCCESS) {
                    RLOGI("RIL_REQUEST_GET_CURRENT_CALLS.");
                    parserCallList(p);
                } else {
                    resetCallstateRild_0();
                }
                
            } else if (p.rildID == RILD_1){
                //
            } else {
                RLOGE("RIL_REQUEST_GET_CURRENT_CALLS  RILD ID is error!");
            }
            break;
        case RIL_REQUEST_HANGUP:
            if (p.rildID == RILD_0){
                if (error ==  TRIO_RIL_E_SUCCESS) {
                    //
                    RLOGI("RIL_REQUEST_HANGUP SUCCESS.");
                    traceSetShortVoiceCallMsg(TRIORAIL_CALL_END_OK, s_trace_short_call_msg);
                } else {
                    char * pResult;
                    pResult = readStringFromParcel(p);
                    RLOGE("RIL_REQUEST_HANGUP  failed: %s", pResult);
                    error =  TRIO_RIL_E_GENERIC_FAILURE;
                    free(pResult);
                    traceSetShortVoiceCallMsg(TRIORAIL_CALL_END_ERR, s_trace_short_call_msg);
                }
                resetCallstateRild_0();
                sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_0_HANGUP_RSP, (RIL_triorail_Errno)error);
            } else {
                //
            }

            break;
        case RIL_REQUEST_ANSWER:
            if (p.rildID == RILD_0){
                //
            } else if (p.rildID == RILD_1){
                RLOGI("RIL_REQUEST_ANSWER SUCCESS.");
                traceSetIncomingShortVoiceCallMsg(0, RILD_1);
            }
            break;
    }
                
}


static void
processRilResponse (Parcel &p) {
    int type;

    type = p.responseType;

    if (type == RESPONSE_UNSOLICITED) {
        processUnsolicited (p);
    } else if (type == RESPONSE_SOLICITED) {
        processSolicited (p);
    }
}

static void processReadRil_0(int fd)
{
    static RecordStream *  p_rs = NULL; 
    void *p_record;
    size_t recordlen;
    int ret;
    
    RLOGI("processReadRil_0");

    if (p_rs == NULL) {
        p_rs = record_stream_new(fd, MAX_COMMAND_BYTES);
    }
    for (;;) {
        Parcel p;
        /* loop until EAGAIN/EINTR, end of stream, or other error */
        ret = record_stream_get_next(p_rs, &p_record, &recordlen);

        if (ret == 0 && p_record == NULL) {
            /* end-of-stream */
            break;
        } else if (ret < 0) {
            break;
        } else if (ret == 0) { 
            ret = setParcelData(p, p_record, recordlen);
            p.rildID = RILD_0;
            
            if (ret != 0) {
                RLOGE("error on setParcelData");
                return;
            } 
            processRilResponse(p);
        }
    }
    if (ret == 0 || !(errno == EAGAIN || errno == EINTR)) {
        /* fatal error or end-of-stream */
        if (ret != 0) {
            RLOGE("error on reading command socket errno:%d\n", errno);
        } else {
            RLOGI("EOS. socket errno:%d", errno);
            exit(-1);
        }
        record_stream_free(p_rs);
        p_rs = NULL; 
    }
}

static void processReadRil_1(int fd)
{
    static RecordStream *  p_rs = NULL; 
    void *p_record;
    size_t recordlen;
    int ret;
    
    RLOGI("processReadRil_1");

    if (p_rs == NULL) {
        p_rs = record_stream_new(fd, MAX_COMMAND_BYTES);
    }
    for (;;) {
        Parcel p;
        /* loop until EAGAIN/EINTR, end of stream, or other error */
        ret = record_stream_get_next(p_rs, &p_record, &recordlen);
        RLOGI("processReadRil_1 ret=%d", ret);

        if (ret == 0 && p_record == NULL) {
            /* end-of-stream */
            break;
        } else if (ret < 0) {
            break;
        } else if (ret == 0) { 
            ret = setParcelData(p, p_record, recordlen);
            p.rildID = RILD_1;
            if (ret != 0) {
                RLOGE("error on setParcelData");
                return;
            } 
            processRilResponse(p);
        }
    }
    if (ret == 0 || !(errno == EAGAIN || errno == EINTR)) {
        /* fatal error or end-of-stream */
        if (ret != 0) {
            RLOGE("error on reading command socket errno:%d\n", errno);
        } else {
            RLOGI("EOS. socket errno:%d", errno);
            exit(-1);
        }
        record_stream_free(p_rs);
        p_rs = NULL; 
    }
}

static void processReadTrace_0(int fd)
{
/*
struct trace_cmd_frame_rsp {
	uint32_t token;
	uint32_t cmd;
	uint32_t status;
};

*/
    uint32_t cmdHead[3] = {0}; 
    uint32_t token;
    uint32_t cmd;
    uint32_t status;

    int HAL_cmd;
    RIL_triorail_Errno error;
    RLOGI("processReadTrace_0");
	
	int len = recv(fd, &cmdHead, sizeof(cmdHead), 0);
	RLOGI("processReadTrace_0 len=%d ", len);
	if (len == 0) {
        RLOGE ("processReadTrace_0  error, EOS");
        exit(-1);
        return;
    }
    
    if (len != sizeof(cmdHead)) {
        RLOGE ("processReadTrace_0, error reading on socket: cmdHead");
        return;
    }
    token = cmdHead[0];
    cmd = cmdHead[1];
    status = cmdHead[2];
    RLOGI("processReadTrace_0 token=%d cmd=%d status=%d", token, cmd, status);

    switch (cmd) {
        case TRACE_GET_STATUS:
            HAL_cmd = RIL_TRIO_0_TRACE_STATUS_RSP;
            break;
        case TRACE_ON:
            HAL_cmd = RIL_TRIO_0_TRACE_START_RSP;
            break;
        case TRACE_OFF:
            HAL_cmd = RIL_TRIO_0_TRACE_END_RSP;
            break;   
        default:
            RLOGE ("error cmd.");
            return;
    }

    switch (status) {
        case TRACE_OK:
            error = TRIO_RIL_E_SUCCESS;
            break;
        case TRACE_ERR_OPEN_DEV:
        case TRACE_ERR_OPEN_FILE:
            error = TRIO_RIL_E_TRIO_0_OPEN_PORT_ERR;
            break;   
        default:
            RLOGE ("error status.");
            error = TRIO_RIL_E_TRIO_0_UNKNOW_ERR;
            break;
    }

    sendHALResponse(MODULE_USER_CLIENT, HAL_cmd, error);

}



static bool g_dialReq = false;
static char g_dialAddress[30] = {0};


static bool processHALCmd(int fd)
{
    unsigned short cmdHead[4] = {0}; 
    unsigned short host;
    unsigned short target;
    unsigned short cmd;
    unsigned short len;
    unsigned char * data;

    void *p_record;
    size_t recordlen;
    int ret;
    int readLen = 0;
    RLOGD("processHALCmd");

	len = recv(fd, &cmdHead, sizeof(cmdHead), 0);
	RLOGD("processHALCmd len=%d ", len);
    
    if (len == 0) {
        RLOGE ("processHALCmd  error, EOS");
        exit(-1);
        return false;
    }
    if (len != sizeof(cmdHead)) {
        RLOGE ("processHALCmd, error reading on socket: cmdHead");
        return false;
    }
    //only use cmd now.
    cmd = cmdHead[2];
    len = cmdHead[3];
    RLOGD("processHALCmd cmd=%d len=%d", cmd, len);
    
    if (len !=0) {
        data = (unsigned char *) malloc(sizeof(char) * len);
        do {
            readLen =  recv(fd, data, sizeof(char) * len, 0);
            RLOGD("processHALCmd readLen=%d", readLen);
            if(readLen  <=  0) {
            	usleep(1000*20);
            	continue;
            }
            if ( readLen != (int)sizeof(char) * len) {
            	RLOGE ("error reading on socket: data[]");
            	free(data);
            	return false;
            } else {
            	break;
            }
        } while(1);
    }
    switch (cmd) {
        case CORE_MSG_REGISTER_RSP:
            RLOGI("cmd: CORE_MSG_REGISTER_RSP.");
            break;
           
        case RIL_TRIO_TASK_CFG_IND:
            {
                RLOGI("cmd: RIL_TRIO_TASK_CFG_IND.");
                RIL_task_config_ind * pTask = (RIL_task_config_ind *)data;
                
                s_GPS_Info.direction = pTask->direction;
                memset(&s_taskConfig_Info, 0, sizeof(trace_task_config_ind));
                memcpy(&s_taskConfig_Info, &(pTask->direction), sizeof(trace_task_config_ind));
                RLOGD("s_taskConfig_Info direction=%d", s_taskConfig_Info.direction);
                RLOGD("s_taskConfig_Info Trio_0_taskConfig=%d", s_taskConfig_Info.Trio_0_taskConfig);
                RLOGD("s_taskConfig_Info Trio_0_testType=%d", s_taskConfig_Info.Trio_0_testType);
                RLOGD("s_taskConfig_Info Trio_0_modemType=%d", s_taskConfig_Info.Trio_0_modemType);
                RLOGD("s_taskConfig_Info Trio_0_callType=%d", s_taskConfig_Info.Trio_0_callType);
                RLOGD("s_taskConfig_Info Trio_0_callDuration=%d", s_taskConfig_Info.Trio_0_callDuration);

                RLOGD("s_taskConfig_Info Trio_1_taskConfig=%d", s_taskConfig_Info.Trio_1_taskConfig);
                RLOGD("s_taskConfig_Info Trio_1_testType=%d", s_taskConfig_Info.Trio_1_testType);
                RLOGD("s_taskConfig_Info Trio_1_modemType=%d", s_taskConfig_Info.Trio_1_modemType);
                RLOGD("s_taskConfig_Info Trio_1_callType=%d", s_taskConfig_Info.Trio_1_callType);
                
                sendTraceReq (TRACE_TASK_CONFIG_IND, &s_taskConfig_Info, sizeof(trace_task_config_ind));
                memset(&s_trace_call_statistic_msg, 0, sizeof(trace_short_call_statistic_ind));

            }
            break;
            
        case GPS_MSG_GET_GPS_INFO_CMD:
            {
                static int i = 0;
                i++;
                
                gps_info_t * pGPS = (gps_info_t *)data;
                s_GPS_Info.KmPosition = pGPS->kmposition;
                s_GPS_Info.Speed = pGPS->speed;
                memcpy(&(s_GPS_Info.latitude), &(pGPS->latitude), sizeof(uint64_t));
                memcpy(&(s_GPS_Info.longitude), &(pGPS->longitude), sizeof(uint64_t));
                if (i == 60*2*2) {
                    RLOGI("cmd: GPS_MSG_GET_GPS_INFO_CMD.");
                    RLOGI("GPS: kmposition=%lld", pGPS->kmposition);
                    RLOGI("GPS: speed=%lld", pGPS->speed);
                    RLOGI("GPS: latitude=%lf", pGPS->latitude);
                    RLOGI("GPS: longitude=%lf", pGPS->longitude);
                    i = 0;
                }
#if 0                
                RLOGD("s_GPS_Info kmposition=%lld", pGPS->kmposition);
                RLOGD("s_GPS_Info Speed=%lld", pGPS->speed);
                RLOGD("s_GPS_Info latitude=%lf", pGPS->latitude);
                RLOGD("s_GPS_Info longitude=%lf", pGPS->longitude);
#endif
                sendTraceReq (TRACE_GPS_IND, &s_GPS_Info, sizeof(trace_GPS_Info_ind));
            }
            break;
        
        case RIL_TRIO_0_DIAL_REQ:
            {
                RLOGI("cmd: RIL_TRIO_0_DIAL_REQ.");
                RIL_dial_req * pDilReq = (RIL_dial_req *)data;
                memset(&s_trace_short_call_msg, 0, sizeof(trace_short_call_ind));
                //            dial(pDilReq->address, pDilReq->clir, NULL, RILD_0);
                requestDial(pDilReq->address, 0, NULL, RILD_0);             
            }
            break;
        case RIL_TRIO_0_HANGUP_REQ:
            {
                RLOGI("cmd: RIL_TRIO_0_HANGUP_REQ.");
                RIL_hangup_req * pHangupReq = (RIL_hangup_req *)data;
                hangupConnection(pHangupReq->callID, RILD_0);
            }
            break;
        case RIL_TRIO_0_STATUS_REQ:
            RLOGI("cmd: RIL_TRIO_0_STATUS_REQ.");
            if (s_rild_0_status == TRIO_RIL_E_TRIO_0_UNKNOW_ERR) {
                sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_0_STATUS_RSP, s_rild_0_status);
            } else {
                getModemStatus(RILD_0);
            }
            
            break;
        case RIL_TRIO_1_ANSWER_REQ:
            RLOGI("cmd: RIL_TRIO_1_ANSWER_REQ.");
            acceptCall (RILD_1); 
            break;
        case RIL_TRIO_1_STATUS_REQ:
            RLOGI("cmd: RIL_TRIO_1_STATUS_REQ.");
            
            if (s_rild_0_status == TRIO_RIL_E_TRIO_1_UNKNOW_ERR) {
                sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_1_STATUS_RSP, s_rild_1_status);
            } else {
                getModemStatus(RILD_1);
            }
            
            break;
        case RIL_TRIO_0_TRACE_START_REQ:
            {
                RLOGI("cmd: RIL_TRIO_0_TRACE_START_REQ.");
                RIL_trace_start_req * pReq = (RIL_trace_start_req *)data;
                RLOGI("trace file name=%s", pReq->traceFileName);
                
                traceStart(pReq);
            }
            
            break;
        case RIL_TRIO_0_TRACE_END_REQ:
            {
                RLOGI("cmd: RIL_TRIO_0_TRACE_END_REQ.");
                RIL_trace_end_req * pReq = (RIL_trace_end_req *)data;
                RLOGI("trace file name=%s", pReq->traceFileName);
                traceEnd(pReq);
			}
            break;
        case RIL_TRIO_0_TRACE_STATUS_REQ:
            RLOGI("cmd: RIL_TRIO_0_TRACE_STATUS_REQ.");
            if (s_trace_0_status == TRIO_RIL_E_TRIO_1_UNKNOW_ERR) {
                sendHALResponse(MODULE_USER_CLIENT, RIL_TRIO_0_TRACE_STATUS_RSP, s_trace_0_status);
            } else {
                traceGetStatus();
            }
            break;

        default:
            RLOGE ("error cmd.");
            break;
    }
    if (len !=0) {
		free(data);
	}
    return true;
}

static void * trioReaderLoop(void *arg)
{
    int n;
    fd_set rfds;
    int ret;
    int cmd = 0;
    
    for (;;) {
        RLOGD("trioReaderLoop .\n");  
        // make local copy of read fd_set
        memcpy(&rfds, &readFds, sizeof(fd_set));
        n = select(nfds, &rfds, NULL, NULL, NULL);
        RLOGD("trioReaderLoop select return: %d", n);
        if (n < 0) {
            if (errno == EINTR) continue;
            RLOGE("trioReaderLoop: select error (%d)", errno);
            return NULL;
        }
         
        // Check for read-ready
        if((fd_readSet[RILD_0]>0) && FD_ISSET(s_fdRild_0, &rfds)) {
            processReadRil_0(s_fdRild_0);
        }
        if((fd_readSet[RILD_1]>0) && FD_ISSET(s_fdRild_1, &rfds)) {
            processReadRil_1(s_fdRild_1);         
        }
        if((fd_readSet[TRACE_0]>0) && FD_ISSET(s_fdTrace_0, &rfds)) {
            processReadTrace_0(s_fdTrace_0);        
        }
        if((fd_readSet[HAL_CMD]>0) && FD_ISSET(s_fdTrioHalCmd, &rfds)) {
            ret = processHALCmd(s_fdTrioHalCmd);  
            if (!ret) {
                RLOGE("trioReaderLoop: HAL socket read err.");
                break;
            }
        }
    }
    RLOGE ("error in trioReaderLoop errno:%d", errno);
    // kill self to restart on error
	kill(0, SIGKILL);
}


static int createClientSocket(const char * socketName)
{
    int fd = 0;
    int ret = 0;
    
    RLOGI("creatSocket : %s.", socketName);
    fd = socket(AF_UNIX, SOCK_STREAM, 0);  
    if (fd < 0) {
        RLOGE("opening socket %s failed.", socketName);
        return fd;
    }

    struct sockaddr_un address;  
    address.sun_family = AF_UNIX;  
    strcpy(address.sun_path, socketName);  
       
    /* connect to the server */  
    ret = connect(fd, (struct sockaddr *)&address, sizeof(address));  
    if(ret == -1)  
    {  
        RLOGE("connect socket failed: %s.", socketName);  
        return ret;
    }  
     
    ret = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (ret < 0) {
        RLOGE ("Error setting O_NONBLOCK errno:%d", errno);
        return ret;
    }
    return fd;
}

static int createServerSocket(const char * socketName)
{
    int fd = 0;
    int ret = 0;

    /* delete the socket file */  
    unlink(socketName);  

    /* create a socket */  
    fd = socket(AF_UNIX, SOCK_STREAM, 0);  

    struct sockaddr_un server_addr;  
    server_addr.sun_family = AF_UNIX;  
    strcpy(server_addr.sun_path, socketName);  

    /* bind with the local file */  
    bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)); 

    ret = listen(fd, 4);

    if (ret < 0) {
        RLOGE("Failed to listen on control socket '%d': %s",
             fd, strerror(errno));
        return ret;
    }
    return fd;
}

#if 1
int main(int argc, char *argv[])
{
    int ret = 0;
     
    static pthread_t s_tid_reader;
    
    /* create a socket */  
    s_fdRild_0 = createClientSocket(SOCKET_NAME_RIL_0);  
    if (s_fdRild_0 <= 0) {
        s_rild_0_status = TRIO_RIL_E_TRIO_0_UNKNOW_ERR;
    }

    s_fdRild_1 = createClientSocket(SOCKET_NAME_RIL_1);  
    if (s_fdRild_1 <= 0) {
        s_rild_1_status = TRIO_RIL_E_TRIO_1_UNKNOW_ERR;
    }

    s_fdTrace_0 = createClientSocket(SOCKET_NAME_TRACE_0);  
    if (s_fdTrace_0 <= 0) {
        s_trace_0_status= TRIO_RIL_E_TRIO_0_UNKNOW_ERR;
    }

    s_fdTrioHalCmd = createClientSocket(SOCKET_NAME_HAL_TRIORAIL); 
    if (s_fdTrioHalCmd <= 0) {
        RLOGE("could not creat socket: %s.", SOCKET_NAME_HAL_TRIORAIL); 
        //exit (-1);
    } else {
        // register to uplayer.
        sendHALResponse(MODULE_FRAMEWORK_CORE, CORE_MSG_REGISTER_CMD, TRIO_RIL_E_MAX);            
    }

    FD_ZERO(&readFds);
    addLinstenFd(s_fdRild_0, RILD_0);
    addLinstenFd(s_fdRild_1, RILD_1);
    addLinstenFd(s_fdTrace_0, TRACE_0);
    addLinstenFd(s_fdTrioHalCmd, HAL_CMD);
    
    ret = pthread_create(&s_tid_reader, NULL, trioReaderLoop, NULL);

    if (ret < 0) {
        RLOGE ("pthread_create err. \n");
        exit(-1);
    }
        
    while(1) {
        // sleep(UINT32_MAX) seems to return immediately on bionic
        sleep(0x00ffffff);
    }
}
#else //test
int main(int argc, char *argv[])
{
    int fd;
    int num_socket_args = 0;
    int i  = 0;
    int ret = 0;
    int option = 0;
    int clir = 0; /*subscription default*/
     
    static pthread_t s_tid_reader;
    
    if(error_check(argc, argv)) {
        print_usage();
        exit(-1);
    }

    /* create a socket */  
    fd = socket(AF_UNIX, SOCK_STREAM, 0);  
    if (fd < 0) {
        RLOGE("opening socket %s failed.", SOCKET_NAME_RIL);
        exit(-1);
    }

    struct sockaddr_un address;  
    address.sun_family = AF_UNIX;  
    strcpy(address.sun_path, SOCKET_NAME_RIL);  
       
    /* connect to the server */  
    ret = connect(fd, (struct sockaddr *)&address, sizeof(address));  
    if(ret == -1)  
    {  
        RLOGE("connect failed.");  
        exit(1);  
    }  
     
    s_fdCommand = fd;
    ret = fcntl(s_fdCommand, F_SETFL, O_NONBLOCK);
    if (ret < 0) {
        RLOGE ("Error setting O_NONBLOCK errno:%d", errno);
        exit(1);
    }
    
    FD_ZERO(&readFds);
    FD_SET(s_fdCommand, &readFds);
    if (s_fdCommand >= nfds) {
        nfds = s_fdCommand+1;
    }
    ret = pthread_create(&s_tid_reader, NULL, rilReaderLoop, NULL);

    if (ret < 0) {
        RLOGE ("pthread_create err. \n");
        exit(-1);
    }
        
    option = atoi(argv[1]);
    switch (option) {
        case RADIO_RESET:
            RLOGI ("debug port: RADIO_RESET.");

            break;
        case RADIO_OFF:
            RLOGI ("debug port: RADIO_OFF.");
            break;
        case RADIO_ON:
            RLOGI ("Debug port: RADIO_ON.");
            break;

        case SIM_STATUS:
            RLOGI("Debug port: SIM_STATUS");
            sleep(2);
            break;
        case VOICE_REGISTRATION_STATE:
            RLOGI("Debug port: VOICE_REGISTRATION_STATE");
            break;
        case SIGNAL_STRENGTH:
            RLOGI("Debug port: SIGNAL_STRENGTH");
            break;
        case DIAL_CALL:
            clir = 0; /*subscription default*/
            RLOGI("Debug port: Dial Call, address: %s\n", argv[2]);

            dial(argv[2], clir, NULL);
            break;
        case ANSWER_CALL:
            RLOGI("Debug port: Answer Call");

            break;
        case END_CALL:
            RLOGI("Debug port: End Call");
            hangupConnection (1);

            break;
        default:
            RLOGI ("Invalid request");
            break;
    }
    
    pthread_mutex_lock(&s_commandmutex);
    pthread_cond_wait(&s_commandcond, &s_commandmutex);
    pthread_mutex_unlock(&s_commandmutex);

    close(fd);
     RLOGI ("main exit.");
    return 0;
}
#endif
