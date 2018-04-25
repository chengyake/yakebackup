#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>

#include "../core/core.h"

int register_debug_module() {
    struct cmd_header cmd_rsp;

    syslog(LOG_INFO, "debug register to core\n");
    cmd_rsp.host = IDX_USER_DEBUG;
    cmd_rsp.target = IDX_FRAMEWORK_CORE;
    cmd_rsp.cmd = CORE_MSG_REGISTER_CMD;
    cmd_rsp.len = 0;

    return debug_write_msg((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}

static int register_debug_module_rsp() {

    syslog(LOG_INFO, "debug register to core\n");
    return 0;
}

int debug_get_system_time(struct cmd_header *chp) {
    
    if(chp->len > 0) {
    	printf("%s\n", chp->data);
    }
    return 0;
}

int debug_set_system_time(struct cmd_header *chp) {
    
    if(chp->len > 0) {
    	printf("%s\n", chp->data);
    }
    return 0;
}

int debug_release_rsp(struct cmd_header *chp) {
    	printf("debug module release\n");
    	return 0;
}

static int debug_touch(struct cmd_header *chp) {
    syslog(LOG_INFO, "debug touch, rsp success\n");
    struct cmd_header cmd_rsp;
    cmd_rsp.host = IDX_USER_DEBUG;
    cmd_rsp.target = chp->host;
    cmd_rsp.cmd = DEBUG_MSG_GET_STATUS_RSP;
    cmd_rsp.len = 0;

    return debug_write_msg((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}



int debug_event_process(struct cmd_header *p) {
    
    int ret;

    switch (p->cmd) {

        case DEBUG_MSG_GET_STATUS_CMD:
            ret = debug_touch(p);
            break;

        case CORE_MSG_REGISTER_RSP:
            ret =  register_debug_module_rsp(p);
            break;

        case DEBUG_MSG_GET_TIME_RSP:
            ret = debug_get_system_time(p);
            break;

        case DEBUG_MSG_SET_TIME_RSP:
            ret = debug_set_system_time(p);
            break;

        case DEBUG_MSG_CHECK_SD_SIZE_RSP:
            //ret = debug_check_sdcard_size(p);
            break;

        case DEBUG_MSG_RELEASE_RSP:
            ret = debug_release_rsp(p);
            break;

        default:
            ret = 0;
            break;
    }

    return ret;

}



