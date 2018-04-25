#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <sys/un.h>
#include <fcntl.h>

#include "core.h"
#include "core_event.h"


fd_set fd_sets;
static int server_fd;
static struct sockaddr_un server_addr;
static int core_event_loop=1;
static int fd_clients[IDX_CLIENTS_MAX]={0}; 



/*debug API*/
void print_local_frame(struct cmd_header *p) {
#ifdef debug
    int i;
    unsigned char pbf[SOCKET_BUFFER_SIZE]={0};
    logdebug("+++++++++++++++++++++");
    logdebug("header:");
    logdebug("host:%d target:%d cmd:%d len:%d", p->host, p->target, p->cmd, p->len);
    for(i=0; i<p->len; i++) {
    	sprintf(&pbf[i*3], "%02X ", p->data[i]);
    }
    logdebug("%s", &pbf[0]);
    logdebug("+++++++++++++++++++++");
#endif
}



static int add_client_fd(int fd) {
	int i;

    fcntl(fd, F_SETFL, O_NONBLOCK);
	for(i=0; i<IDX_CLIENTS_MAX; i++) {
        if(fd_clients[i]==0) {
            fd_clients[i]=fd;
            return 0;
        }
    }
    return -1;
}
static int del_client_fd(int fd) {
	int i;
	for(i=0; i<IDX_CLIENTS_MAX; i++) {
        if(fd_clients[i]==fd) {
            fd_clients[i]=0;
            close(fd);
            return 0;
        }
    }
    return -1;
}
int free_clients_fd() {
	int i;
    for (i = 0; i < IDX_CLIENTS_MAX; i++) {
        if (fd_clients[i] != 0) {
            close(fd_clients[i]);
            fd_clients[i]=0;
        }
    }
}
static int get_clients_count() {
    int i, sum=0;
    for(i=0; i<IDX_CLIENTS_MAX; i++) {
        if(fd_clients[i] != 0) {
            sum++;
        }
    }
    return sum;
}
int set_select_sets() { 
    int i;
    FD_ZERO(&fd_sets);
    FD_SET(server_fd, &fd_sets);
    for(i=0; i<IDX_CLIENTS_MAX; i++) {
        if(fd_clients[i] != 0) {
            FD_SET(fd_clients[i], &fd_sets);
        }
    }
    return 0;
}
int get_clients_max() {
	int i;
	int fd=server_fd;
	for(i=0; i<IDX_CLIENTS_MAX; i++) {
        if(fd_clients[i] > fd) {
            fd = fd_clients[i];
        }
    }
    return fd;
}
int write_msg(int fd, char *buf, int len) {

	int ret;
    if(buf == NULL || fd <= 0) {
        logerr("write msg param error: *buf = NULL");
        return -1;
    }
	if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        logerr("write msg error: len = %d", len);
        return -1;
    }
    ret = write(fd, buf, len);
    if(ret < 0) {
        logerr("write msg error: %s", strerror(ret));
        return ret;
    }
    return ret;
}

int read_msg(int fd, char *buf, int len) {

	int ret;

    if(buf == NULL || fd <= 0) {
        logerr("read msg param error: *buf = NULL");
        return -1;
    }
    if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        logerr("read msg error: len = %d", len);
        return -1;
    }
    memset(buf, 0, len);
    ret = read(fd, buf, len);
    if(ret < 0) {
        logerr("read msg error: %s", strerror(ret));
    }

    return ret;
}

static int recv_and_process(int *fd) {
    
    int sum=0;
    int idx=0;
    static int len = 0;//last
    static struct cmd_header *chp=NULL;
    static unsigned char buffer[SOCKET_BUFFER_SIZE]={0};

    if(*fd == 0) {
        logerr("read core server error: fd is 0");
        return 0;
    }

    sum = recv(*fd, &buffer[len], SOCKET_BUFFER_SIZE, 0);
    if(sum <= 0) {
        logerr("read core server error1 %d", sum);
        close(*fd);
        *fd=0; 
        memset(buffer, 0, SOCKET_BUFFER_SIZE);
        len = 0;
    }

    sum+=len;

    while (1) {
        chp = (struct cmd_header *)&buffer[idx];
        if(sum < chp->len + CMD_HEADER_SIZE + idx) {
            break;//wait
        }
        core_event_process(*fd, chp);
        idx+=chp->len + CMD_HEADER_SIZE;
    }

    len = sum-idx;
    if(idx != 0) memmove(&buffer[0], &buffer[idx], len);
    memset(&buffer[sum-idx], 0, SOCKET_BUFFER_SIZE - len);

    return 0;

}

int clients_data_changed() {
    int i;
    int ret;

    for (i = 0; i < IDX_CLIENTS_MAX; i++) {
        if (FD_ISSET(fd_clients[i], &fd_sets)) {
            recv_and_process(&fd_clients[i]);
        }
    }
}

int clients_connection_changed() {
    if (FD_ISSET(server_fd, &fd_sets)) {
        int new_fd;
        socklen_t sin_size;
        struct sockaddr_in client_addr;

        sin_size = sizeof(server_addr);
        new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);
        if (new_fd <= 0) {
            logerr("accept error");
            return -1;
        }
        fcntl(new_fd, F_SETFL, O_NONBLOCK);

        if (get_clients_count() < IDX_CLIENTS_MAX) {
            add_client_fd(new_fd);
            loginfo("add client_fd: count = %d", get_clients_count());
        } else {
            logerr("too many connections attached, ignore this one: fd num = %d", get_clients_count());
            close(new_fd);
        }
    }
}

static void show_clients() {

    int i;
    logdebug("fd amount = %d", get_clients_count());
    for (i = 0; i < IDX_CLIENTS_MAX; i++) {
        logdebug("fd[%d] = %d", i, fd_clients[i]);
    }
}

int setup_core_server(void) {

    int i;
    int ret=-1;

    unlink(LOCAL_SOCKET);
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(server_fd < 0) {
        logerr("request socket error");
        return ret=-1;
    }

    //in case of re-setup; bind fail
    /*
    int yes = 1;
    ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(ret < 0) {
        logerr("set socketopt error");
        return ret;
    }*/
    
    server_addr.sun_family = AF_UNIX;
    strcpy (server_addr.sun_path, LOCAL_SOCKET);

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    ret = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret < 0) {
        logerr("bind error error");
        return ret;
    }

    ret = listen(server_fd, IDX_CLIENTS_MAX);
    if(ret < 0) {
        logerr("listen error");
        return ret;
    }

    return ret;
}


int setup_event_loop() {
    
    int ret;
    while (core_event_loop) {

        set_select_sets();
        ret = select(get_clients_max()+1, &fd_sets, NULL, NULL, NULL);
        if (ret < 0) {
            logerr("select error %d", ret);
            break;
        } else if (ret == 0) {
            logerr("select timeout err %d", ret);
            continue;
        }
        //printf("core event loop");
        clients_data_changed();
        clients_connection_changed();
    }

    free_clients_fd();
    unlink (LOCAL_SOCKET);

}


