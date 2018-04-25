#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "remote_client_connection.h"
#include "../core/core.h"

int main(int argc, char **argv) {

    int ret;

    //openlog("client",LOG_PID|LOG_PERROR, LOG_USER);
    openlog("client",LOG_PID, LOG_USER);

    ret = init_task_database();
    if(ret < 0) {
        logerr("init task database error %d", ret);
        goto exit_tag1;
    }

    ret = init_cmd_database();
    if(ret < 0) {
        logerr("init task database error %d", ret);
        goto exit_tag2;
    }
    
    ret = attach_local_server();
    if(ret < 0) {
        logerr("attach local error %d", ret);
        goto exit_tag3;
    }

    ret = attach_remote_server();
    if(ret < 0) {
        logerr("attach remote error %d", ret);
        goto attach_remote_err;
    }
    
    sleep(1);

    loginfo("attach remote server success, setup client event loop");
    ret = setup_event_loop();
    if(ret < 0) {
        logerr("setup event loop error: %d", ret);
        goto event_loop_err;
    }
        
    //something wrong
event_loop_err:
    deattach_remote_server();
attach_remote_err:
    deattach_local_server();
exit_tag3:
    close_task_queue();
exit_tag2:
    close_cmd_queue();
exit_tag1:
    closelog();
    return ret;

}






