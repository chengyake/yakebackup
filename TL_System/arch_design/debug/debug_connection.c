#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <syslog.h>

#include "../core/core.h"


extern void debug_event_process(struct cmd_header *p);

int debug_fd;
fd_set fd_sets;
int event_loop=1;

int set_select_sets() {

    FD_ZERO(&fd_sets);
    FD_SET(debug_fd, &fd_sets);
    return 0;
}

int get_sets_max() {

    return debug_fd;
}

int recv_and_process() {

    int len;
    struct cmd_header *chp;
    unsigned char buffer[SOCKET_BUFFER_SIZE]={0};

    len = recv(debug_fd, &buffer[0], CMD_HEADER_SIZE, 0);
    if (len == 0) {
        syslog(LOG_ERR, "clients connection changed, recv return = %d\n", len);
    } else if(len == CMD_HEADER_SIZE) {
        chp = (struct cmd_header *)&buffer[0];
        if( chp->len > 0) {
            len = recv(debug_fd, &chp->data[0], chp->len, 0);
        }
        debug_event_process((struct cmd_header *)&buffer[0]);
    } else {
        syslog(LOG_ERR, "clients data changed, recv error: %s\n", strerror(len));
    }

    return 0;
}

int clients_data_changed() {

    if (FD_ISSET(debug_fd, &fd_sets)) {
        return recv_and_process();
    }

    return -1;
}

int debug_write_msg(char *buf, int len) {

	int ret;

    if(buf == NULL) {
        syslog(LOG_ERR, "write msg param error: *buf = NULL\n");
        return -1;
    }
	if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        syslog(LOG_ERR, "write msg error: len = %d\n", len);
        return -1;
    }
    ret = write(debug_fd, buf, len);
    if(ret < 0) {
        syslog(LOG_ERR, "write msg error: %s\n", strerror(ret));
        return ret;
    }

    return ret;
}


int debug_read_msg(char *buf, int len) {

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
    ret = read(debug_fd, buf, len);
    if(ret < 0) {
        syslog(LOG_ERR, "read msg error: %s\n", strerror(ret));
    }

    return ret;
}


int attach_core() {

    struct sockaddr_in server_addr;

    debug_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SOCKET_PORT);
    inet_pton(AF_INET, SOCKET_ADDRESS, &server_addr.sin_addr);

    syslog(LOG_INFO, "attach framework core, connect to [%s] [%d]\n", SOCKET_ADDRESS, SOCKET_PORT);

    if(connect(debug_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        char strip[24];
        bzero(strip, sizeof(strip));
        inet_ntop( AF_INET, &server_addr.sin_addr, strip, sizeof(strip));
        syslog(LOG_INFO, "attache_core error, connect to [%s] [%d]\n", strip, ntohs(server_addr.sin_port));
        return -1;
    }

    return register_debug_module();
}


int deattach_core() {

    syslog(LOG_INFO, "de-attach framework core\n");
    close(debug_fd);

    return 0;

}

void *thrd_debug() {

    int ret;
    
    //cmd&event loop
    while (event_loop) {

        set_select_sets();
        ret = select(get_sets_max()+1, &fd_sets, NULL, NULL, NULL);
        if (ret < 0) {
            syslog(LOG_ERR, "select error: return = %d\n", ret);
            break;
        } else if (ret == 0) {
            syslog(LOG_ERR, "select timeout: return = %d\n", ret);
            continue;
        }
        
        clients_data_changed();
    }

    deattach_core();
    pthread_exit(NULL); 
}


int setup_event_loop() {
    
    int ret;
    pthread_t tid;

    ret = pthread_create(&tid, NULL, thrd_debug, NULL);
    if(ret != 0) {
        syslog(LOG_ERR, "pthread create error: return = %d\n", ret);
        return ret;
    }


    return ret;
}


