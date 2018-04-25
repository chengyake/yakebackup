#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/wait.h> 

#include "../core/core.h"


void main() {

    int ret;
    
    //openlog("monitor",LOG_PID|LOG_PERROR, LOG_USER);
    openlog("monitor",LOG_PID, LOG_USER);

    loginfo("-----------------trains system version:2016-01-06 17:00------------------ \n");

    sleep(60);
    //init database
   /* ret = init_monitor_database();
    if(ret < 0) {
        logerr("init monitor database error %d\n", ret);
        goto exit_tag;
    }*/

    //setup_all_process
    ret = setup_all_process();
    if(ret < 0) {
        logerr("setup all process error %d\n", ret);
        goto exit_tag;
    }
    
    sleep(1);

    //setup connection to core
    ret = attach_monitor_server();
    if(ret < 0) {
        logerr("attach monitor error %d\n", ret);
        kill_all_process();
        goto exit_tag;
    }

    sleep(1);

    //setup_event_loop
    loginfo("attach monitor core server success, setup monitor event loop\n");
    ret = setup_monitor_event_loop();
    if(ret < 0) {
        logerr("setup event loop error: %d\n", ret);
        kill_all_process();
        goto exit_tag;
    }
 

    //something wrong
exit_tag:
    closelog();

}









