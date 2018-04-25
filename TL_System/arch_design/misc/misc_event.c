#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>

#include "../core/core.h"
#include "misc_event.h"
#include "misc_connection.h"


int register_misc_module() {
    struct cmd_header cmd_rsp;

    syslog(LOG_INFO, "misc register to core\n");
    cmd_rsp.host = IDX_DEVICE_MISC;
    cmd_rsp.target = IDX_FRAMEWORK_CORE;
    cmd_rsp.cmd = CORE_MSG_REGISTER_CMD;
    cmd_rsp.len = 0;

    return misc_write_msg((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}

static int register_misc_module_rsp() {

    syslog(LOG_INFO, "misc register to core\n");
    return 0;
}

static int misc_touch(struct cmd_header *p) {

    struct cmd_header cmd_rsp;
    cmd_rsp.host = IDX_DEVICE_MISC;
    cmd_rsp.target = p->host;
    cmd_rsp.cmd = MISC_MSG_GET_STATUS_RSP;
    cmd_rsp.len = 0;

    return misc_write_msg((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}

static int misc_get_system_time(struct cmd_header *p) {

    time_t now;     
    struct tm *timenow;
    struct cmd_header cmd_rsp;
    unsigned char rsp_buf[SOCKET_BUFFER_SIZE] = {0};
    cmd_rsp.host = IDX_DEVICE_MISC;
    cmd_rsp.target = p->host;
    cmd_rsp.cmd = MISC_MSG_GET_TIME_RSP;

    time(&now);
    timenow = localtime(&now);
    sprintf(&rsp_buf[CMD_HEADER_SIZE], "%s", asctime(timenow));
    cmd_rsp.len = strlen(&rsp_buf[CMD_HEADER_SIZE]);
    memcpy(&rsp_buf[0], (char *)(&cmd_rsp), CMD_HEADER_SIZE);

    return misc_write_msg((char*)&cmd_rsp, CMD_HEADER_SIZE+cmd_rsp.len);
}

static int misc_set_system_time(struct cmd_header *p) {

    int ret;
    unsigned char time_cmd[32];
    unsigned char date_cmd[32];
    unsigned char year[3]={0};
    unsigned char mon[3]={0};
    unsigned char day[3]={0};
    unsigned char hour[3]={0};
    unsigned char min[3]={0};
    unsigned char sec[3]={0};

    struct cmd_header cmd_rsp;
    unsigned char rsp_buf[SOCKET_BUFFER_SIZE] = {0};
    cmd_rsp.host = IDX_DEVICE_MISC;
    cmd_rsp.target = p->host;
    cmd_rsp.cmd = MISC_MSG_SET_TIME_RSP;


    sscanf(&p->data[0], "%2s-%2s-%2s|%2s-%2s-%2s", 
            year, mon, day, hour, min, sec);

 
    sprintf(time_cmd, "date -s %2s:%2s:%2s", hour, min, sec);
    sprintf(date_cmd, "date -s %2s/%2s/%2s", mon, day, year);
    //ret = system(time_cmd);
    if(ret<0) {
    	return ret;
    }
    //ret = system(date_cmd);
    if(ret<0) {
    	return ret;
    }

    //ret = system("hwclock  -w");  

    syslog(LOG_INFO, "set systime info: %s\n", time_cmd);
    syslog(LOG_INFO, "set sysdate info: %s\n", date_cmd);


    return 0;
}

static int misc_check_sdcard_size(struct cmd_header *p) {
    

    return 0;
}

static int misc_release(struct cmd_header *p) {

    
    return 0;
}






//run in thread if need
int misc_event_process(struct cmd_header *p) {
    
    int ret;

    switch(p->cmd) {

        case CORE_MSG_REGISTER_RSP:
            ret =  register_misc_module_rsp(p);
            break;

        case MISC_MSG_GET_STATUS_CMD:
            ret = misc_touch(p);
            break;

        case MISC_MSG_GET_TIME_CMD:
            ret = misc_get_system_time(p);
            break;

        case MISC_MSG_SET_TIME_CMD:
            ret = misc_set_system_time(p);
            break;

        case MISC_MSG_CHECK_SD_SIZE_CMD:
            ret = misc_check_sdcard_size(p);
            break;

        case MISC_MSG_RELEASE_CMD:
            ret = misc_release(p);
            break;

        default:
            ret = 0;
            break;
    }

    return ret;

}

