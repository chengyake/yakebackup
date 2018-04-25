
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/un.h>  
#include <pthread.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>

#include "../Triorail/hal_triorail.h"

#define LOG_TAG "HAL_TRIO_TEST"
#include "../liblog/log.h"


#define SOCKET_NAME_HAL_TRIORAIL "hal-triorail-socket"

static int s_halSocketFd = 0;
static pthread_mutex_t s_writeMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t s_tid_readHAL;

static fd_set HALreadFds;
static int nfds = 0;

enum options {
    DIAL_CALL,
    ANSWER_CALL,
    END_CALL,
    OPEN_TRACE,
    CLOSE_TRACE,
    TRIO_TEST,
};


static void print_usage() {
    perror("Usage: Hal_trio_test [option] [extra_socket_args]\n\
           0 number - DIAL_CALL number, \n\
           1 - ANSWER_CALL, \n\
           2 - END_CALL \n\
           3 file name - OPEN_TRACE file name, \n\
           4 file name - CLOSE_TRACE file name\n\
           5  - TEST\n");
}

static int error_check(int argc, char * argv[]) {
    if (argc < 2) {
        return -1;
    }
    const int option = atoi(argv[1]);
    RLOGI("error_check option=%d   argc=%d", option, argc);
    
    if (option < 0 || option > 4) {
        return -1;
    } else if ((option == DIAL_CALL || option == OPEN_TRACE
                    || option == CLOSE_TRACE) && argc == 3) {
        return 0;
    } else if ((option == ANSWER_CALL || option == END_CALL || option == TRIO_TEST) && argc == 2) {
        return 0;
    }
    return -1;
}

static int get_number_args(char *argv[]) {
    const int option = atoi(argv[1]);
    if (option == ANSWER_CALL && option == END_CALL) {
        return 1;
    } else {
        return 2;
    }
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
sendHALReq (int cmd, void * data, int len) {
    int ret;
/*
    unsigned short host;
    unsigned short target;
    unsigned short cmd;
    unsigned short len;
    unsigned char data[0];
*/    
    unsigned short cmdHead[4] = {0};
    unsigned char *  pSendData;
    pSendData = (unsigned char *)malloc(sizeof(cmdHead) + len);

    if (s_halSocketFd <= 0) {
        RLOGE("sendHALResponse: s_fdTrioHalAccept=%d", s_halSocketFd);
        return -1;
    }
    RLOGE("sendHALResponse: cmd=%d, len=%d", cmd, len);

    cmdHead[0] = (unsigned short)MODULE_USER_CLIENT;
    cmdHead[1] = (unsigned short)MODULE_DEVICE_TRIO;
    cmdHead[2] = (unsigned short)cmd;
    cmdHead[3] = (unsigned short)len;
	memcpy(pSendData, cmdHead, sizeof(cmdHead));
	memcpy(pSendData+sizeof(cmdHead), data, len);
    pthread_mutex_lock(&s_writeMutex);

    ret = blockingWrite(s_halSocketFd, (void *)pSendData, sizeof(cmdHead)+len);

    if (ret < 0) {
        pthread_mutex_unlock(&s_writeMutex);
        free(pSendData);
        return ret;
    }

    pthread_mutex_unlock(&s_writeMutex);
	free(pSendData);
    return 0;
}

static bool processHALRsp(int fd)
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
    RIL_cmd_rsp * rsp;
        
    RLOGI("processHALRsp");

    if (recv(fd, &cmdHead, sizeof(cmdHead), 0) != sizeof(cmdHead)) {
        RLOGE ("processHALRsp, error reading on socket: cmdHead");
        return false;
    }
    RLOGI("processHALRsp host=%d", cmdHead[0]);
    RLOGI("processHALRsp target=%d", cmdHead[1]);
    //only use cmd now.
    cmd = cmdHead[2];
    len = cmdHead[3];
    RLOGI("processHALRsp cmd=%d", cmd);
    RLOGI("processHALRsp len=%d", len);
    
    if (len !=0) {
		data = (unsigned char *) malloc(sizeof(char) * len);
		do {
			readLen =  recv(fd, data, sizeof(char) * len, 0);
			RLOGI("processHALRsp readLen=%d", readLen);
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
		rsp = (RIL_cmd_rsp *)data;
		RLOGI("processHALRsp rsp->status =%d", rsp->status);
	}
    
    switch (cmd) {
		case CORE_MSG_REGISTER_CMD:
            RLOGI("cmd: CORE_MSG_REGISTER_CMD.");
            sendHALReq(CORE_MSG_REGISTER_RSP, NULL, 0);
            break;
        case RIL_TRIO_0_DIAL_RSP:
            RLOGI("cmd: RIL_TRIO_0_DIAL_RSP.");
            break;
        case RIL_TRIO_0_HANGUP_RSP:
            RLOGI("cmd: RIL_TRIO_0_HANGUP_RSP.");

            break;
        case RIL_TRIO_0_STATUS_RSP:
            RLOGI("cmd: RIL_TRIO_0_STATUS_RSP.");
            break;
        case RIL_TRIO_1_STATUS_RSP:
            RLOGI("cmd: RIL_TRIO_1_STATUS_RSP.");
            break;
        case RIL_TRIO_0_TRACE_START_RSP:
            RLOGI("cmd: RIL_TRIO_0_TRACE_START_RSP.");
            break;
        case RIL_TRIO_0_TRACE_END_RSP:
            RLOGI("cmd: RIL_TRIO_0_TRACE_END_RSP.");
            break;
        case RIL_TRIO_0_TRACE_STATUS_RSP:
            RLOGI("cmd: RIL_TRIO_0_TRACE_STATUS_RSP.");
            break;

        default:
            RLOGI ("error cmd.");
            break;
    }
    if (len !=0) {
		free(data);
	}
    return true;
}
static void *readHALLoop(void *arg)
{
    int n;
    fd_set rfds;
	int ret;
	
    for (;;) {
        RLOGI("readHALLoop .\n");    
        // make local copy of read fd_set
        memcpy(&rfds, &HALreadFds, sizeof(fd_set));
        n = select(nfds, &rfds, NULL, NULL, NULL);
        RLOGI("readHALLoop select return: %d", n);
        if (n < 0) {
            if (errno == EINTR) continue;
            RLOGE("readHALLoop: select error (%d)", errno);
            return NULL;
        }
        // Check for read-ready
        ret = processHALRsp(s_halSocketFd);
        if (!ret) {
			 RLOGE("readHALLoop: socket read err.");
			break;
		}
    }
    RLOGE ("error in rilReaderLoop errno:%d", errno);
    // kill self to restart on error
    kill(0, SIGKILL);

    RLOGI("readTraceLoop end.\n");
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


int main(int argc, char *argv[])
{
    int fd, s_fdCommand;
    int option = 0;
    int i  = 0;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
        
    if (error_check(argc, argv)) {
        print_usage();
        exit(-1);
    }
#if 1
    /* create a socket */  
    fd = createServerSocket(SOCKET_NAME_HAL_TRIORAIL);

    s_fdCommand = accept(fd, (sockaddr *) &peeraddr, &socklen);

    fcntl(s_fdCommand, F_SETFL, O_NONBLOCK); 
    s_halSocketFd = s_fdCommand;
    
    FD_ZERO(&HALreadFds);
    FD_SET(s_fdCommand, &HALreadFds);
    if (s_fdCommand >= nfds){
        nfds = fd+1;
    } 
    pthread_create(&s_tid_readHAL, NULL, readHALLoop, NULL);    
#endif

    option = atoi(argv[1]);
    char * pArg = argv[2];
    char str[20];
    while (1) {
        switch (option) {
            case DIAL_CALL:
                {
                    RLOGI("Dial Call, address: %s\n", pArg);
                    RIL_dial_req * dialReq;
                    int len = sizeof(RIL_dial_req)+strlen(pArg)+1;
                    dialReq = (RIL_dial_req *)malloc(len);
                    dialReq->token = 0;
                    dialReq->clir = 0;
                    dialReq->addressLen = strlen(pArg)+1;
                    memcpy(dialReq->address, pArg, dialReq->addressLen);
                    sendHALReq(RIL_TRIO_0_DIAL_REQ, dialReq, len);
                    free(dialReq);
                }
                break;
            case ANSWER_CALL:
                perror("Answer Call");

                break;
            case END_CALL:
                perror("End Call");
                RIL_hangup_req hangupReq;
                hangupReq.token = 0;
                hangupReq.callID = 1;

                sendHALReq(RIL_TRIO_0_HANGUP_REQ, &hangupReq, sizeof(RIL_hangup_req));
                break;
            case OPEN_TRACE:
                {
                    int len;
                    printf("OPEN_TRACE.\n");
                    RIL_trace_start_req * startTraceReq;
                    len = sizeof(RIL_trace_start_req)+strlen(pArg)+1;
                    startTraceReq = (RIL_trace_start_req *)malloc(len);
                    startTraceReq->token = 0;
                    startTraceReq->traceFileNameLen= strlen(pArg)+1;
                    memcpy(startTraceReq->traceFileName, pArg, startTraceReq->traceFileNameLen);

                    sendHALReq(RIL_TRIO_0_TRACE_START_REQ, startTraceReq, len);  
                }
                break;
           case CLOSE_TRACE:
                {
                    int len;
                    printf("CLOSE_TRACE.\n");
                    RIL_trace_end_req * endTraceReq;
                    len = sizeof(RIL_trace_end_req)+strlen(pArg)+1;
                    endTraceReq = (RIL_trace_end_req *)malloc(len);
                    endTraceReq->token = 0;
                    endTraceReq->traceFileNameLen= strlen(pArg)+1;
                    memcpy(endTraceReq->traceFileName, pArg, endTraceReq->traceFileNameLen);

                    sendHALReq(RIL_TRIO_0_TRACE_END_REQ, endTraceReq, len);  
                }

                break;    
            case TRIO_TEST:
                perror("TRIO_TEST");
                break;            
                        
            default:
                perror ("Invalid request");
                break;
        }
        scanf("%d",&option);
        memset(str, 0, sizeof(str));
        if (option==0||option==3||option==4) {
            scanf("%s",str);
            pArg = str;
        }
        
    }

	
    while(1) {
        sleep(0x00ffffff);
    }
    return 0;
}
