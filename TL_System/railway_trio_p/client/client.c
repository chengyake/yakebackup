#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "remote_client_connection.h"


int main(int argc, char **argv) {

    int ret;

    //openlog("client",LOG_PID|LOG_PERROR, LOG_USER);
    openlog("client",LOG_PID, LOG_USER);

    ret = init_task_database();
    if(ret < 0) {
        syslog(LOG_ERR, "init task database error %d", ret);
        goto exit_tag;
    }

    ret = init_cmd_database();
    if(ret < 0) {
        syslog(LOG_ERR, "init task database error %d", ret);
        goto exit_tag;
    }
    
    ret = attach_local_server();
    if(ret < 0) {
        syslog(LOG_ERR, "attach local error %d", ret);
        goto exit_tag;
    }

    ret = attach_remote_server();
    if(ret < 0) {
        syslog(LOG_ERR, "attach remote error %d", ret);
        goto attach_remote_err;
    }
    
    sleep(1);

    syslog(LOG_INFO, "attach remote server success, setup client event loop");
    ret = setup_event_loop();
    if(ret < 0) {
        syslog(LOG_ERR, "setup event loop error: %d", ret);
        goto event_loop_err;
    }
        
    //something wrong
event_loop_err:
    deattach_remote_server();
attach_remote_err:
    deattach_local_server();
exit_tag:
    close_task_queue();
    closelog();
    return ret;

}






