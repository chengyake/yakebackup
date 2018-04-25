#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/mman.h>
#include <errno.h>

#include "../core/core.h"
#include "client_task_queue.h"

struct task_t * tt = NULL;



int64_t datetime_to_unix(uint64_t data) {

    uint64_t date_time=data;
    return (int64_t)(((date_time & 0x3FFFFFFFFFFFFFFFLL)-621355968000000000LL)/10000000LL); //  + (3600*8);
}

unsigned long unix_to_datetime(unsigned char *data, uint64_t utime) {
    unsigned long ret;
    ret = utime*10000000 + 621355968000000000LL;
}

int init_task_database() {
    int fd, i;
    int ret=-1;

    if(access(DB_FILE, F_OK|R_OK|W_OK) != 0) {//don't exist
        unsigned char data = 0;
        //create db file in xx size
        fd = open(DB_FILE, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            logerr("create TDataBase.db error");
            return ret;
        }
        for(i=0; i<DB_SIZE; i++) {
            ret = write(fd, &data, 1);
            if(ret < 0) {
                logerr("write TDataBase.db error");
                return ret;
            }
        }
    } else {
        //restore db queue
        fd = open(DB_FILE, O_RDWR, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            logerr("open TDataBase.db error");
            return ret;
        }
    }

    ret = lseek(fd, 0, SEEK_SET);
    if(ret < 0) {
        logerr("create TDataBase.db fixed space error");
        return ret;
    }

    //mmap and get tt
    tt = (struct task_t *) mmap(0, DB_SIZE , PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0) ;
    if(tt == MAP_FAILED) {
        logerr("mmap TDataBase.db error: %d", errno);
        return -1;
    }

    //munmap((void *)tt, DB_SIZE) ;//just for test
    close(fd);

    return 0;
}


struct task_t *get_void_task() {
    int i;
    for(i=0; i<QUEUE_SIZE; i++) {
        if(tt[i].task.len == 0) {
            return &tt[i];
        }
    }

    return NULL;
}


int add_task(struct task_t *task) {
    //fill task[n] and msync
    if(task == NULL) {
        logerr("params of add task() error");
        return -1;
    }
    msync((void *)tt, DB_SIZE ,MS_ASYNC);
    return 0;
}

int update_task(struct task_t *task, enum task_status status) {
    //change task[n] and msync

    if(task == NULL) {
        logerr("params of update task() error");
        return -1;
    }
    task->status = status;
    msync((void *)tt, DB_SIZE ,MS_ASYNC);
    print_all_task_info();
    return 0;

}

int del_task(struct task_t *task) {
    //memset task[n] and msync
    if(task == NULL) {
        logerr("params of del task() error: *task == NULL");
        return -1;
    }
    memset(task, 0, sizeof(struct task_t));
    msync((void *)tt, DB_SIZE ,MS_ASYNC);
    return 0;
}


int close_task_queue() {
    //msync and munmap

    munmap((void *)tt, DB_SIZE) ;
    return 0;
}

/*debug api*/
int print_task_info(struct task_t *t) {
    int i;

    unsigned char pbf[SOCKET_BUFFER_SIZE]={0};
    if(t == NULL) {
        logerr("try to dump task, but task == NULL");
        return -1;
    }

    loginfo("---------------------");
    loginfo("task_id:");
    for(i=0; i<16; i++) {
        sprintf(&pbf[i*3], "%02X ", t->task_id[i]);
    }
    loginfo("%s", &pbf[0]);
    loginfo("task_status %d", t->status);
    loginfo("online %x", t->online_interval);
    loginfo("offline %x", t->offline_interval);
    loginfo("phone len %d :", t->phone_len);
    for(i=0; i<t->phone_len; i++) {
        sprintf(&pbf[i*3], "%02X ", t->phone_num[i]);
    }
    pbf[i*3+1]=0;;
    loginfo("%s", &pbf[0]);
    loginfo("---------------------");

}
/*debug api*/
int print_all_task_info() {
    int i;
    for(i=0; i<QUEUE_SIZE; i++) {
        if(tt[i].task.len != 0) {
            print_task_info(&tt[i]);
        }
    }
    return 0;
}



struct task_t *get_running_task() {
    //check task[n] status
    int i;
    for(i=0; i<QUEUE_SIZE; i++) {
        if(tt[i].status == TASK_TESTING_AND_UPLOADING) {
            return &tt[i];
        }
    }
    return NULL;
}

struct task_t *get_stop_task() {
    int i;
    for(i=0; i<QUEUE_SIZE; i++) {
        if(tt[i].status == TASK_STOP_AUTO || tt[i].status == TASK_STOP_MANU) {
            return &tt[i];
        }
    }
    return NULL;
}

struct task_t *get_uploading_task() {

    int i;
    static uint64_t idx = 0;
    static uint64_t start_time = 0;
    struct task_t *r = NULL;


    if(start_time > 0 && tt[idx].task.start_time == start_time) {
        return &tt[idx];
    } else {
        idx = 0;
        start_time = 0;
        for(i=0; i<QUEUE_SIZE; i++) {
            if(tt[i].status == TASK_TESTING_AND_UPLOADING || tt[i].status == TASK_UPLOADING) {
                if (r == NULL) {
                    idx = i;
                    start_time = tt[i].task.start_time;
                    r = &tt[i];
                } else {
                    if(tt[i].task.start_time < r->task.start_time) {
                        idx = i;
                        start_time = tt[i].task.start_time;
                        r = &tt[i];
                    }
                }
            }
        }
    }

    return r;
}

struct task_t *get_task_by_id(unsigned char *id) {

    //check task[n] task_id
    int i, j;
    int flag;

    if(id == NULL) {
        logerr("params of get_task_by_id() error: *id == NULL");
        return NULL;
    }

    for(i=0; i<QUEUE_SIZE; i++) {
        flag = 1;
        for(j=0; j<FRAME_TASK_ID_SIZE; j++) {
            if(tt[i].task_id[j] != id[j]) {
                flag = 0;
                break;
            }
        }
        if(flag == 1) return &tt[i];
    }

    return NULL;
}


int check_new_task(struct task_fmt *tfp) {
    int i;

    if(tfp == NULL) {
        logerr("params of check_new_task() error: *tfp == NULL");
        return -1;
    }

    if( time(NULL) >=  datetime_to_unix(tfp->stop_time)) {
        logerr("expired task");
        return -ACTION_EXPIRED_ERR ;
    }
    for(i=0; i<QUEUE_SIZE; i++) {
        if(tt[i].task.len != 0) {
            if(!(tfp->stop_time <  tt[i].task.start_time || tfp->start_time >  tt[i].task.stop_time))  {
                if(tfp->start_time ==  tt[i].task.start_time && tfp->stop_time ==  tt[i].task.stop_time) {
                    logerr("repeat task id");
                    return -ACTION_RECV_REPEAT_TASK;
                }
                logerr("repeat task time");
                return -ACTION_EXPIRED_ERR;
            }
        }
    }

    return 0;
}

int get_upload_file_name(struct task_t *t, char *data) {

    int i=0;
    int64_t tmp;
    struct tm *p;

    if(t == NULL || data == NULL) {
        logerr("params of get_upload_file_name() error");
        return -1;
    }

    for(i=0; i<t->task.file_name_len; i++) {
        data[i] = t->task.file_name[i];
    }
    data[i++] =  0x5f;  //"_"
    if(t->task.direction == 0) {
        //shangxing
        data[i++]=0xe4;
        data[i++]=0xb8;
        data[i++]=0x8a; 
        data[i++]=0xe8; 
        data[i++]=0xa1; 
        data[i++]=0x8c;
    } else {
        //xiaxing
        data[i++]=0xe4;
        data[i++]=0xb8;
        data[i++]=0x8b; 
        data[i++]=0xe8; 
        data[i++]=0xa1; 
        data[i++]=0x8c;
    }

    tmp = datetime_to_unix(t->task.start_time);
    p=localtime((time_t *)&tmp);
    sprintf(&data[i], "_%4d%02d%02d_%02d%02d.gsmr",
            (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min);

    i+=19;
    return i;

}


