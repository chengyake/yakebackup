#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>
#include <stddef.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#include "../core/core.h"
#include "client_task_queue.h"
#include "remote_client_connection.h"
#include "local_client_connection.h"









static int set_system_datetime(struct frame_fmt *fhp) {
    
    int fd;
    int len, i; 
    struct timeval tv;
    unsigned char sb[SOCKET_BUFFER_SIZE]={0};
    unsigned char data[64]={0};
    
    memcpy(&data[0], &fhp->data[0], sizeof(time_t));

    tv.tv_sec = datetime_to_unix(*(uint64_t *)(&fhp->data[0]));
    tv.tv_usec = 0;

    settimeofday(&tv, (struct timezone *)0);


    system("hwclock -w");

    len = fill_frame_buffer(&sb[0], TIME_TASK, SET_TIME_SUCCESS, &fhp->task_id[0], 
                            &data[0], len);


    return write_remote_server(&sb[0], len);

}


int set_time_process(struct frame_fmt *fhp) {

    int ret;
    logdebug("date time process");

    switch(fhp->task_state) {

        case SET_TIME_REQUEST:
            ret = set_system_datetime(fhp);
            break;

        default:
            logerr("set time proccess default process");
            ret = 0;
            break;
    }
    return ret;
}






