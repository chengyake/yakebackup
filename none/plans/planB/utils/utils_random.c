#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "none_debug.h"


static int fd;

int32_t init_random() {

    fd = open ("/dev/urandom", O_RDONLY);
    if (fd <= 0) {
        logerr("open /dev/urandom error\n");
        return -1;
    }
    return 0;
}

int32_t get_random(int8_t *data, uint32_t num) {
    
    int32_t ret;
/*
    if(fd <= 0) {
        init_random();
    }
*/
    ret = read(fd, data, num);
    if(ret < 0) {
        logerr("get random num error when read /dev/urandom");
        return -1;
    }
    return ret;
}

void close_random () {
    close (fd);
    fd = 0;
}























