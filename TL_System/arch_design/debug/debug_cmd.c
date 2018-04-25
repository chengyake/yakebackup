#include <stdio.h>
#include <string.h>

#include "../core/core.h"

void time_usage() {
    printf("Get or Set system time\n");
    printf("\n");
    printf("Usage:\n");
    printf("time [year-mon-day|hour-min-sec]\n");
    printf("\n");
    printf("for example:\n");
    printf("\t>>time\n");
    printf("\tget local time\n");
    printf("\n");
    printf("\t>>time 15-06-01|12-45-00\n");
    printf("\tset local time year:2015 mon:06 day:01 hour:12 min:24 sec:00\n");
}


int time_func(char *arg1, char *arg2, char *arg3, char *arg4) {
    
    unsigned char buf[SOCKET_BUFFER_SIZE] = {0};
    struct cmd_header *p = (struct cmd_header *)&buf[0];
    
    p->host = IDX_USER_DEBUG;
    p->target = IDX_DEVICE_MISC;
    p->cmd = DEBUG_MSG_GET_TIME_CMD;

    if(arg1 == NULL) {
        p->len = 0;
    } else {
        p->len = strlen(arg1);
        memcpy(&p->data[0], arg1, strlen(arg1));
    }

    return debug_write_msg(p, (CMD_HEADER_SIZE+p->len));
}

void exit_usage() {
    printf("exit debug mode\n\n");
}

extern int cmd_loop;
int exit_func(char *arg1, char *arg2, char *arg3, char *arg4) {
    
    unsigned char buf[SOCKET_BUFFER_SIZE] = {0};
    struct cmd_header *p = (struct cmd_header *)&buf[0];
    
    p->host = IDX_USER_DEBUG;
    p->target = IDX_FRAMEWORK_CORE;
    p->cmd = DEBUG_MSG_RELEASE_CMD;
    p->len = 0;

    debug_write_msg(p, (CMD_HEADER_SIZE+p->len));

    cmd_loop = 0;

    return 0;
}
