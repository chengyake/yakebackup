#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>

#include "core.h"
#include "core_event.h"
#include "core_connection.h"

//user handler
int debug_fd=0;
int monitor_fd=0;
int client_fd=0;
int ui_fd=0;

//device handler
int misc_fd=0;
int gps_fd=0;
int gprs_fd=0;
int trio_fd=0;

static int *get_fd_by_module(enum module_idx idx) {

    int *fdp=NULL;
    switch(idx) {
        case IDX_DEVICE_TRIO:
            fdp = &trio_fd;
            break;

        case IDX_USER_DEBUG: 
            fdp = &debug_fd;
            break;

        case IDX_USER_MONITOR:
            fdp = &monitor_fd;
            break;

        case IDX_USER_CLIENT: 
            fdp = &client_fd;
            break;

        case IDX_USER_UI:
            fdp = &ui_fd;
            break;

        default:
            fdp = NULL;
    }

    return fdp;

}

int core_status_rsp(struct cmd_header *chp) {
	int *fdp=NULL;
    struct cmd_header cmd_rsp;
	unsigned char rsp[CMD_HEADER_SIZE+4]={0};

    cmd_rsp.host = IDX_FRAMEWORK_CORE;
    cmd_rsp.target = chp->host;
    cmd_rsp.cmd = CORE_MSG_GET_STATUS_RSP;
    cmd_rsp.len = 1;

    memcpy(rsp, &cmd_rsp, CMD_HEADER_SIZE+4);
    fdp = get_fd_by_module(chp->host);
    if(fdp == NULL) {
        syslog(LOG_ERR, "%s error: fd = NULL", __func__);
        return -1;
    }
    return write_msg(*fdp, &rsp[0], CMD_HEADER_SIZE+4);
}


int to_core_event(int fd, struct cmd_header *chp) {

	int ret;
    int *fdp=NULL;
    switch(chp->cmd) {

    	case MISC_MSG_GET_STATUS_CMD:
    	    ret = core_status_rsp(chp);
    	    break;
        case CORE_MSG_REGISTER_CMD:
            fdp = get_fd_by_module(chp->host);
            if(fdp == NULL) {
                syslog(LOG_ERR, "to core event error: fd = NULL");
                return -1;
            }
            *fdp = fd;
            //rsp ack to module
            break;
        case CORE_MSG_UNREGISTER_CMD:
            fdp = get_fd_by_module(chp->host);
            if(fdp == NULL) {
                syslog(LOG_ERR, "to core event error: fd = NULL");
                return -1;
            }
            *fdp = 0;
            break;

        default:
            syslog(LOG_ERR, "to core event error: no cmd detected");
            return -1;
            break;
    }
    return 0;
}

int to_others_event(struct cmd_header *chp) {
    int *fdp;
    fdp = get_fd_by_module(chp->target);
    if(fdp == NULL) {
        syslog(LOG_ERR, "to other event error: fd = NULL");
        return -1;
    }
    return write_msg(*fdp, (char*)chp, CMD_HEADER_SIZE + chp->len);
}

int core_event_process(int fd, struct cmd_header *chp) {

    int ret;
    print_local_frame(chp);
    switch(chp->target) {
        case IDX_FRAMEWORK_CORE:
            ret = to_core_event(fd, chp);
            break;

        default:
            ret = to_others_event(chp);
            break;
    }

    return ret;
}


