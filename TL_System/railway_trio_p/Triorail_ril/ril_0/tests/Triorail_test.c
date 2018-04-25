

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>
#include <string.h>
#define     _GNU_SOURCE

#define LOG_TAG "TRIO_TEST"

#define DEBUG
#if defined (DEBUG)
#define RLOGI(...)  {fprintf(stderr, LOG_TAG);  fprintf(stderr,  ":    "); fprintf(stderr,  __VA_ARGS__);  fprintf(stderr,  "\n");}
#define RLOGD(...)  {fprintf(stderr, LOG_TAG);  fprintf(stderr,  ":    "); fprintf(stderr,  __VA_ARGS__);fprintf(stderr,  "\n");}
#define RLOGE(...)  {fprintf(stderr, LOG_TAG);  fprintf(stderr,  ":    "); fprintf(stderr,  __VA_ARGS__);fprintf(stderr,  "\n");}
#else
#define RLOGI(...)   ((void)0)
#define RLOGD(...)   ((void)0)
#define RLOGE(...)  {fprintf(stderr, LOG_TAG);  fprintf(stderr,  ":    "); fprintf(stderr,  __VA_ARGS__);fprintf(stderr,  "\n");}
#endif

enum options {
    RADIO_RESET,
    RADIO_OFF,
    UNSOL_NETWORK_STATE_CHANGE,
    RADIO_ON,
    SETUP_PDP,
    DEACTIVATE_PDP,
    DIAL_CALL,
    ANSWER_CALL,
    END_CALL,
    CALL_TEST,
};


typedef enum {
    AT_LOG = 0,
    TRACE_LOG = 1
} log_type;

char * s_device_path ="/dev/ttyUSB0"; 
char * s_device_path_trace = "/dev/ttyUSB2"; //"/dev/ttyUSB2"; 

static pthread_mutex_t s_commandmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_commandcond = PTHREAD_COND_INITIALIZER;


#define MAX_AT_RESPONSE 500
static char s_ATBuffer[MAX_AT_RESPONSE+1];
static char s_TraceBuffer[MAX_AT_RESPONSE+1];

int s_fd;
int s_fd_trace;
FILE *fp_AT;
FILE *fp_TRACE;
static void print_usage() {
    perror("Usage: ril_test [option] [extra_socket_args]\n\
           0 - RADIO_RESET, \n\
           1 - RADIO_OFF, \n\
           2 - UNSOL_NETWORK_STATE_CHANGE, \n\
           3 - RADIO_ON, \n\
           4 apn- SETUP_PDP apn, \n\
           5 - DEACTIVE_PDP, \n\
           6 number - DIAL_CALL number, \n\
           7 - ANSWER_CALL, \n\
           8 - END_CALL, \n\
           9 number - CALL_TEST\n");
}
static void writeLog(log_type type, void * data, int len) ;
static int error_check(int argc, char * argv[]) {
    if (argc < 2) {
        return -1;
    }
    const int option = atoi(argv[1]);
    if (option < 0 || option > 9) {
        return -1;
    } else if ((option == DIAL_CALL || option == SETUP_PDP || option == CALL_TEST) && argc == 3) {
        return 0;
    } else if ((option != DIAL_CALL && option != SETUP_PDP) && argc == 2) {
        return 0;
    }
    return -1;
}

static int get_number_args(char *argv[]) {
    const int option = atoi(argv[1]);
    if (option != DIAL_CALL && option != SETUP_PDP) {
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

static bool writeCmd(const char * cmd) {
    int cur = 0;
    int written, len;
    len = strlen(cmd);
    printf("Debug port:strlen of cmd :%d\n",len);
    while (cur < len) {
        do {
            written = write (s_fd, cmd + cur, len - cur);
        } while (written < 0 && errno == EINTR);
		printf("Debug port: cmd written  :%d\n",written);
        if (written < 0) {
            perror ("write AT cmd err!");
            return false;
        }

        cur += written;
    }
    
    perror ("write AT cmd ok!");

    /* the \r  */
    do {
        written = write (s_fd, "\r" , 1);
    } while ((written < 0 && errno == EINTR) || (written == 0));

    if (written < 0) {
        return false;
    }
    perror ("write cr ok!");
    writeLog(AT_LOG, cmd, strlen(cmd));
    
    pthread_mutex_lock(&s_commandmutex);
    pthread_cond_wait(&s_commandcond, &s_commandmutex);
    pthread_mutex_unlock(&s_commandmutex);
    
    return true;

}
static void *readerLoop(void *arg)
{
    int count = 0;
    char * p_read;
    perror("readerLoop .\n");    
    memset(s_ATBuffer, sizeof(s_ATBuffer), 0);
     p_read = s_ATBuffer;

    for (;;) {
        do {
            count = read(s_fd, p_read,
                            MAX_AT_RESPONSE - (p_read - s_ATBuffer));
        } while (count < 0 && errno == EINTR);

        if (count > 0) {
            printf ("readerLoop: %s:\n", p_read);
            if ((strstr(s_ATBuffer, "OK") != NULL) ||(strstr(s_ATBuffer, "OK") != NULL)) {
                memset(s_ATBuffer, sizeof(s_ATBuffer), 0);
                p_read = s_ATBuffer;
                
                writeLog(AT_LOG, p_read, strlen(p_read));
                
                pthread_mutex_lock(&s_commandmutex);
                pthread_cond_signal(&s_commandcond);
                pthread_mutex_unlock(&s_commandmutex);
            } else {
                p_read += count;
                continue;
            }
        }
    }
    perror("readerLoop end.\n");
}

static void *readTraceLoop(void *arg)
{
    int count = 0;
    char * p_read;
    int i;
    int total = 0;
    perror("readTraceLoop .\n");    

    for (;;) {
        memset(s_TraceBuffer, sizeof(s_TraceBuffer), 0);
        p_read = s_TraceBuffer;
        int total = 0;
        do {
            count = read(s_fd_trace, p_read,
                            MAX_AT_RESPONSE - (p_read - s_TraceBuffer));
			total	+=  count;
           printf ("readTraceLoop count : %d  total=%d. \n", count, total);      
            fflush(stdout);               
        } while (count < 0 && errno == EINTR);
        
		for (i=0; i<total; i++) {
            printf("%02x", *(s_TraceBuffer+i) );
        }
        fflush(stdout);
       // writeLog(TRACE_LOG, p_read, count);
    }
    perror("readTraceLoop end.\n");
}


static bool   tty_init(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
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
            perror("com set error!\n");    
            return false;   
        }  
        return true;

}

static FILE * openLog(log_type type) {
    FILE *fp;
    time_t now;
    struct tm *tm_now;
    char * file_name;
    time(&now);
    tm_now = localtime(&now);
    char tmpBuf[50] = {0};  
    strftime(tmpBuf, 50, "%Y%m%d%H%M%S", tm_now);
    perror(tmpBuf);   
    if (type == AT_LOG){
		 perror("AT_LOG");  
        asprintf(&file_name, "AT-%s", tmpBuf);
        perror("AT_LOG end");  
    } else if (type == TRACE_LOG) {
        asprintf(&file_name, "TRACE-%s", tmpBuf);    
    } else {
        return NULL;
    }
    perror("file_name");  
    perror(file_name);    
     perror("file_name end");  
    if((fp=fopen(file_name, "w"))==NULL) {
		perror("fp null");    
    } 
    free(file_name);
    return fp;
    
}
static void writeLog(log_type type, void * data, int len) {
    if (type == AT_LOG){
        time_t now;
        struct tm *tm_now;
        char * timeTag;
        time(&now);
        tm_now = gmtime(&now);
        asprintf(&timeTag, "%d:%d:%d:   ", tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
        fwrite(timeTag,strlen(timeTag), 1, fp_AT);
        fwrite(data, len, 1, fp_AT);
        free(timeTag);
    } else if (type == TRACE_LOG) {
        fwrite(data, len, 1, fp_TRACE);
    } else {
        return;
    }
    
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
    
    s_fd_trace= open (s_device_path_trace, O_RDWR); // | O_NDELAY
    if (s_fd_trace < 0) {
        RLOGE ("could not open device: %s. \n", s_device_path_trace);
        return false;
    }

    if (!tty_init(s_fd_trace,B57600, 0, 8, 1, 'N') )
    {
        RLOGE ("the device: %s init failed! \n", s_fd_trace);
        return false;
    }
    asprintf(&cmd, "AT+CTR=2%c", '\r');
    
    int cur = 0;
    int written, len;
    len = strlen(cmd);
    while (cur < len) {
        do {
            written = write (s_fd_trace, cmd + cur, len - cur);
        } while (written < 0 && errno == EINTR);
        
        RLOGE("trace port: cmd written  :%d\n",written);
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
    if (!tty_init(s_fd_trace, B115200, 0, 8, 1, 'N') )
    {
        RLOGE ("the device: %s init failed!B115200 \n", s_fd_trace);
        return false;
    }
    usleep(100*1000);
    blockingWrite(s_fd_trace, initData, sizeof(initData));
    usleep(100*1000);
    blockingWrite(s_fd_trace, L2L3Data, sizeof(L2L3Data));
    usleep(100*1000);
    blockingWrite(s_fd_trace, GSMC_IData, sizeof(GSMC_IData));
    usleep(100*1000);
    blockingWrite(s_fd_trace, LayerStateMeasurementL1, sizeof(LayerStateMeasurementL1));

    return true;
}
int main(int argc, char *argv[])
{
    int num_socket_args = 0;
    int i  = 0;
    int option = 0;
    char cmd[50]={0};
    int ret;


    pthread_t tid;
    pthread_attr_t attr;
    static pthread_t s_tid_reader;
    static pthread_t s_tid_readTrace;

    void *tret;

    char * string;
    
    if(error_check(argc, argv)) {
        print_usage();
        exit(-1);
    }
    
    s_fd = open (s_device_path, O_RDWR); // | O_NDELAY
    if (s_fd < 0) {
        printf ("could not open device: %s. \n", s_device_path);
        exit(-1);
    }

    if (!tty_init(s_fd,B19200,0,8,1,'N') )
    {
        printf ("the device: %s init failed! \n", s_device_path);
        exit(-1);
    }


    
    fp_AT = openLog(AT_LOG); 
    if (fp_AT == NULL) {
        printf ("open at log file failed! \n");
        exit(-1);
    }
    fp_TRACE= openLog(TRACE_LOG); 
    if (fp_TRACE == NULL) {
        printf ("open at log file failed! \n");
        exit(-1);
    }

// read the data from AT.
    ret = pthread_create(&s_tid_reader, NULL, readerLoop, NULL);

    if (ret < 0) {
        perror ("pthread_create err. \n");
        return -1;
    }

    option = atoi(argv[1]);
    switch (option) {
        case RADIO_RESET:
            perror ("Connection on debug port: issuing reset.");
 //           asprintf(&string, "AT+CPIN=%s", "1234");
            printf ("RADIO_RESET: %s. \n", string);
            break;
        case RADIO_OFF:
            perror ("Connection on debug port: issuing radio power off.");
            break;
        case UNSOL_NETWORK_STATE_CHANGE:
            perror ("Debug port: issuing unsolicited voice network change.");
            break;

        case RADIO_ON:
            perror("Debug port: Radio On");
            sleep(2);
            break;
        case SETUP_PDP:
            printf("Debug port: Setup Data Call, Apn :%s\n", argv[2]);
            break;
        case DEACTIVATE_PDP:
            perror("Debug port: Deactivate Data Call");
            break;
        case DIAL_CALL:
            printf("Debug port: Dial Call, address: %s\n", argv[2]);
            sprintf(cmd, "ATD%s;", argv[2]);
            writeCmd(cmd);
            break;
        case ANSWER_CALL:
            perror("Debug port: Answer Call");
            sprintf(cmd, "ATA;");
            writeCmd(cmd);
            break;
        case END_CALL:
            perror("Debug port: End Call");
            sprintf(cmd, "ATH;");
            writeCmd(cmd);
            break;
        case CALL_TEST:
            printf("Debug port: CALL TEST, address: %s\n", argv[2]);

            if (ret < 0) {
                perror ("pthread_create err. \n");
                return -1;
            }
            if (!traceInit()) {
                printf("traceInit err.\n");
            } else {
                // read the data from trace.    
                printf("begin to read trace.\n");
                ret = pthread_create(&s_tid_readTrace, NULL, readTraceLoop, NULL);
            }
            sleep(1);
            sprintf(cmd, "ATD%s;", argv[2]);
            if (writeCmd(cmd)) {
                if (strstr(s_ATBuffer, "OK") == NULL) {
                    printf("Debug port: ATD err.\n");
                } else {
                    printf("Debug port: CALL TEST, ATD err.\n");
                    sleep(60);
                    sprintf(cmd, "ATH");
                    writeCmd(cmd);
                    printf("Debug port: CALL TEST, ATH.\n");
                }
            };
            sleep(1);
            pthread_cancel(s_tid_readTrace);
            fclose(fp_AT);
            fclose(fp_TRACE);
            break;
        default:
            perror ("Invalid request");
            break;
    }


    close(s_fd);
    return 0;
}
