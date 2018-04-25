#include <stdio.h>
#include <syslog.h>

#include "../core/core.h"
#include "misc_event.h"
#include "misc_connection.h"



int main() {
    
    int ret;

	openlog("misc",LOG_PID|LOG_PERROR, LOG_USER);

    //setup connection
    ret = attach_core();
    if(ret < 0) {
        syslog(LOG_ERR, "attach core error: %d\n", ret);
        return ret;
    }

    syslog(LOG_INFO, "attach core success, setup event loop now\n");
    
    //cmd&event loop
    ret = setup_event_loop();
    if(ret < 0) {
        syslog(LOG_ERR, "setup event loop error: %d\n", ret);
        return ret;
    }

    syslog(LOG_INFO, "error: de-attach core and exit process\n");
    deattach_core();
    closelog();
}






