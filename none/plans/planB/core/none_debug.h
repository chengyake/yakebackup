#ifndef debug_h
#define debug_h

#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <syslog.h>




//#define debug




//#define LOG_CON     (LOG_PID|LOG_PERROR)
#define LOG_CON     (LOG_PID)

#ifndef debug
#define logdebug(...)   ((void)0)
#define loginfo(...)   syslog(LOG_INFO, __VA_ARGS__)
#define logerr(...)   syslog(LOG_ERR, __VA_ARGS__)
#else
#define logdebug(...)   syslog(LOG_DEBUG, __VA_ARGS__)
#define loginfo(...)   syslog(LOG_INFO, __VA_ARGS__)
#define logerr(...)   syslog(LOG_ERR, __VA_ARGS__)
#endif










#endif
