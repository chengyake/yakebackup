#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "gps.h"
#include "gps_device_connection.h"
#include "gps_core_connection.h"
#include "gps_connection.h"
#include "../core/core.h"

int main(int argc, char **argv) {

    int ret;

    //openlog("client",LOG_PID|LOG_PERROR, LOG_USER);
    openlog("gps",LOG_PID, LOG_USER);

    
    /*ret = attach_core_server();
    if(ret < 0) {
        logerr( "attach remote error %d\n", ret);
        goto close_dev;
    }

    while(1) {
        unsigned char tty[39] = {0xaa, 0x55, 0x3a, 0x11, 0x11, 0x11, 0x11, 0x50, 0x5f, 0x11, 0x11, 0x79, 0x1e, 0x09, 0x00, 0x11, 0x11,
            0x03, 0x11, 0x33, 0x35, 0x32, 0x31, 0x2e, 0x30, 0x36, 0x31, 0x34, 0x31, 0x31, 0x31, 0x31, 0x30, 0x2e, 0x38, 0x39, 0x34, 0x33, 0x86};
        process_device_event(&tty[0]);
        sleep(10);
    }*/
    



    ret = attach_device();
    if(ret < 0) {
        logerr( "attach local error %d\n", ret);
        goto exit_tag;
    }

    ret = attach_core_server();
    if(ret < 0) {
        logerr( "attach remote error %d\n", ret);
        goto close_dev;
    }
    
    sleep(1);

    loginfo("attach remote server success, setup client event loop\n");
    ret = setup_gps_event_loop();
    if(ret < 0) {
        logerr( "setup event loop error: %d\n", ret);
        goto event_loop_err;
    }
        
    //something wrong
event_loop_err:
    deattach_core_server();
close_dev:
    deattach_device();
exit_tag:
    closelog();
    return ret;

}






