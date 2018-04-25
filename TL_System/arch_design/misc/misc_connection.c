#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

#include "../core/core.h"
#include "misc_event.h"
#include "misc_connection.h"



static int misc_fd;
static fd_set fd_sets;
int misc_loop = 1;

int set_select_sets() {

    FD_ZERO(&fd_sets);
    FD_SET(misc_fd, &fd_sets);
    return 0;
}

int get_sets_max() {

    return misc_fd;
}
/*
static int check_cmds_stream(char *buf) {
 

}
*/

static int recv_and_process() {

    int len;
    struct cmd_header *chp;
    char buffer[SOCKET_BUFFER_SIZE]={0};

    len = recv(misc_fd, &buffer[0], CMD_HEADER_SIZE, 0);
    if (len == 0) {
        syslog(LOG_ERR, "clients connection changed, recv return = %d\n", len);
    } else if(len == CMD_HEADER_SIZE) {
        chp = (struct cmd_header *)&buffer[0];
        if( chp->len > 0) {
            len = recv(misc_fd, &chp->data[0], chp->len, 0);
        }
        misc_event_process((struct cmd_header *)&buffer[0]);
    } else {
        syslog(LOG_ERR, "clients data changed, recv error: %s\n", strerror(len));
    }

    return 0;
}

int clients_data_changed() {

    if (FD_ISSET(misc_fd, &fd_sets)) {
        return recv_and_process();
    }

    return -1;

}

int misc_write_msg(char *buf, int len) {

	int ret;

    if(buf == NULL) {
        syslog(LOG_ERR, "write msg param error: *buf = NULL\n");
        return -1;
    }

	if(len <=0 || len > SOCKET_BUFFER_SIZE) {
        syslog(LOG_ERR, "write msg error: len = %d\n", len);
        return -1;
    }
    ret = write(misc_fd, buf, len);
    if(ret < 0) {
        syslog(LOG_ERR, "write msg error: %s\n", strerror(ret));
        return ret;
    }

    return ret;
}


int misc_read_msg(char *buf, int len) {

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
    ret = read(misc_fd, buf, len);
    if(ret < 0) {
        syslog(LOG_ERR, "read msg error: %s\n", strerror(ret));
    }

    return ret;
}


int attach_core() {

    struct sockaddr_in server_addr;

    misc_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SOCKET_PORT);
    inet_pton(AF_INET, SOCKET_ADDRESS, &server_addr.sin_addr);

    syslog(LOG_INFO, "attach framework core, connect to [%s] [%d]\n", SOCKET_ADDRESS, SOCKET_PORT);

    if(connect(misc_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        char strip[24];
        bzero(strip, sizeof(strip));
        inet_ntop( AF_INET, &server_addr.sin_addr, strip, sizeof(strip));
        syslog(LOG_INFO, "attache_core error, connect to [%s] [%d]\n", strip, ntohs(server_addr.sin_port));
        return -1;
    }

    return register_misc_module();
}


int deattach_core() {

    syslog(LOG_INFO, "de-attach framework core\n");
    close(misc_fd);

    return 0;

}

int setup_event_loop() {

	int ret;
    while (misc_loop) {
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

    return 0;

}
