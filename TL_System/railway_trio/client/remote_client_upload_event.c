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
#include "client_cmd_queue.h"
#include "remote_client_connection.h"
#include "local_client_connection.h"

static int fragment_id = 0;
static int64_t old_moment = 0;
static int trace_stop=1;


//frame = 1k
#define UPLOAD_FRAME_DATA_SIZE   (1024)
int upload_frame(struct task_t *tp, int fragment_id, unsigned char *content, int length, int final) {

    int len = length;
    struct upload_fmt uf;
    unsigned char frame[SOCKET_BUFFER_SIZE]={0};
    unsigned char data[SOCKET_BUFFER_SIZE]={0};

    uf.fragment_id = fragment_id;
    uf.file_name_len =  get_upload_file_name(tp, &uf.file_name[0]);
    uf.fragment_len = len;
    uf.compress_flag = 0;

    memcpy(&data[0], &uf, 8+uf.file_name_len);
    memcpy(&data[8+uf.file_name_len], content, len);

    //logdebug("len = %d, %s", uf.file_name_len, uf.file_name);

    if(final==0) {
        len = fill_frame_buffer(&frame[0],  UPLOAD_TASK, UPLOAD_UPLOADING, &tp->task_id[0], 
                &data[0], 8+uf.file_name_len+len);
    } else {
        len = fill_frame_buffer(&frame[0],  UPLOAD_TASK, UPLOAD_LATEST, &tp->task_id[0], 
                &data[0], 8+uf.file_name_len+len);
    }

    write_remote_server(&frame[0], len);
}

int upload_file(struct frame_fmt *fhp) {

    int ret;
    int fd;
    int len;
    char file_name[128]={0};
    char content[SOCKET_BUFFER_SIZE]={0};
    struct task_t *tp;
    struct upload_fmt *ufp;

    if(fhp == NULL) {
        logerr("upload file, fhp == null");
        return -1;
    }
    ufp =(struct upload_fmt *)&fhp->data[0];
    tp = get_task_by_id(&fhp->task_id[0]);
    if(tp == NULL) {
        logerr("something wrong, no this task id found in datebase");
        return -1;
    }

    ret = get_upload_file_name(tp, file_name);
    if(ret < 0) {
        logerr("something wrong, no this task id found in datebase");
        return ret;
    }

    fd = open(file_name, O_RDONLY);
    if(fd < 0) {
        logerr("open upload file error");
        return fd;
    }

    len = lseek(fd, 0, SEEK_END);
    if(len < 0) {
        logerr("create TDataBase.db fixed space error");
        close(fd);
        return len;
    }

    fragment_id = ufp->fragment_id;

    logdebug("fragment id=%d, file len=%d", fragment_id, len);

    if(len >= UPLOAD_FRAME_DATA_SIZE*ufp->fragment_id) {
        ret = lseek(fd, UPLOAD_FRAME_DATA_SIZE*(ufp->fragment_id-1), SEEK_SET);
        if(ret < 0) {
            logerr("create TDataBase.db fixed space error");
            close(fd);
            return ret;
        }
        read(fd, &content[0], UPLOAD_FRAME_DATA_SIZE);
        upload_frame(tp, ufp->fragment_id, &content[0],  UPLOAD_FRAME_DATA_SIZE, 0);
        close(fd);
        return 0;//more then a frame
    }


    if(len < UPLOAD_FRAME_DATA_SIZE*ufp->fragment_id && (tp->status != TASK_UPLOADING || trace_stop == 0)) {
        close(fd);
        return 0;//wait full frame
    }

    if(len <= UPLOAD_FRAME_DATA_SIZE*ufp->fragment_id && tp->status == TASK_UPLOADING) {

        ret = lseek(fd, UPLOAD_FRAME_DATA_SIZE*(ufp->fragment_id-1), SEEK_SET);
        if(ret < 0) {
            logerr("create TDataBase.db fixed space error");
            close(fd);
            return ret;
        }
        len = read(fd, &content[0], UPLOAD_FRAME_DATA_SIZE);
        upload_frame(tp, ufp->fragment_id, &content[0],  len, 1);
        close(fd);

        return 0;//last frame
    }

    close(fd);

}


//upload request
void check_and_request(struct task_t *t) {

    int fd;
    int len;
    int ret;
    char file_name[128]={0};
    unsigned char sb[SOCKET_BUFFER_SIZE]={0};
    static int old_file_len;

    static int no_file_check=0;

    ret = get_upload_file_name(t, file_name);
    if(ret < 0) {
        logerr("check and request, no this task id found in datebase");
        return;
    }

    logdebug("file len = %d, file name:%s", ret, file_name);
    fd = open(file_name, O_RDONLY);
    if(fd < 0) {
        logerr("check and request: open upload file %s error %d", file_name, fd);
        if(no_file_check++ >= 6) {
            del_task(t);
            no_file_check = 0;
            logerr("check and request: open upload file error 5 times, del this task now");
        }
        return;
    }

    no_file_check = 0;
    len = lseek(fd, 0, SEEK_END);
    if(len < 0) {
        logerr("gsmr file len < 0 error");
        close(fd);
        return;
    }

    close(fd);

    if(old_file_len > 0 && old_file_len == len) {
        trace_stop=1;
    } else {
        trace_stop=0;
    }
    old_file_len = len;

    loginfo("check and upload %s, fragment_id = %d, file len = %d", file_name, fragment_id, len);
    //status changed
    if(t->status == TASK_UPLOADING) {
        goto request_cmd;
    }

    //big enough
    if(len >= (fragment_id+1)*UPLOAD_FRAME_DATA_SIZE) {
        goto request_cmd;
    }


    return;

request_cmd:

    len = fill_frame_buffer(&sb[0],  UPLOAD_TASK,  UPLOAD_REQUEST, 
            &t->task_id[0], NULL, 0);

    write_remote_server(&sb[0], len);
}




int64_t get_upload_interval() {
    int64_t tmp;
    tmp = time(NULL) - old_moment;
    tmp = UPLOAD_INTERVAL_SEC - tmp;
    return tmp>0?tmp:1;
}

int check_and_upload_task() {

    int64_t ret;
    struct task_t *t;


    int64_t current = time(NULL);
    if(old_moment <= 0) {
        old_moment = current+2;
        return 0;
    }

    ret = current - old_moment;
    if(ret >= UPLOAD_INTERVAL_SEC) {
        old_moment = current;
        t = get_uploading_task();
        if(t != NULL) {
            check_and_request(t);
            check_stop_task();
        }
    }

    return 0;
}

int upload_task_process(struct frame_fmt *fhp) {

    int ret;
    struct task_t *tp;
    logdebug("upload task process");

    switch(fhp->task_state) {

        case UPLOAD_AGREE:
            ret = upload_file(fhp);
            break;

        case UPLOAD_GOON:
            ret = upload_file(fhp);
            break;

        case UPLOAD_SUCCESS:
            tp = get_task_by_id(&fhp->task_id[0]);
            del_task(tp);
            fragment_id = 0;
            loginfo("upload file success, task over");
            break;


        default:
            ret = 0;
            break;
    }
    return 0;
}






