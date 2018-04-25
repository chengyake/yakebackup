#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <syslog.h>
#include <sys/un.h>
#include <fcntl.h>

#include "../core/core.h"
#include "gps.h"
#include "gps_device_event.h"
#include "gps_core_connection.h"



int register_client_module() {

    struct cmd_header cmd_rsp;
    loginfo("gps register to core\n");
    cmd_rsp.host = IDX_DEVICE_GPS;
    cmd_rsp.target = IDX_FRAMEWORK_CORE;
    cmd_rsp.cmd = CORE_MSG_REGISTER_CMD;
    cmd_rsp.len = 0;

    return write_core_server((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}


int send_gps_info_to(int target) {
    
    unsigned char bf[SOCKET_BUFFER_SIZE]={0};

    struct cmd_header cmd_rsp;
    logdebug("send gps info to core\n");
    cmd_rsp.host = IDX_DEVICE_GPS;
    cmd_rsp.target = target;
    cmd_rsp.cmd = GPS_MSG_GET_GPS_INFO_RSP;
    cmd_rsp.len = sizeof(struct gps_info_t);
    
    memcpy(&bf[0], &cmd_rsp, CMD_HEADER_SIZE);

    memcpy(&bf[CMD_HEADER_SIZE], &gps_info, sizeof(gps_info));

    return write_core_server((char *)&bf[0], CMD_HEADER_SIZE + sizeof(gps_info));
}

int gps_status_rsp(int target) {

    struct cmd_header cmd_rsp;
	unsigned char rsp[CMD_HEADER_SIZE+1]={0};

    cmd_rsp.host = IDX_DEVICE_GPS;
    cmd_rsp.target = target;
    cmd_rsp.cmd = CORE_MSG_GET_STATUS_RSP;
    cmd_rsp.len = 1;
    memcpy(rsp, &cmd_rsp, CMD_HEADER_SIZE+1);
    
    memcpy(&rsp[CMD_HEADER_SIZE], (unsigned char *)&gps_status, sizeof(gps_status));

    return write_core_server(&rsp[0], CMD_HEADER_SIZE + sizeof(gps_status));
}


int process_core_event(struct cmd_header *chp) {

    int ret;
    struct task_t *t;

    print_core_frame(chp);
    switch (chp->cmd) {
        case GPS_MSG_GET_GPS_INFO_CMD:
            send_gps_info_to(chp->host);
            break;
        case GPS_MSG_GET_STATUS_CMD:
            gps_status_rsp(chp->host);
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

