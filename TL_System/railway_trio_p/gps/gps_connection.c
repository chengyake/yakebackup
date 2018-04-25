#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>


#include "../core/core.h"
#include "gps_connection.h"
#include "gps_core_connection.h"
#include "gps_device_connection.h"


static fd_set fd_sets;
static int event_loop=1;

int exit_client_event_loop() {
    event_loop = 0;
}

static int set_select_sets() {
    
    int ret = -1;
    FD_ZERO(&fd_sets);
    if(dev_fd > 0) {
    	ret = 0;
        FD_SET(dev_fd, &fd_sets);
    }
    if(core_fd > 0) {
    	ret = 0;
        FD_SET(core_fd, &fd_sets);
    }
    return ret;
}

static int get_sets_max() {
    
    int fd;
    fd = (dev_fd >= core_fd) ? dev_fd : core_fd;
    return fd;

}

static int clients_data_changed() {
    int ret=0;
    if (dev_fd > 0 && FD_ISSET(dev_fd, &fd_sets)) {
        ret = recv_device();
        if(ret < 0) {
            syslog(LOG_ERR, "recv remote server error %d; and retry to connect to server \n", ret);
            return ret;
        }
    }
    if (core_fd > 0 && FD_ISSET(core_fd, &fd_sets)) {
        ret = recv_core_server();
        if(ret < 0) {
            syslog(LOG_ERR, "recv and process error %d\n", ret);
            return ret;
        }
    }
    return ret;
}

int setup_gps_event_loop() {

    int ret;

    //cmd&event loop
    while (event_loop) {

        ret = set_select_sets();
        if(ret < 0) {
            syslog(LOG_ERR, "set select sets error: return = %d\n", ret);
            exit(-1);
        }

        ret = select(get_sets_max()+1, &fd_sets, NULL, NULL, NULL);
        if (ret < 0) {
            syslog(LOG_ERR, "select error: return = %d\n", ret);
            exit(-1);
        } else if (ret == 0) {
            syslog(LOG_INFO, "select timeout: return = %d\n", ret);
        }

        clients_data_changed();
    }

    return ret;
}





