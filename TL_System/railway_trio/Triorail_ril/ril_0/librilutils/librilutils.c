
#include <telephony/librilutils.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdarg.h>
#include <log.h>

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

#define LOG_BUF_SIZE    512  

void __log_print(int prio, const char *tag, const char *fmt, ...) {
    va_list ap;  
    char buf[LOG_BUF_SIZE] = {0};  
    char out[LOG_BUF_SIZE] = {0}; 
    static FILE* stream ;
    int len = 0;

    struct timeval tv;    
    time_t timer;
    struct tm *t;
    timer = time(NULL);
    
    if (stream == NULL) {
        stream = stdout;//fopen("log.txt", "a+");  //stderr;
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
    fprintf(stream, "%s", out); 
    fflush(stream);

    if (prio == LOG_FATAL) {
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

