#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/tcp.h>


#include "md5.h"
#include "../core/core.h"
#include "remote_client_connection.h"
#include "client_config_parser.h"

int remote_fd=-1;

int write_remote_server(char *buf, int len) {

	int idx=0;
	int ret;

    if(buf == NULL) {
        logerr("write_remote_server param error: *buf = NULL");
        return -1;
    }

	if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        logerr("write_remote_server error: len = %d", len);
        return -1;
    }
    
    if(remote_fd <= 0) {
        logerr("write remote_server error: remote fd is null");
        return -1;
    }

    //debug
    debug_print_frame((struct frame_fmt *)buf);
    do {
        ret = write(remote_fd, &buf[idx], len);
        if(ret < 0) {
            logerr("write remote_server error: %s", strerror(ret));
            return ret;
        }
        idx += ret;

    } while (idx < len);



    return ret;
}

int read_remote_server(char *buf, int len) {

	int ret;

    if(buf == NULL) {
        logerr("read msg param error: *buf = NULL");
        return -1;
    }

    if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        logerr("read msg error: len = %d", len);
        return -1;
    }

    if(remote_fd <= 0) {
        logerr("read msg error: remote fd is null");
        return -1;
    }


    memset(buf, 0, len);
    ret = read(remote_fd, buf, len);
    if(ret < 0) {
        logerr("read msg error: %s", strerror(ret));
    }

    return ret;
}



void attach_remote_server_thread_func() {
    
    int tmp_fd;
    int keepAlive = 1;
    int keepIdle = 14;
    int keepInterval = 2;
    int keepCount = 3;
    struct sockaddr_in server_addr;
    char server_ip[32] = {0};
    unsigned int server_port;

    get_server_ip(server_ip);
    server_port = get_server_port();


    pthread_detach(pthread_self());

    tmp_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr=inet_addr(server_ip);
    //inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    logerr("attach remote server, connect to %s:%d", server_ip, server_port);

    fcntl(tmp_fd, F_SETFL, O_NONBLOCK);

    setsockopt(tmp_fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
    setsockopt(tmp_fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    setsockopt(tmp_fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
    setsockopt(tmp_fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));


    while(connect(tmp_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        //logdebug("attach remote server, connect to %s:%d error", server_ip, server_port);
        logerr("attach remote server, connect to %s:%d error", server_ip, server_port);
        sleep(5);
    }
    
    remote_fd = tmp_fd;
}

int attach_remote_server() {
    int ret;
	pthread_t pid;
    
    if(remote_fd == 0) {
        return 0;
    } else {
        if(remote_fd > 0) {
            close(remote_fd);
        }
        remote_fd = 0;
    }
	ret = pthread_create(&pid, NULL, (void *)attach_remote_server_thread_func, NULL);
	if(ret != 0) {
        logerr("pthread create error: %d", ret);
    }

    return ret;
}

int deattach_remote_server() {

    logdebug("de-attach framework core");
    close(remote_fd);

    return 0;

}

int recv_remote_server() {
    int ret;
    static int len = 0;
    static struct frame_fmt *chp=NULL;
    static unsigned char buffer[SOCKET_BUFFER_SIZE]={0};

    if(len < FRAME_FMT_SIZE) {
        ret= recv(remote_fd, &buffer[len], FRAME_FMT_SIZE-len, 0);
        if(ret <= 0) {
            logerr("read remote server error1 %d", ret);
            attach_remote_server();
            return -1;
        }
        len+=ret;
    }
    if(len >= FRAME_FMT_SIZE) {
        chp = (struct frame_fmt *)&buffer[0];

        if(len >= chp->len) goto recv_over;//check

        ret = recv(remote_fd, &buffer[len], chp->len - len, 0);
        if(ret <= 0) {
            logerr("read remote server error2 %d", ret);
            attach_remote_server();
            return -1;
        }
        len+=ret;

        if(len >= chp->len) goto recv_over;//check
    }

    return 0;

recv_over:
    //debug
    debug_print_frame((struct frame_fmt*)&buffer[0]);
    process_remote_event((struct frame_fmt *)&buffer[0]);
    memset(buffer, 0, SOCKET_BUFFER_SIZE);
    len = 0;
    return 0;
}


