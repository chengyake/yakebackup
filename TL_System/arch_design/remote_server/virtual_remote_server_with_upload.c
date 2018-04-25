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
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <stdint.h>


fd_set fd_sets;
static int server_fd;
static int new_fd;
static struct sockaddr_in server_addr;

static unsigned char token;
static unsigned char task_id;

int loop = 1;

static unsigned char time1[9]={0};
static unsigned char time2[9]={0};
static int64_t time_name;
static unsigned int file_id=1;

int get_random_bytes() {

}


int write_msg(int fd, char *buf, int len) {

	int ret;
    if(buf == NULL || fd <= 0) {
        syslog(LOG_ERR, "write msg param error: *buf = NULL\n");
        return -1;
    }
	if(len <=0 || len > 1600) {
        syslog(LOG_ERR, "write msg error: len = %d\n", len);
        return -1;
    }
    ret = write(fd, buf, len);
    if(ret < 0) {
        syslog(LOG_ERR, "write msg error: %s\n", strerror(ret));
        return ret;
    }
    return ret;
}

int read_msg(int fd, char *buf, int len) {

	int ret;

    if(buf == NULL || fd <= 0) {
        syslog(LOG_ERR, "read msg param error: *buf = NULL\n");
        return -1;
    }
    if(len <=0 || len > 1600) {
        syslog(LOG_ERR, "read msg error: len = %d\n", len);
        return -1;
    }
    memset(buf, 0, len);
    ret = read(fd, buf, len);
    if(ret < 0) {
        syslog(LOG_ERR, "read msg error: %s\n", strerror(ret));
    }

    return ret;
}


int setup_core_server(void) {

    int i;
    int ret=-1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        syslog(LOG_ERR, "request socket error\n");
        return ret=-1;
    }

    //in case of re-setup; bind fail
    int yes = 1;
    ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(ret < 0) {
        syslog(LOG_ERR, "set socketopt error\n");
        return ret;
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(55555);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    ret = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret < 0) {
        syslog(LOG_ERR, "bind error error\n");
        return ret;
    }

    ret = listen(server_fd, 2);
    if(ret < 0) {
        syslog(LOG_ERR, "listen error\n");
        return ret;
    }

    return ret;
}

static int test_fd;
int open_test_file() {
    test_fd = open("recv_file.gsmr", O_RDWR|O_CREAT);
    if(test_fd < 0) {
        syslog(LOG_ERR, "open_recv_file error: %d\n", test_fd);
         exit(-1);
    }

    return 0;

}


int print_all() {

    int i, len=0;
	unsigned char buf[1600]={0};

    len = read_msg(new_fd, &buf[0], 1600);
    
    syslog(LOG_INFO, "get data %d\n", len);
    if(len <10) return -1;


    //agree
    if(buf[8] == 0x2c && buf[9] ==0x01 ) {
        upload_agree();
    }

    //goon
    if(buf[8] == 0x2f && buf[9] ==0x01 ) {
        write(test_fd, &buf[71], *(unsigned short *)&buf[32]);
        upload_goon();
    }

    //over
    if(buf[8] == 0x30 &&  buf[9] ==0x01 ) {
        write(test_fd, &buf[71], *(unsigned short*)&buf[32]);
        upload_success();
        close(test_fd);
        test_fd = 0;
    }

    
    return 0;
}

int request_register() {
    syslog(LOG_INFO, "send register request\n");
    unsigned char sb1[]={0xFE/*head*/, 43, 0x00/*len*/, token++, 0x00, 0x00, 0x00, 0x01/*type*/, 103, 0x00/*state*/,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/*id*/, 
        0x4C, 0xF2, 0xCE, 0xD5, 0xF4, 0xA0, 0xBC, 0xD2, 0x39, 0x96, 0x55, 0x1B, 0x8A, 0x37, 0x9F, 0x46/*md5*/,
        0xFF};
    write_msg(new_fd, sb1, 43);

}

int register_sucess() {
    syslog(LOG_INFO, "register success\n");
    unsigned char sb2[]={0xFE/*head*/, 43, 0x00/*len*/, token++, 0x00, 0x00, 0x00, 0x01/*type*/, 102, 0x00/*state*/,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/*id*/, 
        0x69, 0xDB, 0xD1, 0x0B, 0x70, 0xD5, 0xBE, 0xAB, 0x53, 0x32, 0xC2, 0x61, 0x32, 0x3B, 0x25, 0x82,
        0xFF};
    write_msg(new_fd, sb2, 43);
}


//---------------

int unix_to_datetime(unsigned char *data, time_t utime) {
    unsigned long long ret;
    ret = utime*10000000 + 621355968000000000LL;
    data[0] =       ret & 0xffLL;
    data[1] =  (ret>>8) & 0xffLL;
    data[2] = (ret>>16) & 0xffLL;
    data[3] = (ret>>24) & 0xffLL;
    data[4] = (ret>>32) & 0xffLL;
    data[5] = (ret>>40) & 0xffLL;
    data[6] = (ret>>48) & 0xffLL;
    data[7] = (ret>>56) & 0xffLL;
}


int send_task() {

    memset(time1, 0, sizeof(time1));
    memset(time2, 0, sizeof(time2));
    time_name = time(NULL);
    unix_to_datetime(&time1[0], time_name+20);
    unix_to_datetime(&time2[0], time_name+60*3+20);
    file_id = 1;

    open_test_file();

    syslog(LOG_INFO, "send task\n");
    unsigned char sb3[]={0xFE, 155+9, 0x00/*len*/, token++, 0x00, 0x00, 0x00/*token*/, 0x02/*type*/, 200, 0x00/*state*/,
        0x11, ++task_id, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/*id*/, 

        //112data
        0x29, 0x00, 0x14, 0x00, 0x00, 0x00, 0xDC, 0x00, 0x00, 0x00, 
        /*0x00, 0xD2, 0xD5, 0x3E, 0x23, 0x93, 0xD2, 0x88, 
        0x00, 0x30, 0xA6, 0xF1, 0x23, 0x93, 0xD2, 0x88,*/
        time1[0], time1[1], time1[2], time1[3],time1[4], time1[5],time1[6], time1[7],
        time2[0], time2[1], time2[2], time2[3],time2[4], time2[5],time2[6], time2[7],
        0x01, 
        0x00, 0x00, 0x00, 0x00, 

        0x09, 
        0xE6, 0x98, 0x8C, 0xE7, 0xA6, 0x8F, 0xE7, 0xBA, 0xBF, 

        0x47, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,

        0x09, 
        0xE6, 0x98, 0x8C, 0xE7, 0xA6, 0x8F, 0xE7, 0xBA, 0xBF, 
        
        0x00, 0x00, 0x01, 0x00,
        0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05,
        0x00, 0x06, 0x00, 0x07, 0x01, 0x21, 0x00,
        0x02, 
        0x0b, 
        //0x30+1, 0x30+8, 0x30+5, 0x30+1, 0x30+5, 0x30+0, 0x30+6, 0x30+1, 0x30+5, 0x30+7, 0x30+4,
        0x30+1, 0x30+5, 0x30+8, 0x30+1, 0x30+0, 0x30+3, 0x30+5, 0x30+2, 0x30+1, 0x30+7, 0x30+4,

        
        
        //dial timeout
        0x30, 0x00, 0x00, 0x00, 
        0x30, 0x00, 0x00, 0x00, 
        0x30, 0x00, 0x00, 0x00, 
        
        
        0x00, 0x08, 0x00, 0x09, 0x00,
        0x0A, 0x00, 0x0B, 0x00, 0x0C, 0x00, 0x0D,
        0x00, 0x0E, 0x00, 0x0F, 0x00, 0x10, 0x00,



        0x69, 0xDB, 0xD1, 0x0B, 0x70, 0xD5, 0xBE, 0xAB, 0x53, 0x32, 0xC2, 0x61, 0x32, 0x3B, 0x25, 0x82,
        0xFF};
    write_msg(new_fd, sb3, sizeof(sb3));
}

int start_task() {
    syslog(LOG_INFO, "start task\n");
    unsigned char sb4[]={0xFE/*head*/, 43, 0x00/*len*/, token++, 0x00, 0x00, 0x00/*token*/, 0x02/*type*/, 202, 0x00/*state*/,
        0x11, task_id, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/*id*/, 
        0x69, 0xDB, 0xD1, 0x0B, 0x70, 0xD5, 0xBE, 0xAB, 0x53, 0x32, 0xC2, 0x61, 0x32, 0x3B, 0x25, 0x82,
        0xFF};
    write_msg(new_fd, sb4, sizeof(sb4));
}


int cancel_task() {
    syslog(LOG_INFO, "cancle task\n");
    unsigned char sb4[]={0xFE/*head*/, 43, 0x00/*len*/, token++, 0x00, 0x00, 0x00/*token*/, 0x02/*type*/, 208, 0x00/*state*/,
        0x11, task_id, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/*id*/, 
        0x69, 0xDB, 0xD1, 0x0B, 0x70, 0xD5, 0xBE, 0xAB, 0x53, 0x32, 0xC2, 0x61, 0x32, 0x3B, 0x25, 0x82,
        0xFF};
    write_msg(new_fd, sb4, sizeof(sb4));
}

int stop_task() {
    syslog(LOG_INFO, "stop task\n");
    unsigned char sb4[]={0xFE/*head*/, 43, 0x00/*len*/, token++, 0x00, 0x00, 0x00/*token*/, 0x02/*type*/, 205, 0x00/*state*/,
        0x11, task_id, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/*id*/, 
        0x69, 0xDB, 0xD1, 0x0B, 0x70, 0xD5, 0xBE, 0xAB, 0x53, 0x32, 0xC2, 0x61, 0x32, 0x3B, 0x25, 0x82,
        0xFF};
    write_msg(new_fd, sb4, sizeof(sb4));
}


int upload_agree() {
    syslog(LOG_INFO, "upload agree\n");

    int64_t tmp;
    struct tm *p;
    p=localtime((time_t *)&time_name);

    unsigned char sb4[]={0xFE/*head*/, 43+35+8, 0x00/*len*/, token++, 0x00, 0x00, 0x00/*token*/, 0x03/*type*/, 0x2D, 0x01/*state*/,
        0x11, task_id, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/*id*/, 

        //file id 4
        (file_id&0xff), (file_id>>8&0xff), (file_id>>16&0xff), (file_id>>24&0xff), 
        //file len 2  yasuo 1  filename len 1
        0x00, 0x00, 0x00, 35,
        //file name[]

        0xE6, 0x98, 0x8C, 0xE7, 0xA6, 0x8F, 0xE7, 0xBA, 0xBF, 
        0x5F,
        0xe4, 0xb8, 0x8a, 0xe8, 0xa1, 0x8c,

        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,

        0x69, 0xDB, 0xD1, 0x0B, 0x70, 0xD5, 0xBE, 0xAB, 0x53, 0x32, 0xC2, 0x61, 0x32, 0x3B, 0x25, 0x82,
        0xFF};

    sprintf(&sb4[50], "_%4d%02d%02d_%02d%02d.gsmr",
            (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min);
    file_id++;
    write_msg(new_fd, sb4, sizeof(sb4));
}

int upload_goon() {
    syslog(LOG_INFO, "upload goon\n");

    int64_t tmp;
    struct tm *p;
    p=localtime((time_t *)&time_name);

    unsigned char sb4[]={0xFE/*head*/, 43+35+8, 0x00/*len*/, token++, 0x00, 0x00, 0x00/*token*/, 0x03/*type*/, 0x31, 0x01/*state*/,
        0x11, task_id, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/*id*/, 

        //file id 4
        (file_id&0xff), (file_id>>8&0xff), (file_id>>16&0xff), (file_id>>24&0xff), 
        //file len 2  yasuo 1  filename len 1
        0x00, 0x00, 0x00, 35,
        //file name[]

        0xE6, 0x98, 0x8C, 0xE7, 0xA6, 0x8F, 0xE7, 0xBA, 0xBF, 
        0x5F,
        0xe4, 0xb8, 0x8a, 0xe8, 0xa1, 0x8c,

        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,

        0x69, 0xDB, 0xD1, 0x0B, 0x70, 0xD5, 0xBE, 0xAB, 0x53, 0x32, 0xC2, 0x61, 0x32, 0x3B, 0x25, 0x82,
        0xFF};

    sprintf(&sb4[50], "_%4d%02d%02d_%02d%02d.gsmr",
            (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min);
    file_id++;
    write_msg(new_fd, sb4, sizeof(sb4));
}



int upload_success() {

    syslog(LOG_INFO, "upload success\n");
    unsigned char sb4[]={0xFE/*head*/, 43, 0x00/*len*/, token++, 0x00, 0x00, 0x00/*token*/, 0x03/*type*/, 0x37, 0x01/*state*/,
        0x11, task_id, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00/*id*/, 
        0x69, 0xDB, 0xD1, 0x0B, 0x70, 0xD5, 0xBE, 0xAB, 0x53, 0x32, 0xC2, 0x61, 0x32, 0x3B, 0x25, 0x82,
        0xFF};
    write_msg(new_fd, sb4, sizeof(sb4));
}


void read_server() {
    int ret;
    static fd_set fd_sets;
    while(loop) {

        FD_ZERO(&fd_sets);
        FD_SET(new_fd, &fd_sets);


        ret = select(new_fd+1, &fd_sets, NULL, NULL, NULL); 
        if(ret < 0) {
            syslog(LOG_INFO, "return select\n");
        }

        print_all();
    }
}

int start_read_server() {
    int ret;
	pthread_t pid;

	ret=pthread_create(&pid, NULL, (void *)read_server, NULL);
	if(ret != 0) {
        syslog(LOG_INFO, "pthread create error: %d\n", ret);
    }

    return ret;
}





int main() {
    
    openlog("yakedebug",LOG_PID, LOG_USER);
    //openlog("yakedebug", LOG_PID|LOG_PERROR, LOG_USER);

    syslog(LOG_INFO, "step 1: setup remote server\n");
    setup_core_server();

    syslog(LOG_INFO, "step 2: wait local client attach\n");
    FD_ZERO(&fd_sets);
    FD_SET(server_fd, &fd_sets);
    select(server_fd+1, &fd_sets, NULL, NULL, NULL);

    syslog(LOG_INFO, "step 3: add local client fd\n");
    socklen_t sin_size;
    struct sockaddr_in client_addr;
    sin_size = sizeof(server_addr);

    new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);
    if(new_fd <= 0) {
        printf("xxxxxxxxxxxx\n");
    }

    //start read pthread
    start_read_server();


    syslog(LOG_INFO, "step 4: into cmd loop, input cmd please\n");

    while(loop) {

        int cmd=getchar();
        switch(cmd) {
            case '0':
                request_register();
                break;
            case '1':
                register_sucess();
                break;
            case '2':
                send_task();
                break;
            case '3':
                start_task();
                break;
            case '4':
                cancel_task();
                break;
            case '5':
                stop_task();
                break;
            case '6':
                //upload_agree();
                break;
            case '7':
                //upload_goon();
                break;
            case '8':
                //upload_success();
                break;
            case '9':
                break;
            case 'e':
                loop = 0;
                break;
            default:
                continue;
                break;

        }
        
        syslog(LOG_INFO, "cmd = %d\n", cmd);
    }

    syslog(LOG_INFO, "press any to exit\n");

    close(new_fd);
    close(server_fd);
    return 0;

}
