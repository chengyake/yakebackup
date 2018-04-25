#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <syslog.h>

#include "debug_cmd.h"


int main(void) {
    
    int ret;
	openlog("debug",LOG_PID|LOG_PERROR, LOG_USER);

    ret = attach_core();
    if(ret < 0) {
        syslog(LOG_ERR, "attach core error %d\n", ret);
        return ret;
    }
    syslog(LOG_INFO, "attach core server success, setup debug event loop\n");
    ret = setup_event_loop();
    if(ret < 0) {
        syslog(LOG_ERR, "setup event loop %d\n", ret);
        return ret;
    }
    syslog(LOG_INFO, "setup debug event loop success, setup debug cmd loop\n");
    ret = setup_cmd_loop();
    if(ret < 0) {
        syslog(LOG_ERR, "setup cmd loop error %d\n", ret);
        return ret;
    }

    syslog(LOG_ERR, "main loop break\n");
    closelog();

    return 0;
}





