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

static int fragment_id = 0;
static int64_t old_moment = 0;
static int trace_stop=1;


//debug interface
#if 0
int upload_file(struct frame_fmt *fhp) {
	int fd, i, len;
	struct task_t *tp;
    struct upload_fmt uf;
    struct upload_fmt *up;
	int read_len, limit_len;
    unsigned char sb[SOCKET_BUFFER_SIZE]={0};
    
    up = (struct upload_fmt *)&fhp->data[0];
    tp = get_task_by_id(&fhp->task_id[0]);

    if(tp == NULL) {
        syslog(LOG_ERR, "something wrong, no this task id found");
        return 0;
    }

    if(tp->status != TASK_TESTING_AND_UPLOADING && tp->status != TASK_UPLOADING) {
        syslog(LOG_ERR, "something wrong, no need to upload this task");
        return 0;
    }


    uf.fragment_id = up->fragment_id;
    uf.file_name_len =  get_upload_file_name(tp, &uf.file_name[0]);
    uf.fragment_len = 1024 - 8 - uf.file_name_len;
    uf.compress_flag = 0;
    
    //syslog(LOG_INFO,"len = %d, %s", uf.file_name_len, uf.file_name);
    if(tp->status == TASK_TESTING_AND_UPLOADING) {
        //one
        len = fill_frame_buffer(&sb[0],  UPLOAD_TASK, UPLOAD_UPLOADING, &fhp->task_id[0], 
                &uf, 1024);
        write_remote_server(&sb[0], len);
    }

    if(tp->status == TASK_UPLOADING) {
        //last one
        len = fill_frame_buffer(&sb[0],  UPLOAD_TASK, UPLOAD_LATEST, &fhp->task_id[0], 
                &uf, 1024);
        write_remote_server(&sb[0], len);
    }





/*
	tp = get_task_by_id(&fhp->task_id[0]);
    fd = open(&tp->task.file_name[0], O_RDONLY);
    
    limit_len = SOCKET_BUFFER_SIZE - FRAME_FMT_SIZE;
    for(i=0; ;i++){
    	read_len = read(fd, data, limit_len);
    	if(read_len < 0) {
            syslog(LOG_ERR, "upload task process");
            return -1;
    	} else if(read_len == limit_len) {
            len = fill_frame_buffer(&sb[0],  UPLOAD_TASK, UPLOAD_UPLOADING, &fhp->task_id[0], 
                    &data[0], read_len);
            write_remote_server(&sb[0], len);
        } else if(read_len < limit_len) {
            len = fill_frame_buffer(&sb[0],  UPLOAD_TASK, UPLOAD_LATEST, &fhp->task_id[0], 
                    &data[0], read_len);
            write_remote_server(&sb[0], len);
        }

    }
*/
    return 0;
}


#else 



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

    //debug
    //syslog(LOG_INFO,"len = %d, %s", uf.file_name_len, uf.file_name);

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
        syslog(LOG_ERR, "upload file, fhp == null");
        return -1;
    }
    syslog(LOG_INFO,"111111111111111111");
    ufp =(struct upload_fmt *)&fhp->data[0];
    tp = get_task_by_id(&fhp->task_id[0]);
    if(tp == NULL) {
        syslog(LOG_ERR, "something wrong, no this task id found in datebase");
        return -1;
    }

    ret = get_upload_file_name(tp, file_name);
    if(ret < 0) {
        syslog(LOG_ERR, "something wrong, no this task id found in datebase");
        return ret;
    }

    syslog(LOG_INFO,"1111111111111111112222");
    fd = open(file_name, O_RDONLY);
    if(fd < 0) {
        syslog(LOG_ERR, "open upload file error");
        return fd;
    }

    syslog(LOG_INFO,"11111111111111111122223333");
    len = lseek(fd, 0, SEEK_END);
    if(len < 0) {
        syslog(LOG_ERR, "create TDataBase.db fixed space error");
        close(fd);
        return len;
    }
    
    fragment_id = ufp->fragment_id;

    syslog(LOG_INFO,"fragment id=%d, file len=%d", fragment_id, len);

    if(len >= UPLOAD_FRAME_DATA_SIZE*ufp->fragment_id) {
        ret = lseek(fd, UPLOAD_FRAME_DATA_SIZE*(ufp->fragment_id-1), SEEK_SET);
        if(ret < 0) {
            syslog(LOG_ERR, "create TDataBase.db fixed space error");
            close(fd);
            return ret;
        }
        read(fd, &content[0], UPLOAD_FRAME_DATA_SIZE);
        upload_frame(tp, ufp->fragment_id, &content[0],  UPLOAD_FRAME_DATA_SIZE, 0);
        close(fd);
        syslog(LOG_INFO,"11111111111111111122223333444");
        return 0;//more then a frame
    }


    if(len < UPLOAD_FRAME_DATA_SIZE*ufp->fragment_id && (tp->status != TASK_UPLOADING || trace_stop == 0)) {
        syslog(LOG_INFO,"11111111111111111122223333555");
        close(fd);
        return 0;//wait full frame
    }

    if(len <= UPLOAD_FRAME_DATA_SIZE*ufp->fragment_id && tp->status == TASK_UPLOADING) {

        ret = lseek(fd, UPLOAD_FRAME_DATA_SIZE*(ufp->fragment_id-1), SEEK_SET);
        if(ret < 0) {
            syslog(LOG_ERR, "create TDataBase.db fixed space error");
            close(fd);
            return ret;
        }
        len = read(fd, &content[0], UPLOAD_FRAME_DATA_SIZE);
        upload_frame(tp, ufp->fragment_id, &content[0],  len, 1);
        close(fd);

        syslog(LOG_INFO,"11111111111111111122223333666");

        return 0;//last frame
    }

}

#endif


//upload request
void check_and_request(struct task_t *t) {

    int fd;
	int len;
    int ret;
    char file_name[128]={0};
    unsigned char sb[SOCKET_BUFFER_SIZE]={0};
    static int old_file_len;

    ret = get_upload_file_name(t, file_name);
    if(ret < 0) {
        syslog(LOG_ERR, "check and request, no this task id found in datebase");
        return;
    }

    syslog(LOG_INFO,"file len = %d, file name:%s", ret, file_name);
    fd = open(file_name, O_RDONLY);
    if(fd < 0) {
        syslog(LOG_ERR, "check and request: open upload file error");
        return;
    }

    len = lseek(fd, 0, SEEK_END);
    if(len < 0) {
        syslog(LOG_ERR, "gsmr file len < 0 error");
        close(fd);
        return;
    }
    
    if(old_file_len > 0 && old_file_len == len) {
        trace_stop=1;
    } else {
        trace_stop=0;
    }
    old_file_len = len;

    syslog(LOG_INFO,"check and upload, fragment_id = %d, file len = %d", fragment_id, len);
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
        old_moment = current;
        return 0;
    }

    ret = current - old_moment;
    if(ret >= UPLOAD_INTERVAL_SEC) {
        old_moment = current;
        t = get_uploading_task();
        if(t != NULL) {
            check_and_request(t);
        }
    }

    return 0;
}

int upload_task_process(struct frame_fmt *fhp) {

    int ret;
    struct task_t *tp;
    syslog(LOG_INFO, "upload task process");

    switch(fhp->task_state) {

        case UPLOAD_AGREE:
            syslog(LOG_INFO,"11111111111111111100000000");
            ret = upload_file(fhp);
            break;

        case UPLOAD_GOON:
            ret = upload_file(fhp);
            break;

        case UPLOAD_SUCCESS:
            tp = get_task_by_id(&fhp->task_id[0]);
            del_task(tp);
            fragment_id = 0;
            syslog(LOG_INFO, "upload file success, task over");
            break;


        default:
            ret = 0;
            break;
    }
    return 0;
}






