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

#include "client_cmd_queue.h"

static struct cmd_t  *cmd;
static unsigned int cmd_idx=0;


unsigned int get_cmd_max_idx();

int init_cmd_database() {
    int fd, i;
    int ret=-1;

    if(access(CB_FILE, F_OK|R_OK|W_OK) != 0) { //don't exist
        unsigned char data = 0;
        //create db file in xx size
        fd = open(CB_FILE, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            syslog(LOG_ERR, "create TDataBase.db error");
            return ret;
        }
        for(i=0; i<CB_SIZE; i++) {
            ret = write(fd, &data, 1);
            if(ret < 0) {
                syslog(LOG_ERR, "write TDataBase.db error");
                return ret;
            }
        }
    } else {
        //restore db queue
        fd = open(CB_FILE, O_RDWR, S_IRUSR|S_IWUSR);
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

    //mmap and get cmd queue
    cmd = (struct cmd_t *) mmap(0, CB_SIZE , PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(cmd == MAP_FAILED) {
        syslog(LOG_ERR, "mmap TDataBase.db error: %d", errno);
        return -1;
    }
    
    cmd_idx = get_cmd_max_idx();
    //printf("sdf---------------------cmd idx=%d", cmd_idx);
    //munmap((void *)cmd, CB_SIZE) ;//just for test
    close(fd);
    
    return 0;
}

unsigned int get_cmd_max_idx() {

    int i = 0;
    unsigned int idx=0;

    for(i=0; i<CMD_LINE_NUM; i++) {
        if(cmd[i].len != 0 && cmd[i].idx > idx) {
            idx = cmd[i].idx;
        }
    }

    return idx;
}

struct cmd_t *get_void_cmd() {
    int i = 0;
    for(i=0; i<CMD_LINE_NUM; i++) {
        if(cmd[i].len == 0) {
            return &cmd[i];
        }
    }
    return NULL;
}

struct cmd_t *get_cmd_by_idx(unsigned int idx) {
    int i = 0;
    for(i=0; i<CMD_LINE_NUM; i++) {
        if(cmd[i].idx == idx) {
            return &cmd[i];
        }
    }

    return NULL;
}

int add_cmd(unsigned int idx, unsigned char *buf, int len) {

    if(buf == NULL) {
        syslog(LOG_ERR, "add_cmd param error: *buf = NULL");
        return -1;
    }

    struct cmd_t *c = get_void_cmd();
    if(c == NULL) {
        cmd_idx--;
        syslog(LOG_ERR, "get void cmd queue buffer error: cmd = NULL");
        return -1;
    }
    c->idx = idx;
    c->len = len;
    memcpy(&c->data[0], buf, len);
    return 0;
}

int del_cmd(struct cmd_t *c) {

    if(c == NULL) {
        syslog(LOG_ERR, "del_cmd param error: *c = NULL");
        return -1;
    }

    memset((unsigned char *)c, 0, sizeof(struct cmd_t));
    return 0;
}

int close_cmd_queue() {

    munmap((void *)cmd, CB_SIZE) ;
    return 0;
}


int write_remote_server_by_queue(char *buf, int len) {

	int ret;

    if(buf == NULL) {
        syslog(LOG_ERR, "write msg param error: *buf = NULL");
        return -1;
    }

	if(len <=0 || len > CMD_LINE_SIZE) {
        syslog(LOG_ERR, "write msg error: len = %d", len);
        return -1;
    }
    
    ret = add_cmd(cmd_idx++, buf, len); 
    if(ret < 0) {
        syslog(LOG_ERR, "write remote server by queue error: more then %d cmd frame add to queue", CMD_LINE_NUM);
    }
    check_and_send_cmd_queue();

    return ret;
}

int check_and_send_cmd_queue() {
    
    int i, idx=0, ret = 0;
    struct cmd_t *c=NULL;


    do {
        c = get_cmd_by_idx(idx++);
        if(c==NULL) {
            continue;
        }
        ret = write_remote_server(&c->data[0], c->len);
        if(ret < 0) {
            idx--;
        } else {
            del_cmd(c);
        }
    } while(ret >= 0 && idx < cmd_idx && idx < CMD_LINE_NUM);

    if(idx > 0) {
        for(i=0; i<CMD_LINE_NUM; i++) {
            if(cmd[i].idx != 0) {
                cmd[i].idx -= idx;
            }
        }
        cmd_idx -= idx;
    }
    msync((void *)cmd, CB_SIZE ,MS_ASYNC);
}


















