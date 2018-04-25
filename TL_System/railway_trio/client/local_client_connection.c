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
#include "local_client_connection.h"

int local_fd=0;


/*debug API*/
void print_local_frame(struct cmd_header *p) {
#ifdef debug
    int i;
    unsigned char pbf[SOCKET_BUFFER_SIZE]={0};
    logdebug("+++++++++++++++++++++c");
    logdebug("header:");
    logdebug("host:%d target:%d cmd:%d len:%d", p->host, p->target, p->cmd, p->len);
    for(i=0; i<p->len; i++) {
    	sprintf(&pbf[i*3], "%02X ", p->data[i]);
    }
    logdebug("%s", &pbf[0]);
    logdebug("+++++++++++++++++++++c");
#endif
}



int write_local_server(char *buf, int len) {

	int ret;

    if(buf == NULL) {
        logerr("write msg param error: *buf = NULL");
        return -1;
    }

	if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        logerr("write msg error: len = %d", len);
        return -1;
    }

    print_local_frame((struct cmd_header *)buf);
    //printf("write local fd len = %d", len);
    ret = write(local_fd, buf, len);
    if(ret < 0) {
        logerr("write msg error: %s", strerror(ret));
        return ret;
    }

    return ret;
}


int read_local_server(char *buf, int len) {

	int ret;

    if(buf == NULL) {
        logerr("read msg param error: *buf = NULL");
        return -1;
    }

    if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        logerr("read msg error: len = %d", len);
        return -1;
    }

    memset(buf, 0, len);
    ret = read(local_fd, buf, len);
    if(ret < 0) {
        logerr("read msg error: %s", strerror(ret));
    }

    return ret;
}


int recv_local_server() {
    int sum;
    int idx=0;
    static int len = 0;//last
    static struct cmd_header *chp=NULL;
    static unsigned char buffer[SOCKET_BUFFER_SIZE]={0};

    

    sum = recv(local_fd, &buffer[len], SOCKET_BUFFER_SIZE, 0);
    if(sum <= 0) {
        logerr("read local server error1 %d", sum);
        close(local_fd);
        len = 0;
        local_fd=0;
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
        process_local_event(chp);
        idx+=chp->len + CMD_HEADER_SIZE;
    }

    len = sum-idx;
    if(idx != 0) memmove(&buffer[0], &buffer[idx], len);
    memset(&buffer[sum-idx], 0, SOCKET_BUFFER_SIZE - len);

    return 0;

  }


void attach_local_server_thread_func() {

    struct sockaddr_un server_addr;

    local_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    pthread_detach(pthread_self());

    server_addr.sun_family = AF_UNIX;
    strcpy (server_addr.sun_path, LOCAL_SOCKET);

    loginfo("attach framework core, connect to LOCAL_SOCKET");


    while(connect(local_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        logerr("attache_core error");
        sleep(2);
    }

    //sleep(1);
    fcntl(local_fd, F_SETFL, O_NONBLOCK);
    register_client_module();
}


int attach_local_server() {
    int ret;
	pthread_t pid;

	ret = pthread_create(&pid, NULL, (void *)attach_local_server_thread_func, NULL);
	if(ret != 0) {
        logerr("pthread create error: %d", ret);
    }

    return ret;
}

int deattach_local_server() {

    loginfo("de-attach framework core");
    close(local_fd);

    return 0;

}


