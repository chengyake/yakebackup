#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/wait.h> 



void main() {

    int ret;
    
    //openlog("monitor",LOG_PID|LOG_PERROR, LOG_USER);
    openlog("monitor",LOG_PID, LOG_USER);

    syslog(LOG_INFO, "-----------------trains system------------------ \n");

    sleep(60);
    //init database
   /* ret = init_monitor_database();
    if(ret < 0) {
        syslog(LOG_ERR, "init monitor database error %d\n", ret);
        goto exit_tag;
    }*/

    //setup_all_process
    ret = setup_all_process();
    if(ret < 0) {
        syslog(LOG_ERR, "setup all process error %d\n", ret);
        goto exit_tag;
    }
    
    sleep(1);

    //setup connection to core
    ret = attach_monitor_server();
    if(ret < 0) {
        syslog(LOG_ERR, "attach monitor error %d\n", ret);
        kill_all_process();
        goto exit_tag;
    }

    sleep(1);

    //setup_event_loop
    syslog(LOG_INFO, "attach monitor core server success, setup monitor event loop\n");
    ret = setup_monitor_event_loop();
    if(ret < 0) {
        syslog(LOG_ERR, "setup event loop error: %d\n", ret);
        kill_all_process();
        goto exit_tag;
    }
 

    //something wrong
exit_tag:
    closelog();

}









