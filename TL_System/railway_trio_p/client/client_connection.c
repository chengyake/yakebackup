#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <syslog.h>


#include "md5.h"
#include "../core/core.h"
#include "client_task_queue.h"
#include "remote_client_connection.h"
#include "local_client_connection.h"


static fd_set fd_sets;
static int event_loop=1;

int exit_client_event_loop() {
    event_loop = 0;
}

static int set_select_sets() {
    
    int ret = -1;
    FD_ZERO(&fd_sets);
    if(remote_fd > 0) {
    	ret = 0;
        FD_SET(remote_fd, &fd_sets);
    }
    if(local_fd > 0) {
    	ret = 0;
        FD_SET(local_fd, &fd_sets);
    }
    return ret;
}

static int get_sets_max() {
    
    int fd;
    fd = (remote_fd >= local_fd) ? remote_fd : local_fd;
    return fd;

}

int check_frame_fcs(struct frame_fmt *fhp) {
    int len;

    char *p = (char *)fhp;
    len = fhp->len - 17;

	return check_md5_fcs(p, len, p[len]);
}

int check_md5_fcs(char *data, int len, char *fcs) {

    int i, res=0;
    struct MD5Context md5c;
    unsigned char local_fcs[16];

    MD5Init(&md5c);
    MD5Update(&md5c, data, len);
    MD5Final(&local_fcs[0], &md5c);

    for( i=0; i<16; i++ ){
    	if(fcs[i] != local_fcs[i]) {
           res = -1;
           break;
        }
    }

    return res;
}



int make_md5_fcs(char *data, int len, char *fcs) {

    struct MD5Context md5c;
    unsigned char local_fcs[16];

    MD5Init(&md5c);
    MD5Update(&md5c, data, len);
    MD5Final(&local_fcs[0], &md5c);

    memcpy(fcs, &local_fcs[0], FRAME_MD5_SIZE);

    return 0;
}



/*debug API*/
void debug_print_header(struct frame_fmt *p) {

    int i;
    unsigned char pbf[SOCKET_BUFFER_SIZE]={0};
    //syslog(LOG_INFO,"header:");
    syslog(LOG_INFO,"head:0x%2X len:%d token:0x%08X", p->head, p->len, p->token);
    syslog(LOG_INFO,"task_type=%d task_state=%4d task_id:0x", p->task_type, p->task_state); 
    for(i=0; i<FRAME_TASK_ID_SIZE; i++) {
    	sprintf(&pbf[i*3], "%02X ", p->task_id[i]);
    }
    syslog(LOG_INFO, "%s", &pbf[0]);
}

/*debug API*/
void debug_print_data(struct frame_fmt *p) {
    int i;
    unsigned char pbf[SOCKET_BUFFER_SIZE*3]={0};

    syslog(LOG_INFO,"%ld bytes data:0x", p->len-FRAME_FMT_SIZE);
    for(i=0; i<(p->len-FRAME_FMT_SIZE); i++) {
    	sprintf(&pbf[i*2], "%02X", p->data[i]);
    }
    syslog(LOG_INFO, "%s", &pbf[0]);

}

/*debug API*/
void debug_print_tail(struct frame_fmt *p) {
    int i;
    int init_len;
    unsigned char *tp = (unsigned char *)p;
    unsigned char pbf[SOCKET_BUFFER_SIZE]={0};
    
    init_len = (p->len-FRAME_MD5_SIZE-1);
    syslog(LOG_INFO,"MD5:0x");
    for(i=init_len; i<p->len-1; i++){
    	sprintf(&pbf[(i-init_len)*2], "%02X ", tp[i]);
    }
    syslog(LOG_INFO, "%s", &pbf[0]);

    syslog(LOG_INFO,"end: 0x%2X", tp[i]);
}


/*debug API*/
void debug_print_frame(struct frame_fmt *p) {
#if 1
    syslog(LOG_INFO,"---------------------");
    debug_print_header(p);
    debug_print_data(p);
    debug_print_tail(p);
    syslog(LOG_INFO,"---------------------");
#endif
}

static int clients_data_changed() {
    int ret=0;
    if (remote_fd > 0 && FD_ISSET(remote_fd, &fd_sets)) {
        ret = recv_remote_server();
        if(ret < 0) {
            syslog(LOG_ERR, "recv remote server error %d; and retry to connect to server ", ret);
            return ret;
        }
    }
    if (local_fd > 0 && FD_ISSET(local_fd, &fd_sets)) {
        ret = recv_local_server();
        if(ret < 0) {
            syslog(LOG_ERR, "recv and process error %d", ret);
            return ret;
        }
    }
    return ret;
}



uint64_t get_minimal_interval() {

    uint64_t t1, t2;
    t2 = get_remote_interval(); 
    t1 = get_local_interval(); 

    return (t1<t2?t1:t2);
}

int process_task_queue() {
    
    int ret;
    ret = check_and_process_remote_task();
    if(ret < 0) {
        syslog(LOG_ERR, "check and process local task error: return = %d", ret);
        return ret;
    }
    ret = check_and_process_local_task();
    if(ret < 0) {
        syslog(LOG_ERR, "check and process local task error: return = %d", ret);
        return ret;
    }

    return 0;

}

int setup_event_loop() {

    int ret;
    struct timeval timer={0, 0};

    //cmd&event loop
    while (event_loop) {

        ret = set_select_sets();
        if(ret < 0) {
            syslog(LOG_ERR, "set select sets error: return = %d", ret);
            sleep(1);
        }

        process_task_queue();

        timer.tv_sec = get_minimal_interval();

        syslog(LOG_INFO, "<< timeout:%d >>",(int) timer.tv_sec);

        ret = select(get_sets_max()+1, &fd_sets, NULL, NULL, &timer);//too fast
        if (ret < 0) {
            syslog(LOG_ERR, "select error: return = %d", ret);
            sleep(1);
            continue;
        } else if (ret == 0) {
            syslog(LOG_INFO, "select timeout: return = %d", ret);
        }

        //syslog(LOG_INFO, "client data comming");
        clients_data_changed();//add del cancel stop task

    }

    return ret;
}





