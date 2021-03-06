

#ifndef LOG_H
#define LOG_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG  1

typedef enum LogPriority {
   LOG_UNKNOWN = 0,
   LOG_DEFAULT,   
   LOG_VERBOSE,
   LOG_DEBUG,
   LOG_INFO,
   LOG_WARN,
   LOG_ERROR,
   LOG_FATAL,
} LogPriority;

extern void __log_print(int prio, const char *tag, const char *fmt, ...);

#if DEBUG
#define RLOGD(...)   __log_print(LOG_DEBUG, LOG_TAG, __VA_ARGS__)  
#define RLOGI(...)   __log_print(LOG_INFO, LOG_TAG, __VA_ARGS__)  
#define RLOGE(...)   __log_print(LOG_ERROR, LOG_TAG, __VA_ARGS__)  
#else
#define RLOGI(...)   ((void)0)
#define RLOGD(...)   ((void)0)
#define RLOGE(...)   ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /*LOG_H */
