#ifndef remote_client_task_queue_h
#define remote_client_task_queue_h

#include <stdio.h>
#include <time.h>
#include <netinet/in.h>

#include "remote_client_connection.h"
#include "hal-triorail.h"



#define DB_FILE "TTask.db"
#define DB_SIZE   (1024*1024*1)
#define QUEUE_SIZE   (DB_SIZE/sizeof(struct task_t))


enum task_status {
    TASK_QUEUE = 0,
    TASK_TESTING_AND_UPLOADING,
    TASK_UPLOADING,
    TASK_STOP_UPLOADING,
    TASK_DONE,
    TASK_STATUS_MAX,
};

struct task_t {
    unsigned char task_id[FRAME_TASK_ID_SIZE];
    enum task_status status;
    unsigned int access_interval;
    unsigned int online_interval;
    unsigned int offline_interval;
    unsigned int phone_len;
    unsigned char phone_num[16];
    //unsigned int fragment_id;
    struct task_fmt task;
    RIL_task_config_ind cfg;

};

extern int init_task_list();
extern int add_task(struct task_t *task);
extern int del_task(struct task_t *task);
extern int print_task_info(struct task_t *task);
extern struct task_t *get_running_task();
extern struct task_t *get_uploading_task();
extern struct task_t *get_void_task();
extern struct task_t *get_task_by_id(unsigned char *id);


extern struct task_t *tt;


#endif





