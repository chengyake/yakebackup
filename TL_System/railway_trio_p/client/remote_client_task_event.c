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
#include "local_client_connection.h"
#include "hal-triorail.h"


static int print_task_data(struct task_fmt *tth) {
	int64_t t1, t2;
    struct tm *tm1;
    struct tm *tm2;
	unsigned char name[256]={0};
    if(tth == NULL) { 
    	syslog(LOG_INFO, "print test data *tth = NULL");
    	return -1;
    }

    memcpy(&name[0], &tth->file_name[0], tth->file_name_len);

    t1 = datetime_to_unix(tth->start_time);
    t2 = datetime_to_unix(tth->stop_time);

    syslog(LOG_INFO, "---------------------");
    syslog(LOG_INFO, "len:%d", tth->len);
    syslog(LOG_INFO, "position:init=%d start=%d stop=%d", tth->init_position, tth->start_position, tth->stop_position);
    //syslog(LOG_INFO, "time:start=%ld stop=%ld", t1, t2);
    
    tm1 = localtime((time_t *)&t1);
    syslog(LOG_INFO, "start time is: %s",asctime(tm1));
    tm2 = localtime((time_t *)&t2);
    syslog(LOG_INFO, "stop time is: %s",asctime(tm2));
    syslog(LOG_INFO, "nlen:%d", tth->file_name_len);
    syslog(LOG_INFO, "name:%s", name);
    syslog(LOG_INFO, "---------------------");


    return 0;
}


void stop_task_rsp(struct task_t *tth, int ok, int reason, unsigned char manual)  {
	int len;
    unsigned char sb[SOCKET_BUFFER_SIZE]={0};
    
    if(ok >= 0) {
        len = fill_frame_buffer(&sb[0],  TEST_TASK,  STOP_TASK_SUCCESS, 
                &tth->task_id[0], NULL, 0);
        write_remote_server_by_queue(&sb[0], len);
        if(manual) {
            len = fill_frame_buffer(&sb[0],  TEST_TASK,  STOP_TASK_SUCCESS_MANUAL, 
                    &tth->task_id[0], NULL, 0);

            write_remote_server_by_queue(&sb[0], len);
        } else {
            len = fill_frame_buffer(&sb[0],  TEST_TASK,  STOP_TASK_SUCCESS_AUTO, 
                    &tth->task_id[0], NULL, 0);

            write_remote_server(&sb[0], len);
        }

    } else {
        len = fill_frame_buffer(&sb[0],  TEST_TASK,  STOP_TASK_FAILED, 
                &tth->task_id[0], &reason, 1);
        write_remote_server_by_queue(&sb[0], len);
    }
}

void start_task_rsp(struct task_t *tth, int ok, unsigned char reason) {
	int len;
    unsigned char sb[SOCKET_BUFFER_SIZE]={0};
    
    if(ok >= 0) {
        len = fill_frame_buffer(&sb[0],  TEST_TASK,  START_TASK_SUCCESS, 
                &tth->task_id[0], NULL, 0);
        write_remote_server_by_queue(&sb[0], len);

        len = fill_frame_buffer(&sb[0],  TEST_TASK,  START_TASK_DOING, 
                &tth->task_id[0], NULL, 0);
        write_remote_server_by_queue(&sb[0], len);


    } else {
        len = fill_frame_buffer(&sb[0],  TEST_TASK,  START_TASK_FAILED, 
                &tth->task_id[0], &reason, 1);
        write_remote_server_by_queue(&sb[0], len);
    }

}

void cancel_task_rsp(struct task_t *tth, int ok, unsigned char reason) {

	int len;
    unsigned char sb[SOCKET_BUFFER_SIZE]={0};
    
    if(ok == 0) {
        len = fill_frame_buffer(&sb[0],  TEST_TASK,  CANCEL_TASK_SUCCESS, 
                &tth->task_id[0], NULL, 0);
    } else if(ok > 0){
        len = fill_frame_buffer(&sb[0],  TEST_TASK,  START_TASK_DOING, 
                &tth->task_id[0], NULL, 0);
    } else {
        len = fill_frame_buffer(&sb[0],  TEST_TASK,  CANCEL_TASK_FAILED, 
                &tth->task_id[0], NULL, 0);
    }

    write_remote_server_by_queue(&sb[0], len);
}




static int recv_task(struct frame_fmt *ffp) {

    int i;
    int ret, len;
    int len1, len2;
    unsigned char *seek;
    unsigned int direction;
    unsigned int *interval;
    RIL_task_config_ind cfg;
    unsigned char fail_reason=0;

    unsigned char sb[SOCKET_BUFFER_SIZE]={0};
    struct task_fmt *tfp = (struct task_fmt *)&ffp->data[0];
    struct task_t *ttp = get_void_task();
    if(ttp == NULL) {
        syslog(LOG_ERR, "database queue no space");
        fail_reason = ACTION_DATABASE_NO_SPACE_ERR;
        goto recv_add_failed;
    }
    
    //debug api
    print_task_data(tfp);
    
    ret = check_new_task(tfp);
    if(ret < 0) {
        syslog(LOG_ERR, "checksum error or repeat at time");
        fail_reason = ACTION_EXPIRED_ERR;
        goto recv_add_failed;
    }
    
    {//seek at module flag bits
        seek = &tfp->file_name[tfp->file_name_len];
        direction = seek[3];
        len1 = seek[8];
        seek = &seek[9 + len1 +14];
    }
    
    memset(&cfg, 0, sizeof(cfg));
    cfg.direction = direction;
    if(seek[1] != 0) {
        len2 = seek[5];
        interval = (unsigned int *)&seek[6 + len2];
        memcpy(ttp->task_id, ffp->task_id, 16);
        ttp->status = TASK_QUEUE;
        ttp->access_interval = interval[0];
        ttp->online_interval = interval[1];
        ttp->offline_interval = interval[2];
        ttp->phone_len = len2;
        for(i=0; i<len2&&i<15; i++) {
            ttp->phone_num[i] = seek[6 + i];
        }
        
        cfg.Trio_0_taskConfig = seek[1];
        cfg.Trio_0_testType = seek[2];
        cfg.Trio_0_callType = seek[3];
        cfg.Trio_0_modemType = seek[4];
        cfg.Trio_0_callDuration = interval[1]==0?1:0;
        
        seek = &seek[6 + len2 + 13];
    } else {
        seek = &seek[2];
    }

    if(seek[1] != 0) {
        len2 = seek[5];
        /*interval = (unsigned int *)&seek[6 + len2];
        memcpy(ttp->task_id, ffp->task_id, 16);
        ttp->status = TASK_QUEUE;
        ttp->online_interval = interval[1];
        ttp->offline_interval = interval[2];
        ttp->phone_len = len2;
        for(i=0; i<len2&&i<15; i++) {
            ttp->phone_num[i] = seek[6 + i];
        }*/
        
        cfg.Trio_1_taskConfig = seek[1];
        cfg.Trio_1_testType = seek[2];
        cfg.Trio_1_callType = seek[3];
        cfg.Trio_1_modemType = seek[4];
        cfg.Trio_1_callDuration = interval[1]==0?1:0;
        
        seek = &seek[6 + len2 + 13];
    } else {
        seek = &seek[2];
    }


    memcpy(&ttp->task, &ffp->data[0], TASK_DATA_SIZR);
    memcpy(&ttp->cfg, &cfg, sizeof(cfg));


    //debug
    //print_task_info(ttp);
    //printf("==================");
    print_all_task_info(ttp);
    add_task(ttp);

    len = fill_frame_buffer(&sb[0], TEST_TASK, RECV_TASK_SUCCESS, 
            &ffp->task_id[0], NULL , 0);

    return write_remote_server_by_queue(&sb[0], len);

recv_add_failed:
    len = fill_frame_buffer(&sb[0], TEST_TASK, RECV_TASK_FAILED, 
            &ffp->task_id[0], &fail_reason, 1);
    return write_remote_server_by_queue(&sb[0], len);

}



static int start_task(struct task_t *tp) {
    int ret;
    int len;
    char file_name[128];
     
    len = get_upload_file_name(tp, &file_name[0]);

    ret = check_new_task(tp);
    if(ret < 0) {
        syslog(LOG_ERR, "checksum error or repeat in time when start task");
        start_task_rsp(tp, -1,  ACTION_EXPIRED_ERR);
        return ret;
    }

    if(tp->task.len != 0 && tp->status == TASK_QUEUE) {
        start_trace(tp);
        update_task(tp, TASK_TESTING_AND_UPLOADING);
        start_task_rsp(tp, 1, 0);
        return 0;
    } else {
        syslog(LOG_ERR, "no task to run or task state error");
        start_task_rsp(tp, -1, ACTION_NO_THIS_TASK);
        return -1;
    }
    return 0;
}

static int start_task_manual(struct frame_fmt *ffp) {
    
	struct task_t *tp;

	tp = get_running_task();
	if(tp != NULL) {
        syslog(LOG_ERR, "task is running; only one task ");
        start_task_rsp(tp, -1, ACTION_OTHER_TASK_RUNNING);
        return -1;
    }

	tp = get_task_by_id(&ffp->task_id[0]);
	if(tp == NULL) {
        syslog(LOG_ERR, "start test task error: can't found task by this id");
        start_task_rsp(tp, -1, ACTION_NO_THIS_TASK);
        return -1;
    }

    return start_task(tp);
}


static int stop_task(struct task_t *tp, int manual) {

    int len;
    char file_name[128];
     
    len = get_upload_file_name(tp, &file_name[0]);
    if(tp->task.len != 0 && tp->status <= TASK_TESTING_AND_UPLOADING) {
        stop_trace(file_name, len);
        update_task(tp, TASK_UPLOADING);
        stop_task_rsp(tp, 1, 0, manual);
        return 0;
    } else {
        syslog(LOG_ERR, "no task to run or task state error");
        stop_task_rsp(tp, -1, ACTION_NO_THIS_TASK, manual);
        return -1;
    }
    return 0;

}

static int stop_task_manual(struct frame_fmt *ffp) {

    struct task_t *tp;

	tp = get_running_task();
	if(tp == NULL) {
        syslog(LOG_ERR, "task is't running; no need to start it");
        start_task_rsp(tp, -1, 0);
        return -1;
    }

    tp = get_task_by_id(&ffp->task_id[0]);
    if(tp == NULL) {
        syslog(LOG_ERR, "start test task at some time error: can't found task by this id");
        int len;
        unsigned char sb[SOCKET_BUFFER_SIZE]={0};
        len = fill_frame_buffer(&sb[0],  TEST_TASK,  CANCEL_TASK_FAILED, 
                &ffp->task_id[0], NULL, 0);

        write_remote_server(&sb[0], len);
        return -1;
    }

    return stop_task(tp, 1);

}

static int cancel_task(struct frame_fmt *ffp) {
    
    int ret = 0;
	struct task_t *tp;
	unsigned char fail_reason=ACTION_NO_THIS_TASK;
	tp = get_task_by_id(&ffp->task_id[0]);
	if(tp == NULL) {
        syslog(LOG_ERR, "start test task error: can't found task by this id");
        int len;
        unsigned char sb[SOCKET_BUFFER_SIZE]={0};
        len = fill_frame_buffer(&sb[0],  TEST_TASK,  STOP_TASK_FAILED, 
                &ffp->task_id[0], &fail_reason, 1);

        write_remote_server(&sb[0], len);
        return ret;
    }
    
    if(tp->status == TASK_QUEUE) {
        del_task(tp);
    } else {
        ret = 1;
    }

    cancel_task_rsp(tp, ret, 0);
    return ret;
}







int64_t get_task_interval() {

    int i;
    //uint64_t ret = REPORT_INTERVAL_SEC+1;
    int64_t ret = 61;
    int64_t tmp;
        
    for(i=0; i<QUEUE_SIZE; i++) {

        if(tt[i].task.len != 0 && tt[i].status == TASK_TESTING_AND_UPLOADING) {
            tmp = datetime_to_unix(tt[i].task.stop_time) - time(NULL);
            tmp<0?0:tmp;
            ret = ret<tmp?ret:tmp;
        }

        if(tt[i].task.len != 0 && tt[i].status == TASK_QUEUE) {
            tmp = datetime_to_unix(tt[i].task.start_time) - time(NULL);
            tmp<0?0:tmp;
            ret = ret<tmp?ret:tmp;
        }

    }

    return ret<=0?1:ret;
}


int check_and_process_task() {

    int i;
    int64_t tmp;

    for(i=0; i<QUEUE_SIZE; i++) {

        if(tt[i].task.len != 0 && tt[i].status == TASK_TESTING_AND_UPLOADING) {
            tmp = datetime_to_unix(tt[i].task.stop_time) - time(NULL);
            if(tmp < 0) {
                stop_task(&tt[i], 0);
            }
        }

        if(tt[i].task.len != 0 && tt[i].status == TASK_QUEUE) {
            tmp = datetime_to_unix(tt[i].task.start_time) - time(NULL);
            if(tmp <= 0) {
                start_task(&tt[i]);
            }
        }


    }

    return 0;

}

int task_process(struct frame_fmt *ffp) {

    int ret;
    syslog(LOG_INFO, "task process");

    switch(ffp->task_state) {

        case RECV_TASK:
            ret = recv_task(ffp);
            break;

        case START_TASK:
            ret = start_task_manual(ffp);
            break;

        case STOP_TASK:
            ret = stop_task_manual(ffp);
            break;

        case CANCEL_TASK:
            ret = cancel_task(ffp);
            break;

        default:
            ret = 0;
            break;
    }

    return 0;
}





