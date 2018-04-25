
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
void __log_print(int prio, const char *tag, const char *fmt, ...) {
	va_list ap;  
    char buf[LOG_BUF_SIZE] = {0};    
    static FILE* stream = NULL;
    
    struct timeval tv;    
    time_t timer;
    struct tm *t;
    timer = time(NULL);

    if (stream == NULL) {
        stream = stdout;//fopen("log.txt", "w");  
    }
    
    pthread_mutex_lock(&s_logMutex);
    t = localtime(&timer);

    gettimeofday(&tv, NULL); 
    fprintf(stream, "%02d-%02d %02d:%02d:%02d.%03d  ",
            t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec, (int)tv.tv_usec / 1000);
    

	fprintf(stream, "%s", tag); //stderr
	fprintf(stream,  ":    "); 

    va_start(ap, fmt);  
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);  
    va_end(ap); 
    fprintf(stream, "%s", buf); 
    
	fprintf(stream,  "\n"); 
	fflush(stream);
    pthread_mutex_unlock(&s_logMutex);
}


