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
#include "client_config_parser.h"



static int register_request(struct frame_fmt *fhp) {
    
    int fd;
    int len, size; 

    unsigned char sb[SOCKET_BUFFER_SIZE]={0};
    unsigned char data[128]={0};
    /*unsigned char data[28]={0xFF, 0xFE, 0x43, 0x00, 0x52, 0x00, 0x48, 0x00, 0x33, 
                            0x00, 0x38, 0x00, 0x30, 0x00, 0x42, 0x00, 0x4A, 0x00,
                            0x2D, 0x00, 0x30, 0x00, 0x33, 0x00, 0x30, 0x00, 0x31, 
                            0x00};*/
    //add 
    size = get_train_name(data);
    close(fd);

    //end
    len = fill_frame_buffer(&sb[0], REG_TASK, REG_ACTION, &fhp->task_id[0], 
                            &data[0], size);


    return write_remote_server(&sb[0], len);

}


int reg_task_process(struct frame_fmt *fhp) {

    int ret;
    logdebug("reg task process");

    switch(fhp->task_state) {

        case REG_FAILED:
            ret = register_request(fhp);
            logerr("reg task failed");
            break;

        case REG_SUCCESS:
            loginfo("reg task success");
            break;

        case REG_REQUEST:
            ret = register_request(fhp);
            break;

        default:
            logerr("default process");
            ret = 0;
            break;
    }
    return ret;
}






