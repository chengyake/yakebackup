
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>  
#include <sys/un.h>  
#include <netinet/in.h>
#include <sched.h>

#include <utils.h>
#include <Triorail_trace.h>
#include <Triorail_trace_internal.h>
#include <Triorail_trace_stream.h>

#define LOG_TAG "TRIO_TRACE"
#include <log.h>

#define     _GNU_SOURCE


#ifdef PROJECT_TI
const char * s_device_path_trace = "/dev/ttyUSB2"; 
#elif defined PROJECT_FSL
const char * s_device_path_trace = "/dev/ttymxc1"; 
#else
const char * s_device_path_trace = "/dev/ttyUSB0"; 
#endif



#define SOCKET_NAME_TRACE_0 "trio-trace-0"	//HAL layer


#define  MAX_TRACE_BUFFER 1024*8
#define TEST_TRACE_BUFFER  100

typedef enum {
    AT_LOG = 0,
    TRACE_LOG = 1
} log_type;

trace_GPS_Info_ind g_GPS_Info = {0,0,0,0,0};
trace_task_config_ind g_taskConfig_Info;


static int s_token = 0;

static char s_TraceBuffer[MAX_TRACE_BUFFER];

static fd_set readSocketFds;
static int nfds = 0;
static trace_status s_trace_status = TRACE_OK;

static int s_fdTrioHALListen;
static int s_fdTrioHalAccept;

char s_openFileName[100] = {0};

static FILE *s_fpTraceFile = NULL;
static int s_fdTraceDev = 0;

static pthread_t s_tid_readHAL;
static pthread_t s_tid_readTrace;
static pthread_t s_tid_processTrace;

static pthread_mutex_t s_writeSocketMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_writeFileMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_frameListMutex = PTHREAD_MUTEX_INITIALIZER;


static pthread_mutex_t s_frameMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_frameCond = PTHREAD_COND_INITIALIZER;

static bool s_trace_open = false;
static Trio_trace_frame s_traceFrameList;



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

static int sendRspToHAL(trace_cmd  cmd, trace_status status) {
    int ret;
    
/*
struct trace_cmd_frame_rsp {
	uint32_t token;
	uint32_t cmd;
	uint32_t status;
};
*/  
    uint32_t cmdHead[3] = {0};

    if (s_fdTrioHalAccept <= 0) {
        RLOGE("sendTraceReq: s_fdTrace_0=%d", s_fdTrioHalAccept);
        return -1;
    }

    cmdHead[0] = (uint32_t)s_token;
    cmdHead[1] = (uint32_t)cmd;
    cmdHead[2] = (uint32_t)status;
    RLOGI("sendTraceReq: token=%d, cmd=%d, status=%d", s_token, cmd, status);
    
    pthread_mutex_lock(&s_writeSocketMutex);
    ret = blockingWrite(s_fdTrioHalAccept, (void *)cmdHead, sizeof(cmdHead));
    if (ret < 0) {
        pthread_mutex_unlock(&s_writeSocketMutex);
        return ret;
    }
    pthread_mutex_unlock(&s_writeSocketMutex);
    return 0;
}

static void frame_init_list(Trio_trace_frame * list)
{
    memset(list, 0, sizeof( Trio_trace_frame));
    list->next = list;
    list->prev = list;
    list->frameID = 0;
    list->totalFrames = 0;
}

static void frame_addToList(Trio_trace_frame * traceFrame, Trio_trace_frame * list)
{
    pthread_mutex_lock(&s_frameListMutex);
    RLOGI("frame_addToList");
    traceFrame->next = list;
    traceFrame->prev = list->prev;
    traceFrame->prev->next = traceFrame;
    list->prev = traceFrame;
    list->totalFrames++;
    pthread_mutex_unlock(&s_frameListMutex);
}

static void frame_removeFromList(Trio_trace_frame * traceFrame)
{
    pthread_mutex_lock(&s_frameListMutex);
    RLOGI("frame_removeFromList");
    traceFrame->next->prev = traceFrame->prev;
    traceFrame->prev->next = traceFrame->next;
    traceFrame->next = NULL;
    traceFrame->prev = NULL;
    s_traceFrameList.totalFrames--;
    pthread_mutex_unlock(&s_frameListMutex);
}

static void getTraceFrame()
{
    static TraceStream *  p_rs = NULL; 
    void *p_record;
    size_t recordlen;
    int ret;
    
    RLOGD("getTraceFrame begin.");

    if (p_rs == NULL) {
        p_rs = trace_stream_new(s_fdTraceDev, MAX_TRACE_BUFFER);
    }
    
    for (;;) {
        /* loop until EAGAIN/EINTR, end of stream, or other error */
        ret = trace_stream_get_next(p_rs, &p_record, &recordlen);
		RLOGD("getTraceFrame trace_stream_get_next=%d", ret);
		
        if (ret == 0 && p_record == NULL) {
            /* end-of-stream */
            RLOGE("getTraceFrame end-of-stream");
            break;
        } else if (ret < 0) {
            break;
        } else if (ret == 0) { 
            Trio_trace_frame * traFrame;
            RLOGI("getTraceFrame get a full frame.");
         
            traFrame = (Trio_trace_frame  *)malloc (sizeof(Trio_trace_frame ));
            traFrame->timestamp = getRealtimeOfCS();
            memcpy(&(traFrame->gps_data), &g_GPS_Info, sizeof(trace_GPS_Info_ind));
            
            traFrame->dataLen = recordlen;
            traFrame->data = (uint8_t  *)malloc (recordlen);
            memcpy(traFrame->data, p_record, recordlen);
            
            frame_addToList(traFrame, &s_traceFrameList);
            RLOGI("getTraceFrame total=%d", s_traceFrameList.totalFrames);
            
            pthread_mutex_lock(&s_frameMutex);
			pthread_cond_signal(&s_frameCond);
            pthread_mutex_unlock(&s_frameMutex);
#if 0            
            if (!s_trace_open) {
				return ;
			}
#endif
        }
    }
    if (ret == 0 || !(errno == EAGAIN || errno == EINTR)) {
        RLOGE("getTraceFrame err!");
        /* fatal error or end-of-stream */
        trace_stream_free(p_rs);
        p_rs = NULL; 
         
        if (ret != 0) {
            RLOGE("getTraceFrame socket errno:%d\n", errno);
        } else {
            RLOGE("getTraceFrame EOS. socket errno:%d", errno);
            exit(-1);
        }
    }
}

const int READ_CPU_ID = 2;
const int PROCESS_CPU_ID = 3;

static void *readTraceStreamLoop(void *arg)
{
    int n;
    int ret;
    fd_set rfds, readTraceFds;
    struct timeval tv;
    
    RLOGI("readTraceStreamLoop");    
 /*   
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(PROCESS_CPU_ID, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        RLOGE("readTraceStreamLoop, set thread affinity failed\n");
    }
*/

    FD_ZERO(&readTraceFds);
    FD_SET(s_fdTraceDev, &readTraceFds);
    ret = fcntl(s_fdTraceDev, F_SETFL, O_NONBLOCK);
    if (ret < 0) {
        RLOGE ("Error setting O_NONBLOCK errno:%d", errno);
        return NULL;
    }   
                
    for (;;) {
        if (!s_trace_open) {
            RLOGE("readTraceStreamLoop trace closed.\n");
            
            pthread_mutex_lock(&s_frameMutex);
            pthread_cond_signal(&s_frameCond);
            pthread_mutex_unlock(&s_frameMutex);
            return NULL;
        }
        tv.tv_sec=0;
        tv.tv_usec=1000*200;
        memcpy(&rfds, &readTraceFds, sizeof(fd_set));
        n = select(s_fdTraceDev+1, &rfds, NULL, NULL, &tv);
        RLOGD("readTraceStreamLoop select=%d", n);  
        if (n < 0) {
		    RLOGE("readTraceStreamLoop: select error (%d)", errno);
            if (errno == EINTR) continue;
            return NULL;
        } else if (n ==0) {
            //timeout.
            continue;
        } else {
            getTraceFrame();
        }
    }
    RLOGE("readTraceStreamLoop end.\n");
}


static void *processTraceFrameLoop(void *arg)
{
    RLOGI("processTraceFrameLoop");    
    
    cpu_set_t mask;
    int num = sysconf(_SC_NPROCESSORS_CONF);
    RLOGI("system has %d processor(s)\n", num);
/*
    CPU_ZERO(&mask);
    CPU_SET(READ_CPU_ID, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        RLOGE("readTraceStreamLoop, set thread affinity failed\n");
    }
*/

    for (;;) {
        Trio_trace_frame * traFrame = s_traceFrameList.next;
        while (traFrame != &s_traceFrameList) {
            Trio_trace_frame * next = traFrame->next;
  
            RLOGD("processTraceFrameLoop before process.");
            //parser trace frame.
            processOTRByTypes(traFrame);
            RLOGD("processTraceFrameLoop after process.");
            frame_removeFromList(traFrame);
            RLOGI("processTraceFrameLoop total=%d", s_traceFrameList.totalFrames);
            free(traFrame->data);
            free(traFrame); 
            traFrame = next;
        }
        if (!s_trace_open) {
			fclose(s_fpTraceFile);
			s_fpTraceFile = NULL;
            RLOGE("processTraceFrameLoop trace closed.\n");
            return NULL;
        }
        
        pthread_mutex_lock(&s_frameMutex);
        pthread_cond_wait(&s_frameCond, &s_frameMutex);
        pthread_mutex_unlock(&s_frameMutex);
    }
    RLOGE("processTraceFrameLoop end.\n");
}

static bool tty_init(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    struct termios options;

    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd, &options);

    /*
     * Enable the receiver and set local mode
     */
    options.c_cflag |= (CLOCAL | CREAD);
    
//=======set default values
	/*
	 * Select 8 data bits, 1 stop bit and no parity bit
	 */
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        	/*
	 * Disable hardware flow control
	 */
	options.c_cflag &= ~CRTSCTS;
        /*
         * Set the baud rates to 19200= B19200...
         */
        cfsetispeed(&options, B19200);
        cfsetospeed(&options, B19200);

        /*
         * Choosing raw input
         */
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

        /*
         * Disable software flow control
         */
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
        /*
         * Disable hardware flow control
         */
        options.c_cflag &= ~CRTSCTS;
	/*
	 * Choosing raw output
	 */
	options.c_oflag &= ~OPOST;
	
	options.c_iflag &=~(INLCR|IGNCR|ICRNL);
	options.c_oflag &=~(ONLCR|OCRNL);
	/*
	 * Set read timeouts
	 */
	//options.c_cc[VMIN] = 0;
	//options.c_cc[VTIME] = 0;
//=======enf of set default values


    /*
     * Set the baud rates to 19200= B19200...
     */
        cfsetispeed(&options, speed);
        cfsetospeed(&options, speed);

        switch(flow_ctrl)
        {
            case 0 ://Disable  flow control
                  options.c_cflag &= ~CRTSCTS;
                  break;   
            case 1 ://enable hardware flow control
                  options.c_cflag |= CRTSCTS;
                  break;
            case 2 ://enable software flow control
                  options.c_cflag |= IXON | IXOFF | IXANY;
                  break;
            default:
                fprintf(stderr,"Unsupported flow_ctrl\n");  
                return (false);   

        }
        options.c_cflag &= ~CSIZE;  
        switch (databits)  
        {    
            case 5    :  
                options.c_cflag |= CS5;  
                break;  
            case 6    :  
                options.c_cflag |= CS6;  
                break;  
            case 7    :      
                options.c_cflag |= CS7;  
                break;  
            case 8:      
                options.c_cflag |= CS8;  
                break;    
            default:     
                fprintf(stderr,"Unsupported data size\n");  
                return (false);   
        }  
        switch (parity)  
        {    
            case 'n':  
            case 'N': //no parity bit
                 options.c_cflag &= ~PARENB;   
                 options.c_iflag &= ~INPCK;      
                 break;   
            case 'o':    
            case 'O'://odd
                 options.c_cflag |= (PARODD | PARENB);   
                 options.c_iflag |= INPCK;               
                 break;   
            case 'e':   
            case 'E'://even
                 options.c_cflag |= PARENB;         
                 options.c_cflag &= ~PARODD;         
                 options.c_iflag |= INPCK;        
                 break;  
            case 's':  
            case 'S': //space
                 options.c_cflag &= ~PARENB;  
                 options.c_cflag &= ~CSTOPB;  
                 break;   
            default:    
                 fprintf(stderr,"Unsupported parity\n");      
                 return (false);   
        }   

        switch (stopbits)  
        {    
            case 1:     
                options.c_cflag &= ~CSTOPB; 
                break;   
            case 2:     
                options.c_cflag |= CSTOPB;
                break;  
            default:     
                fprintf(stderr,"Unsupported stop bits\n");   
            return (false);  
        }  

        
        if (tcsetattr(fd,TCSANOW,&options) != 0)    
        {  
            RLOGE("com set error!\n");    
            return false;   
        }  
        return true;

}


static FILE * openTraceFile(const char* fileName) {
    FILE *fp;

    RLOGI("openTraceFile: %s", fileName);   
    
    if((fp=fopen(fileName, "a+"))==NULL) {
		RLOGE("fp null");    
    } 
    return fp;
}


void writeFrame(void * data, int len) {
	RLOGI ("writeFrame. ");
    pthread_mutex_lock(&s_writeFileMutex);
    if (s_fpTraceFile == NULL) {
		s_fpTraceFile= openTraceFile(s_openFileName); 
	}
    
    if (s_fpTraceFile == NULL) {
        RLOGE ("open trace file failed!");
        s_trace_status = TRACE_ERR_OPEN_FILE;
        pthread_mutex_unlock(&s_writeFileMutex);
        return;
    }
                
    fwrite(data, len, 1, s_fpTraceFile);
//    fclose(s_fpTraceFile);
//    s_fpTraceFile = NULL;

    pthread_mutex_unlock(&s_writeFileMutex);
    
}

static bool traceInit(void) {
    char * cmd;
    unsigned char initData[] = {0x02, 0x00, 0x00, 0x04, 0x50, 0x01, 0x01, 0x00, 0x54, 0x03};
    unsigned char L2L3Data[] =   {0x02, 0x00, 0x00, 0x06, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x09, 0x10, 0x03};
    unsigned char GSMC_IData[] = {0x02, 0x00, 0x00, 0x06, 0x10, 0x1f, 0x00, 0x00, 0x80, 0x00, 0x89, 0x03};
    unsigned char LayerStateMeasurementL1[]  = {0x02, 0x00, 0x00, 0x06, 0x20, 0x1f, 0x00, 0x00, 0x00, 0x01, 0x38, 0x03 };
    static bool inited = false;
    if (inited) {
        return true;
    }
    RLOGD("traceInit enter!");
    s_fdTraceDev= open (s_device_path_trace, O_RDWR); // | O_NDELAY
    if (s_fdTraceDev < 0) {
        RLOGE ("could not open device: %s. \n", s_device_path_trace);
        return false;
    }

    if (!tty_init(s_fdTraceDev,B57600, 0, 8, 1, 'N') )
    {
        RLOGE ("the device: %s init failed! \n", s_device_path_trace);
        return false;
    }
    asprintf(&cmd, "AT+CTR=2%c", '\r');
    
    int cur = 0;
    int written, len;
    len = strlen(cmd);
    while (cur < len) {
        do {
            written = write (s_fdTraceDev, cmd + cur, len - cur);
            RLOGI("trace port: cmd written  :%d\n",written);
        } while (written < 0 && errno == EINTR);
        
        RLOGI("trace port:  written  errno  :%d\n",errno);
        if (written < 0) {
            RLOGE ("trace write AT cmd err!");
            free(cmd);
            return false;
        }
        cur += written;
    }
    free(cmd);
    
    //usleep(100*1000);
    sleep(1);
    if (!tty_init(s_fdTraceDev, B115200, 0, 8, 1, 'N') )
    {
        RLOGE ("the device: %s init failed!B115200 \n", s_device_path_trace);
        return false;
    }
    usleep(100*1000);
    blockingWrite(s_fdTraceDev, initData, sizeof(initData));
    usleep(100*1000);
    blockingWrite(s_fdTraceDev, L2L3Data, sizeof(L2L3Data));
    usleep(100*1000);
    blockingWrite(s_fdTraceDev, GSMC_IData, sizeof(GSMC_IData));
    usleep(100*1000);
    blockingWrite(s_fdTraceDev, LayerStateMeasurementL1, sizeof(LayerStateMeasurementL1));

// verify if the trace is configed successfully.
#if 0
    int count = 0;
    int times = 0;
    memset(s_TraceBuffer, sizeof(s_TraceBuffer), 0);

    for (;;) {
        count += read(s_fdTraceDev, s_TraceBuffer,  TEST_TRACE_BUFFER);

        RLOGE ("traceInit count=%d", count);

        if (count>=100) {
            break;
        } 
        if (times > 2){
            inited = false;
            return false;
        }else {
            sleep(1);
            times++;
        }
    }
    
#endif    
    
    inited = true;
    RLOGD("traceInit end!");
    return true;
}

static bool processHALTraceCmd (int fd) {
/*
struct trace_cmd_frame_req {
	uint32_t token;
	uint32_t cmd;
	uint32_t len;
	uint8_t data[0];
};
*/
    uint32_t cmdHead[3] = {0}; 
    uint32_t token;
    uint32_t cmd;
    uint32_t len;
    uint8_t * data;

    int ret = 0;
    int readLen = 0;
    
    RLOGI("processTraceCmd");

    if (recv(fd, &cmdHead, sizeof(cmdHead), 0) != sizeof(cmdHead)) {
        RLOGE ("processTraceCmd, error reading on socket: cmdHead");
        return false;
    }
    s_token = cmdHead[0];
    cmd = cmdHead[1];
    len = cmdHead[2];
    RLOGI("processTraceCmd cmd=%d len=%d", cmd, len);
    
    data = (uint8_t *) malloc(sizeof(uint8_t) * len);
    readLen =  recv(fd, data, sizeof(char) * len, 0);
    RLOGI("processHALCmd readLen=%d", readLen);

    if (readLen != (int)len) {
        RLOGE ("processHALCmd, error reading on: data[]");
        free(data);
        return false;
    } 
    
    switch (cmd) {
        case TRACE_GET_STATUS:
            RLOGI ("cmd: TRACE_GET_STATUS.");
            sendRspToHAL(TRACE_GET_STATUS, s_trace_status);
            break;
            
        case TRACE_TASK_CONFIG_IND:
        {
            RLOGI ("cmd: TRACE_TASK_CONFIG_IND.");
            trace_task_config_ind * pTaskInfo = (trace_task_config_ind *)data;
            memcpy(&g_taskConfig_Info, pTaskInfo, sizeof(trace_task_config_ind));
            
            RLOGI("g_taskConfig_Info Trio_0_taskConfig=%d", g_taskConfig_Info.Trio_0_taskConfig);
            RLOGI("g_taskConfig_Info Trio_0_testType=%d", g_taskConfig_Info.Trio_0_testType);
            RLOGI("g_taskConfig_Info Trio_0_modemType=%d", g_taskConfig_Info.Trio_0_modemType);
            RLOGI("g_taskConfig_Info Trio_0_callType=%d", g_taskConfig_Info.Trio_0_callType);
            RLOGI("g_taskConfig_Info Trio_0_callDuration=%d", g_taskConfig_Info.Trio_0_callDuration);

        }
        break;

                
        case TRACE_GPS_IND:
        {
            RLOGI ("cmd: TRACE_GPS_IND.");
            trace_GPS_Info_ind * pGPS = (trace_GPS_Info_ind *)data;
            memcpy(&g_GPS_Info, pGPS, sizeof(trace_GPS_Info_ind));
        }
            break;

        case TRACE_ON:
            {
                int i;
                const int TRACE_INIT_TIMES = 3;
                pthread_attr_t attr;
                
                RLOGI("cmd: TRACE_ON.");
                char  railLineName[20] = {0};
                char  railwayBureauName[20] = {0};
                if (s_trace_open) {
					 RLOGE("error: TRACE  not close.");
					s_trace_open = false;
					sleep(1);
				}

                for (i=0; i<TRACE_INIT_TIMES; i++){
                    if (!traceInit()) {
                        RLOGE("traceInit err.");
                        continue;
                    }else {
                        break;
                    }
                }
                if (i >= TRACE_INIT_TIMES){
                    s_trace_status = TRACE_ERR_OPEN_DEV;
                    sendRspToHAL(TRACE_ON, TRACE_ERR_OPEN_DEV);
                    s_fdTraceDev = 0;
                    return true;
                }

                trace_start_req * req = (trace_start_req *)data;

                RLOGI("processTraceCmd traceFileNameLen=%d railLineLen=%d railwayBureauLen=%d ", 
                	req->traceFileNameLen, req->railLineLen, req->railwayBureauLen);
				
                memset(s_openFileName, 0, sizeof(s_openFileName));
                memcpy(s_openFileName, req->traceFileName, req->traceFileNameLen);

                memcpy(railLineName, req->traceFileName + req->traceFileNameLen, req->railLineLen);
                memcpy(railwayBureauName, req->traceFileName + req->traceFileNameLen+req->railLineLen,
                    req->railwayBureauLen);
                RLOGD("processCmd traceFileName: %s", req->traceFileName);
                RLOGD("processCmd railLineName: %s", railLineName);
                RLOGD("processCmd railwayBureauName: %s", railwayBureauName);

                writeTraceFileHead(req->railLineName, req->railwayBureauName);
                
                frame_init_list(&s_traceFrameList);
                s_trace_open = true;

                pthread_attr_init(&attr);
                //pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
                pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

                // read the data from trace.    
                RLOGI("begin to process trace stream.");
                ret = pthread_create(&s_tid_readTrace, &attr, readTraceStreamLoop, NULL);
                ret = pthread_create(&s_tid_processTrace, &attr, processTraceFrameLoop, NULL);
                sendRspToHAL(TRACE_ON, TRACE_OK);
            }
            
            break;
        case TRACE_OFF:
            {
                RLOGI ("cmd: TRACE_OFF.");
                trace_end_req * req = (trace_end_req *)data;
                RLOGD("processCmd file name: %s", req->traceFileName);

                if (strcmp(s_openFileName, req->traceFileName) != 0) {
                	RLOGE("TRACE_OFF, file name is not consisted with the opened.");
                }
                				
                s_trace_open = false;
                sendRspToHAL(TRACE_OFF, TRACE_OK);
            }
            break;
        case TRACE_CONFIG:
            RLOGI("cmd: TRACE_CONFIG.");
            break;
            
        case TRACE_SHORT_CALL_IND:
            RLOGI("cmd: TRACE_SHORT_CALL_IND.");
            if (s_trace_open){
                WriteFile_OutgoingShortCall(data, len);
            }
            break;
        case TRACE_SHORT_INCOMING_CALL_IND:
            RLOGI("cmd: TRACE_SHORT_INCOMING_CALL_IND.");
            if (s_trace_open){
                WriteFile_IncomingCall(data, len);
            }
            break;
            
        case TRACE_SHORT_STATISTIC_IND:
            RLOGI("cmd: TRACE_SHORT_STATISTIC_IND.");
            if (s_trace_open){
                // todo, add ril ID for 2 modem.
                WriteFile_ShortCallStatistic(data, len);
            }
            break;
            
        case TRACE_AT_DATA_IND:
            RLOGI("cmd: TRACE_AT_DATA_IND.");
            if (s_trace_open){
                WriteFile_AT(data, len);
            }
            break;
            
        case TRACE_TEST:
            RLOGI("cmd: TRACE_TEST");

      
            break;
        default:
            RLOGE ("cmd: Invalid request");
            break;
    }
    free(data);
    return true;
}


static void processHALTraceConnect(int fd) {
    int ret;

    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);

    s_fdTrioHalAccept = accept(fd, (sockaddr *) &peeraddr, &socklen);

    if (s_fdTrioHalAccept < 0 ) {
        RLOGE("Error on accept() errno:%d", errno);
        return;
    }

    ret = fcntl(s_fdTrioHalAccept, F_SETFL, O_NONBLOCK);

    if (ret < 0) {
        RLOGE ("Error setting O_NONBLOCK errno:%d", errno);
        return;
    }
    
    FD_CLR(fd, &readSocketFds);
    FD_SET(s_fdTrioHalAccept, &readSocketFds);
    if (s_fdTrioHalAccept >= nfds){
        nfds = s_fdTrioHalAccept+1;
    } 

    RLOGI("Trio trace: new connection");
}

static void * HALReaderLoop(void *arg)
{
    int n;
    fd_set rfds;
    int ret;
	
    for (;;) {
        RLOGI("HALReaderLoop .\n");    
        // make local copy of read fd_set
        memcpy(&rfds, &readSocketFds, sizeof(fd_set));
        n = select(nfds, &rfds, NULL, NULL, NULL);
        RLOGI("HALReaderLoop select return: %d", n);
        if (n < 0) {
            if (errno == EINTR) continue;
            RLOGE("HALReaderLoop: select error (%d)", errno);
            return NULL;
        }
        // Check for read-ready
        if(FD_ISSET(s_fdTrioHALListen, &rfds)) {
            processHALTraceConnect(s_fdTrioHALListen);        
        }
        if(FD_ISSET(s_fdTrioHalAccept, &rfds)) {
            ret = processHALTraceCmd(s_fdTrioHalAccept);   
            if (!ret) {
                RLOGE("HALReaderLoop: HAL socket read err.");
                close(s_fdTrioHalAccept);
                s_fdTrioHalAccept = 0;
                FD_ZERO(&readSocketFds);
                FD_SET(s_fdTrioHALListen, &readSocketFds);
                if (s_fdTrioHALListen >= nfds) {
                    nfds = s_fdTrioHALListen+1;
                }
            }
        }
    }
    RLOGE("error in HALReaderLoop errno:%d", errno);
    // kill self to restart on error
    kill(0, SIGKILL);
}


int main(int argc, char *argv[])
{
    int ret;
    int fd = -1;
    pthread_attr_t attr;
      
// start read interface socket
       
    /* delete the socket file */  
    unlink(SOCKET_NAME_TRACE_0);  

    /* create a socket */  
    fd = socket(AF_UNIX, SOCK_STREAM, 0);  
    struct sockaddr_un server_addr;  
    server_addr.sun_family = AF_UNIX;  
    strcpy(server_addr.sun_path, SOCKET_NAME_TRACE_0);  

    /* bind with the local file */  
    bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
    ret = listen(fd, 4);
    fcntl(fd, F_SETFL, O_NONBLOCK);
    FD_ZERO(&readSocketFds);
    FD_SET(fd, &readSocketFds);

    if (fd >= nfds) {
        nfds = fd+1;
    }
    s_fdTrioHALListen = fd;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&s_tid_readHAL, &attr, HALReaderLoop, NULL);
    
    if (ret < 0) {
        RLOGE ("pthread_create err. \n");
        exit(-1);
    }
        
    while(1) {
        // sleep(UINT32_MAX) seems to return immediately on bionic
        sleep(0x00ffffff);
    }

}
