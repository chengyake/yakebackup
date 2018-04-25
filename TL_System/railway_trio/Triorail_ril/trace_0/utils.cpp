#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <stdint.h>
#include <pthread.h>
#include <stdarg.h>

#include <utils.h>

#define LOG_TAG "TRACE_UTILS"
#include <log.h>

void  writeInt16(void * p, int data) {
    int16_t * pInt = (int16_t *)p;
    * pInt = (int16_t)data;
}
void writeInt32(void * p, int data) {
    int32_t * pInt = (int32_t *)p;
    * pInt = (int32_t)data;
}
void  writeInt64 (void * p, int64_t data) {
    int64_t * pInt64 = (int64_t *)p;
    * pInt64 = data;
}

void  writeData(void * p, const void *s, int len) {
    if (len<=0) {
        return;
    } else {
        memcpy(p, s, len);
    }
}


// one byte XOR check sum.
uint8_t frameCheckSum(uint8_t * ptr, int len, int offset)
{
    uint8_t result = ptr[offset];
    for (int i = 1; i<len; i++)
    {
        result ^= ptr[i + offset];
    }
    return result;
}

//对给定byte数组求取CRC校验码，校验码为2个字节
void CRC16(uint8_t * ptr, int size, uint8_t * crc_result)
{
    crc_result[0] = 0;
    crc_result[1] = 0;
    for (int i = 0; i < size; i++)
    {
        uint8_t num4 = ptr[i];
        for (uint8_t j = 0; j < 8; j = (uint8_t)(j + 1))
        {
            uint8_t num3 = (uint8_t)(crc_result[0] ^ num4);
            num3 = (uint8_t)(num3 & 0x80);
            num4 = (uint8_t)(num4 << 1);
            crc_result[0] = (uint8_t)(crc_result[0] << 1);
            if ((crc_result[1] & 0x80) != 0)
            {
                crc_result[0] = (uint8_t)(crc_result[0] | 1);
            }
            crc_result[1] = (uint8_t)(crc_result[1] << 1);
            if (num3 == 0x80)
            {
                crc_result[0] = (uint8_t)(crc_result[0] ^ 0x10);
                crc_result[1] = (uint8_t)(crc_result[1] ^ 0x21);
            }
        }
    }
}
  
static pthread_mutex_t s_logMutex = PTHREAD_MUTEX_INITIALIZER;

uint64_t ril_nano_time() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000000000LL + now.tv_nsec;
}
int64_t elapsedRealtime() {
    // Mark the time this was received, doing this
    // after grabing the wakelock incase getting
    // the elapsedRealTime might cause us to goto
    // sleep.
    struct timeval tv;    
    gettimeofday(&tv, NULL);    
    return (int64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static FILE * openLog() {
    FILE *fp = NULL;
    time_t now;
    struct tm *tm_now;
    char * file_name;
    time(&now);
    tm_now = localtime(&now);
    char tmpBuf[50] = {0};  
    strftime(tmpBuf, 50, "%Y%m%d%H%M%S", tm_now);
    asprintf(&file_name, "TRACE-%s", tmpBuf);  
    
    if((fp=fopen(file_name, "w"))==NULL) {
		printf("fp null");    
    } 
    free(file_name);
    return fp;
    
}

#define LOG_BUF_SIZE    512  
const int DEBUG_FILE_LEVEL = LOG_ERROR;
const int DEBUG_STDOUT_LEVEL = LOG_ERROR;

void __log_print(int prio, const char *tag, const char *fmt, ...) {
	va_list ap;  
    char buf[LOG_BUF_SIZE] = {0};  
    char out[LOG_BUF_SIZE] = {0}; 
    static FILE* stream = NULL;
    int len = 0;

    struct timeval tv;    
    time_t timer;
    struct tm *t;
    timer = time(NULL);

    if (stream == NULL) {
        stream = openLog(); //stderr;
    }

    pthread_mutex_lock(&s_logMutex);
    t = localtime(&timer);
    gettimeofday(&tv, NULL); 
    len += sprintf(out, "%02d-%02d %02d:%02d:%02d.%03d  ",
            t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec, (int)tv.tv_usec / 1000);
	len += sprintf(out+len, "%s%s", tag, ":    "); //stderr

    va_start(ap, fmt);  
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);  
    va_end(ap); 

    sprintf(out+len, "%s%s", buf, "\n"); 
    
    if (prio >= DEBUG_FILE_LEVEL) {
        fprintf(stream, "%s", out); 
        fflush(stream);
    }

    if (prio >= DEBUG_STDOUT_LEVEL) {
        fprintf(stdout, "%s", out);
    }
    
	pthread_mutex_unlock(&s_logMutex);
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
    withKind =  (now.tv_sec + secTo1970) * 10000000LL + now.tv_nsec/100;// + 0x8000000000000000LL;
    return  withKind;
}
