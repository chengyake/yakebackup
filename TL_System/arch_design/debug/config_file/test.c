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

#define MAX_SIZE_OF_FILE    512
#define CONFIG_FILE     "./config.txt"



int get_train_name(char *uname) {
    
    int fd;
    int len, i; 
    unsigned char name[MAX_SIZE_OF_FILE]={0};
    unsigned char data[MAX_SIZE_OF_FILE]={0};

    //add 
    fd = open(CONFIG_FILE, O_RDONLY);
    if(fd < 0) {
        printf("register request: open config file error %d", fd);
        return 0;
    }
    
    len = read(fd, &data[0], MAX_SIZE_OF_FILE);
    if(len <= 0) {
        printf("register request: read config file error %d", fd);
        return 0;
    }

    len = get_config(&data[0], len, "train_name", 10, &name[0]);


    uname[0] = 0xFF;
    uname[1] = 0xFE;
    for(i=0; i<len; i++) {
        uname[i*2+2] = name[i];
    }
    close(fd);
    return i*2+2;
}



int get_server_ip(char *ip) {
    
    int fd;
    int len, i; 
    unsigned char data[MAX_SIZE_OF_FILE]={0};
    //add 
    fd = open(CONFIG_FILE, O_RDONLY);
    if(fd < 0) {
        printf("register request: open config file error %d", fd);
        return 0;
    }
    
    len = read(fd, &data[0], MAX_SIZE_OF_FILE);
    if(len <= 0) {
        printf("register request: read config file error %d", fd);
        return 0;
    }

    len = get_config(&data[0], len, "server_ip", 9, &ip[0]);

    close(fd);
    return 0;

}

int get_server_port() {
    
    int fd;
    int len, i; 
    unsigned char port[16]={0};
    unsigned char data[MAX_SIZE_OF_FILE]={0};

    fd = open(CONFIG_FILE, O_RDONLY);
    if(fd < 0) {
        printf("register request: open config file error %d", fd);
        return 0;
    }
    
    len = read(fd, &data[0], MAX_SIZE_OF_FILE);
    if(len <= 0) {
        printf("register request: read config file error %d", fd);
        return 0;
    }

    len = get_config(&data[0], len, "server_port", 11, &port[0]);

    close(fd);
    return atoi(port);

}


int compare_string(char *org, char *dst, int len) {
    int i;
    for(i=0; i<len; i++) {
        if(org[i] != dst[i]) {
            return -1;
        }
    }
    return 0;
}

/*
 *  get value after ':' by key string
 *  return len of value string
 */
int get_config(char * data, int d_len,  char *key, int k_len, char *target) {

    int i,j;
    int len;
    char tmp[MAX_SIZE_OF_FILE]={0};
    //del ' '
    len=0;
    for(i=0; i<d_len; i++) {
        if(data[i] != 0x20 && data[i] != 0x09) {
            tmp[len++]=data[i]; 
        }
    }
    for(i=0; i<d_len-k_len; i++) {
        if(tmp[i]==key[0]) {
            if(compare_string(&tmp[i], key, k_len) >= 0) {
                for(j=0; j<len; j++) {
                    if(tmp[i+k_len+1]==0x0A) {
                        return j;
                    }
                    target[j] = tmp[i+k_len+1];
                    i++;
                }
            }
        }
    }

    return -1;
}


int main() {
    int i,len;

    unsigned char buff[512]={0};
    len = get_train_name(buff);
    for(i=0; i<len; i++) {
        printf("0x%X ", buff[i]);
    }
    printf("\n");
    
    memset(buff, 0, 512);
    get_server_ip(buff);
    printf("%s\n", buff);

    printf("%d\n", get_server_port());
    
    return 0;
}
