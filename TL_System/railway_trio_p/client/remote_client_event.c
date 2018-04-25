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
#include "remote_client_status_and_position.h"
#include "local_client_connection.h"

static unsigned int local_token = 0;



int fill_frame_buffer(unsigned char *sb,  unsigned char task_type, short task_state, 
        unsigned char *task_id, unsigned char *data, int len) {
    
    int dlen = 0;
    struct frame_fmt *p = (struct frame_fmt *)&sb[0];
    
    if(data != NULL && len > 0) {
        memcpy(&p->data[0], data, len);
        dlen = len;
    }

    p->head         = (unsigned char)0xFE;
    p->len          = (unsigned short)(dlen + FRAME_FMT_SIZE);//data + fix
    p->token        = (unsigned int)local_token++;
    p->task_type    = task_type;
    p->task_state   = task_state;

    if(task_id != NULL) {
        memcpy(&p->task_id[0], task_id, FRAME_TASK_ID_SIZE);
    }
    len = FRAME_DATA_BEFORE_SIZE + dlen;

    make_md5_fcs(&sb[0], len, &sb[len]);
    len =len + FRAME_MD5_SIZE;

    sb[len] = 0xFF;
    len+=1;
    
    /*int i=0;
    for(i=0; i<len; i++) {
        syslog(LOG_INFO, "%02x", sb[i]);
    }
    syslog(LOG_INFO, "");*/


    return len;

}


int no_task_process(struct frame_fmt *fhp) {
    syslog(LOG_INFO, "no task process");
    return 0;
}

uint64_t get_remote_interval(uint64_t pass) {

    uint64_t t1, t2, t3, tmp;
    t1 = get_task_interval();
    t2 = get_report_interval();
    t3 = get_upload_interval();

    syslog(LOG_INFO, "timeout: t1 = %llu, t2 = %llu, t3 = %llu", t1, t2, t3);

    if(t1 < t2) {
        tmp = t1;
    } else {
        tmp = t2;
    }

    tmp = tmp<=t3?tmp:t3;

    return tmp;
}

int check_and_process_remote_task() {
    check_and_process_task();
    check_and_upload_task();
    check_and_report_status_and_position_info();


}

int process_remote_event(struct frame_fmt *fhp) {

    int ret;

    /*ret = check_frame_fcs(fhp);
    if(ret < 0) {
        syslog(LOG_INFO, "process remote event: check frame MD5 error");
        return -1;
    }*/

    switch (fhp->task_type) {
        case NO_TASK:
            ret = no_task_process(fhp);
            break;
        case REG_TASK:
            ret = reg_task_process(fhp);
            break;
        case TEST_TASK:
            ret = task_process(fhp);
            break;
        case UPLOAD_TASK:
            ret = upload_task_process(fhp);
            break;
        case REPORT_TASK:
            ret = status_task_process(fhp);
            break;
        default:
            ret = 0;
            break;
    }

    return ret;
}


















