
#include <telephony/librilutils.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdarg.h>

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

void printTimeStamp(FILE* pOut) {
    struct timeval tv;    
    time_t timer;
    struct tm *t;
    timer = time(NULL);
    t = localtime(&timer);

    gettimeofday(&tv, NULL); 
/*    */ 
    fprintf(pOut, "%02d-%02d %02d:%02d:%02d.%03d  ",
            t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec, (int)tv.tv_usec / 1000);
           
}

void __log_print(int prio, const char *tag, const char *fmt, ...) {
	va_list ap;  
    char buf[LOG_BUF_SIZE] = {0};    
    static FILE* pOut = NULL;

    if (pOut == NULL) {
        pOut = stdout;//fopen("log.txt", "w");  
    }
    
	pthread_mutex_lock(&s_logMutex);
	printTimeStamp(pOut); 
	fprintf(pOut, "%s", tag); //stderr
	fprintf(pOut,  ":    "); 

    va_start(ap, fmt);  
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);  
    va_end(ap); 
    fprintf(pOut, "%s", buf); 
    
	fprintf(pOut,  "\n"); 
	fflush(pOut);
	pthread_mutex_unlock(&s_logMutex);
}


