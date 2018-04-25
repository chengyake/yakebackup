#ifndef remote_client_task_queue_h
#define remote_client_task_queue_h

#include <stdio.h>
#include <time.h>
#include <netinet/in.h>

#include "remote_client_connection.h"





#define CB_FILE "TCmd.db"
#define CMD_LINE_SIZE   (64)
#define CMD_LINE_NUM   (1024)
#define CB_SIZE   (CMD_LINE_SIZE*CMD_LINE_NUM)


struct cmd_t {
    unsigned int idx;
    unsigned int len;
    unsigned char data[CMD_LINE_SIZE - sizeof(unsigned int)*2];
};








extern int write_remote_server_by_queue(char *buf, int len);







#endif






