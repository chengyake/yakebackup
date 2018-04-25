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
#include "local_client_event.h"

static uint64_t old_moment = 0;
static int heart_beat = 3;

static int report_status_info() {

    int len; 

    unsigned char sb[SOCKET_BUFFER_SIZE]={0};

    struct task_t * running = get_running_task();
    if(running != NULL) {
        len = fill_frame_buffer(&sb[0],  REPORT_TASK,  module_status, &running->task_id[0], 
                NULL, 0);
    } else {
        len = fill_frame_buffer(&sb[0],  REPORT_TASK,  module_status, NULL, 
                NULL, 0);
    }

    return write_remote_server(&sb[0], len);
}

static void check_heart_beat() {

    heart_beat--;
    if(heart_beat <= 0) {
        if(remote_fd != 0) {
            attach_remote_server();
        }
        heart_beat = 3;
    }

}

static int report_position_info() {

    int len; 
    unsigned char sb[SOCKET_BUFFER_SIZE]={0};
    struct task_t * running = get_running_task();
    if(running != NULL) {
        len = fill_frame_buffer(&sb[0],  REPORT_TASK, REPORT_POSITION, &running->task_id[0], 
                &gps, sizeof(gps));
    } else {
        len = fill_frame_buffer(&sb[0],  REPORT_TASK, REPORT_POSITION, NULL, 
                &gps, sizeof(gps));
    }

    check_heart_beat();

    return write_remote_server(&sb[0], len);

}


uint64_t get_report_interval() {
    int64_t tmp;
    tmp = time(NULL) - old_moment;
    tmp = REPORT_INTERVAL_SEC - tmp;
    return tmp>0?tmp:1;
}


void check_and_report_status_and_position_info() {

    int64_t ret;
    int64_t current = time(NULL);


    if(old_moment <= 0) {
        old_moment = current;
        return;
    }

    ret = current - old_moment;
    if(ret >= REPORT_INTERVAL_SEC) {
        syslog(LOG_INFO, "report status and position info");
        old_moment = current;

        ret = get_gps_status();
        if(ret < 0) {
            syslog(LOG_INFO, "get gps status error");
        }
        ret = get_gprs_status();
        if(ret < 0) {
            syslog(LOG_INFO, "get gprs status error");
        }
        ret = get_trio0_status();
        if(ret < 0) {
            syslog(LOG_INFO, "get trio0 status error");
        }
        ret = get_trio1_status();
        if(ret < 0) {
            syslog(LOG_INFO, "get trio1 status error");
        }
        ret = get_trace_status();
        if(ret < 0) {
            syslog(LOG_INFO, "get trace status error");
        }

        report_position_info();
        report_status_info();
    }

}

int status_task_process(struct frame_fmt *fhp) {

    int ret;
    struct task_t *tp;
    syslog(LOG_INFO, "upload status task process");

    switch(fhp->task_state) {


        case REPORT_SUCCESS:
            syslog(LOG_INFO, "upload status success, task over");
            heart_beat = 3;
            break;


        default:
            ret = 0;
            break;
    }
    return 0;
}

