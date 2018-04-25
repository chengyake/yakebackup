#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>

#include "core.h"
#include "core_event.h"
#include "core_connection.h"


int main(void) {

    int ret;

	//openlog("core", LOG_PID|LOG_PERROR, LOG_USER);
	openlog("core", LOG_PID, LOG_USER);

    ret = setup_core_server();
    if(ret < 0) {
        syslog(LOG_ERR, "setup core server error %d", ret);
        return ret;
    }

    syslog(LOG_INFO, "setup core server success, into core event loop");

    ret = setup_event_loop();
    if(ret < 0) {
        syslog(LOG_ERR, "setup event loop error %d", ret);
        return ret;
    }

    syslog(LOG_INFO, "release and exit core event loop");
    closelog();

    return 0;
}







