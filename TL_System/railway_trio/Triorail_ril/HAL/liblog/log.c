
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
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#include <log.h>

static pthread_mutex_t s_logMutex = PTHREAD_MUTEX_INITIALIZER;

#define LOG_BUF_SIZE    200  
const int DEBUG_LEVEL = LOG_DEBUG;//LOG_INFO;

static FILE * openLog() {
    FILE *fp = NULL;
    time_t now;
    struct tm *tm_now;
    char * file_name;
    time(&now);
    tm_now = localtime(&now);
    char tmpBuf[50] = {0};  
    strftime(tmpBuf, 50, "%Y%m%d%H%M%S", tm_now);
    asprintf(&file_name, "HAL-%s", tmpBuf);  
    
    if((fp=fopen(file_name, "w"))==NULL) {
        printf("fp null");    
    } 
    free(file_name);
    return fp;
    
}

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
    fprintf(stream, "%s", out); 
    fflush(stream);

    if (prio >= DEBUG_LEVEL) {
        fprintf(stdout, "%s", out);
    }
    
    pthread_mutex_unlock(&s_logMutex);
}



