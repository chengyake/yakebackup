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

int core_fd=0;


/*debug API*/
void print_core_frame(struct cmd_header *p) {
#if debug
    int i;
    unsigned char pbf[SOCKET_BUFFER_SIZE]={0};
    logdebug("\t+++++++++++++++++++++g\n");
    logdebug("\theader:\n");
    logdebug("\thost:%d target:%d cmd:%d len:%d\n\t", p->host, p->target, p->cmd, p->len);
    for(i=0; i<p->len; i++) {
    	sprintf(&pbf[i*3], "%02X ", p->data[i]);
    }
    logdebug("%s\n", &pbf[0]);
    logdebug("\t+++++++++++++++++++++g\n");
#endif
}



int write_core_server(char *buf, int len) {

	int ret;

    if(buf == NULL) {
        logerr( "write msg param error: *buf = NULL\n");
        return -1;
    }

	if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        logerr( "write msg error: len = %d\n", len);
        return -1;
    }

    print_core_frame((struct cmd_header *)buf);
    //printf("write core fd len = %d\n", len);
    ret = write(core_fd, buf, len);
    if(ret < 0) {
        logerr( "write msg error: %s\n", strerror(ret));
        return ret;
    }

    return ret;
}


int read_core_server(char *buf, int len) {

	int ret;

    if(buf == NULL) {
        logerr( "read msg param error: *buf = NULL\n");
        return -1;
    }

    if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        logerr( "read msg error: len = %d\n", len);
        return -1;
    }

    memset(buf, 0, len);
    ret = read(core_fd, buf, len);
    if(ret < 0) {
        logerr( "read msg error: %s\n", strerror(ret));
    }

    return ret;
}


int recv_core_server() {
    int sum;
    int idx=0;
    static int len = 0;//last
    static struct cmd_header *chp=NULL;
    static unsigned char buffer[SOCKET_BUFFER_SIZE]={0};

    sum = recv(core_fd, &buffer[len], SOCKET_BUFFER_SIZE, 0);
    if(sum <= 0) {
        logerr( "read core server error1 %d\n", sum);
        close(core_fd);
        len = 0;
        core_fd=0;
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
        process_core_event(chp);
        idx+=chp->len + CMD_HEADER_SIZE;
    }

    len = sum-idx;
    if(idx != 0) memmove(&buffer[0], &buffer[idx], len);
    memset(&buffer[sum-idx], 0, SOCKET_BUFFER_SIZE - len);

    return 0;

  }


void attach_core_server_thread_func() {

    struct sockaddr_un server_addr;

    core_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    server_addr.sun_family = AF_UNIX;
    strcpy (server_addr.sun_path, LOCAL_SOCKET);

    loginfo("attach framework core, connect to LOCAL_SOCKET\n");


    while(connect(core_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        logerr("attache_core error\n");
        sleep(2);
    }

    //sleep(1);
    fcntl(core_fd, F_SETFL, O_NONBLOCK);
    register_client_module();
}


int attach_core_server() {
    int ret;
	pthread_t pid;

	ret = pthread_create(&pid, NULL, (void *)attach_core_server_thread_func, NULL);
	if(ret != 0) {
        logerr( "pthread create error: %d\n", ret);
    }

    return ret;
}

int deattach_core_server() {

    loginfo("de-attach framework core\n");
    close(core_fd);

    return 0;

}


