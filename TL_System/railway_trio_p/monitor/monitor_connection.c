#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <syslog.h>
#include <sys/un.h>
#include <fcntl.h>

#include "../core/core.h"
#include "monitor.h"
#include "monitor_connection.h"



static int monitor_fd=0;
static fd_set fd_sets;
static int event_loop=1;

/*debug API*/
void print_monitor_frame(struct cmd_header *p) {
#if 1
    int i;
    unsigned char pbf[SOCKET_BUFFER_SIZE]={0};
    syslog(LOG_INFO, "\t+++++++++++++++++++++m\n");
    syslog(LOG_INFO, "\theader:\n");
    syslog(LOG_INFO, "\thost:%d target:%d cmd:%d len:%d\n\t", p->host, p->target, p->cmd, p->len);
    for(i=0; i<p->len; i++) {
    	sprintf(&pbf[i*3], "%02X ", p->data[i]);
    }
    syslog(LOG_INFO, "%s\n", &pbf[0]);
    syslog(LOG_INFO, "\t+++++++++++++++++++++m\n");
#endif
}



int write_monitor_server(char *buf, int len) {

    int ret;

    if(buf == NULL) {
        syslog(LOG_ERR, "write msg param error: *buf = NULL\n");
        return -1;
    }

    if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        syslog(LOG_ERR, "write msg error: len = %d\n", len);
        return -1;
    }

    print_monitor_frame((struct cmd_header *)buf);
    //printf("write monitor fd len = %d\n", len);
    ret = write(monitor_fd, buf, len);
    if(ret < 0) {
        syslog(LOG_ERR, "write msg error: %s\n", strerror(ret));
        return ret;
    }

    return ret;
}


int read_monitor_server(char *buf, int len) {

    int ret;

    if(buf == NULL) {
        syslog(LOG_ERR, "read msg param error: *buf = NULL\n");
        return -1;
    }

    if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        syslog(LOG_ERR, "read msg error: len = %d\n", len);
        return -1;
    }

    memset(buf, 0, len);
    ret = read(monitor_fd, buf, len);
    if(ret < 0) {
        syslog(LOG_ERR, "read msg error: %s\n", strerror(ret));
    }

    return ret;
}


int recv_monitor_server() {
    int sum;
    int idx=0;
    static int len = 0;//last
    static struct cmd_header *chp=NULL;
    static unsigned char buffer[SOCKET_BUFFER_SIZE]={0};



    sum = recv(monitor_fd, &buffer[len], SOCKET_BUFFER_SIZE, 0);
    if(sum <= 0) {
        syslog(LOG_ERR, "read monitor server error1 %d\n", sum);
        close(monitor_fd);
        len = 0;
        monitor_fd=0;
        memset(buffer, 0, SOCKET_BUFFER_SIZE);
        //exit(-1);
        return 0;
    }

    sum+=len;

    while (1) {
        chp = (struct cmd_header *)&buffer[idx];
        if(sum < chp->len + CMD_HEADER_SIZE + idx)  {
            break;//wait
        }
        process_monitor_event(chp);
        idx+=chp->len + CMD_HEADER_SIZE;
    }

    len = sum-idx;
    if(idx != 0) memmove(&buffer[0], &buffer[idx], len);
    memset(&buffer[sum-idx], 0, SOCKET_BUFFER_SIZE - len);

    return 0;

}
static int clients_data_changed() {
    int ret=0;

    if (monitor_fd > 0 && FD_ISSET(monitor_fd, &fd_sets)) {
        ret = recv_monitor_server();
        if(ret < 0) {
            syslog(LOG_ERR, "recv and process error %d\n", ret);
            return ret;
        }
    }
    return ret;
}



void attach_monitor_server_thread_func() {

    struct sockaddr_un server_addr;

    monitor_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    server_addr.sun_family = AF_UNIX;
    strcpy (server_addr.sun_path, LOCAL_SOCKET);

    syslog(LOG_INFO, "attach framework core, connect to LOCAL_SOCKET\n");


    while(connect(monitor_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        syslog(LOG_INFO, "attache_core error\n");
        sleep(2);
    }

    //sleep(1);
    fcntl(monitor_fd, F_SETFL, O_NONBLOCK);
    register_client_module();
}


int attach_monitor_server() {
    int ret;
    pthread_t pid;

    ret = pthread_create(&pid, NULL, (void *)attach_monitor_server_thread_func, NULL);
    if(ret != 0) {
        syslog(LOG_ERR, "pthread create error: %d\n", ret);
    }

    return ret;
}

int deattach_monitor_server() {

    syslog(LOG_INFO, "de-attach framework core\n");
    close(monitor_fd);

    return 0;

}


int setup_monitor_event_loop() {

    int ret;
    struct timeval timer={0, 0};

    while (event_loop) {

        if(monitor_fd <= 0) {
            syslog(LOG_ERR, "can't set fd_sets: monitor fd < 0. return = %d\n", ret);
            setup_all_process();//
            sleep(1);
            attach_monitor_server();
            sleep(1);
        }
        FD_ZERO(&fd_sets);
        FD_SET(monitor_fd, &fd_sets);

        timer.tv_sec = CHECK_PROCESS_INTERVAL;
        ret = select(monitor_fd+1, &fd_sets, NULL, NULL, &timer);
        if (ret < 0) {
            syslog(LOG_ERR, "select error: return = %d\n", ret);
            sleep(1);
            continue;
        } else if (ret == 0) {
            syslog(LOG_INFO, "select timeout: return = %d\n", ret);
        }

        //syslog(LOG_INFO, "client data comming\n");
        check_all_process();
        clients_data_changed();//add del cancel stop task

    }

    return ret;
}





