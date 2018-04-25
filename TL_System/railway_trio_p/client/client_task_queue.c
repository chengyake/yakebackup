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
    return (int64_t)(((date_time & 0x3FFFFFFFFFFFFFFFLL)-621355968000000000LL)/10000000LL);
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
            syslog(LOG_ERR, "create TDataBase.db error");
            return ret;
        }
        for(i=0; i<DB_SIZE; i++) {
            ret = write(fd, &data, 1);
            if(ret < 0) {
                syslog(LOG_ERR, "write TDataBase.db error");
                return ret;
            }
        }
    } else {
        //restore db queue
        fd = open(DB_FILE, O_RDWR, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            syslog(LOG_ERR, "open TDataBase.db error");
            return ret;
        }
    }

    ret = lseek(fd, 0, SEEK_SET);
    if(ret < 0) {
        syslog(LOG_ERR, "create TDataBase.db fixed space error");
        return ret;
    }

    //mmap and get tt
    tt = (struct task_t *) mmap(0, DB_SIZE , PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0) ;
    if(tt == MAP_FAILED) {
        syslog(LOG_ERR, "mmap TDataBase.db error: %d", errno);
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
        syslog(LOG_ERR, "params of add task() error");
        return -1;
    }
    msync((void *)tt, DB_SIZE ,MS_ASYNC);
    return 0;
}

int update_task(struct task_t *task, enum task_status status) {
    //change task[n] and msync
    
    if(task == NULL) {
        syslog(LOG_ERR, "params of update task() error");
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
        syslog(LOG_ERR, "params of del task() error: *task == NULL");
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
        syslog(LOG_ERR, "try to dump task, but task == NULL");
        return -1;
    }

    syslog(LOG_INFO, "---------------------");
    syslog(LOG_INFO, "task_id:");
    for(i=0; i<16; i++) {
        //syslog(LOG_INFO, "0x%x ", t->task_id[i]);
    	sprintf(&pbf[i*3], "%02X ", t->task_id[i]);
    }
    syslog(LOG_INFO, "%s", &pbf[0]);
    syslog(LOG_INFO, "task_status %d", t->status);
    syslog(LOG_INFO, "online %x", t->online_interval);
    syslog(LOG_INFO, "offline %x", t->offline_interval);
    syslog(LOG_INFO, "phone len %d :", t->phone_len);
    for(i=0; i<t->phone_len; i++) {
        //syslog(LOG_INFO, "0x%x ", t->phone_num[i]);
    	sprintf(&pbf[i*3], "%02X ", t->phone_num[i]);
    }
    pbf[i*3+1]=0;;
    syslog(LOG_INFO, "%s", &pbf[0]);
    syslog(LOG_INFO, "---------------------");

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

struct task_t *get_uploading_task() {
    //check task[n] status
    int i;
    for(i=0; i<QUEUE_SIZE; i++) {
        if(tt[i].status == TASK_TESTING_AND_UPLOADING || tt[i].status == TASK_UPLOADING) {
            return &tt[i];
        }
    }
    return NULL;
}



struct task_t *get_task_by_id(unsigned char *id) {

    //check task[n] task_id
    int i, j;
    int flag;

    if(id == NULL) {
        syslog(LOG_ERR, "params of get_task_by_id() error: *id == NULL");
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
        syslog(LOG_ERR, "params of check_new_task() error: *tfp == NULL");
        return -1;
    }

    if( time(NULL) >=  datetime_to_unix(tfp->stop_time)) {
        syslog(LOG_ERR, "expired task");
        return -1;
    }
    for(i=0; i<QUEUE_SIZE; i++) {
        if(tt[i].task.len != 0) {
            if(!(tfp->stop_time <  tt[i].task.start_time || tfp->start_time >  tt[i].task.stop_time))  {
                syslog(LOG_ERR, "repeat task time");
                return -1;
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
        syslog(LOG_ERR, "params of get_upload_file_name() error");
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


