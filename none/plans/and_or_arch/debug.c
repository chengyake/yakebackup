#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#include "none_core.h"


int get_em(unsigned short *a) {
    
    int i;
    unsigned short b[NOA-NOI];
    unsigned long sum=0;
    int avg=0;
    unsigned short max=0;

    memcpy(b, a, sizeof(b));
    for(i=0; i<NOA-NOO; i++) {
        max = max>=b[i] ? max : b[i];
    }
    for(i=0; i<NOA-NOO; i++) {
        b[i]=b[i]*0xFFFF/max;
    }

    for(i=0; i<NOA-NOO; i++) {
        sum+= b[i];
    }
    avg = sum/(NOA-NOO);
    sum = 0;
    for(i=0; i<NOA-NOO; i++) {
       sum += (avg - b[i])*(avg - b[i]);
    }
    
    return sum/10000000;
}

void debug_print_yuan(struct yuan_t *y, int to_target) {

    int i;
    int t;
    if(y== NULL || to_target <= 0 && to_target >= NOA) {
        printf("print yuan parameter error\n");
        return;
    }

    printf("\tidx:\t%d\tout:\t%d\ttype:\t%d\n", y->idx, y->out, (y->idx-NOI)%3);
    printf("\tid1:\t%d\tid2:\t%d\n", y->in1_idx, y->in2_idx);
    printf("\tem1:\t%d\tem2:\t%d\n\tsta1:", get_em(y->sta1), get_em(y->sta2));

    for(i=0; i<NOA-NOO; i++) {
        printf("%d ", y->sta1[i]);
    }
    printf("\n\tsta2:");

    for(i=0; i<NOA-NOO; i++) {
        printf("%d ", y->sta2[i]);
    }
    printf("\n");
}

void main(int argc, char *argv[]) {

    int fd, ret, idx;
    struct yuan_t y;

    if(argc != 2) {
        printf("Get 2 parameteres first please!\n");
        return ;
    }
    idx = atoi(argv[1]);
    if(idx <0 || idx >= NOA) {
        printf("Get param idx error\n");
        return;
    }

    fd = open(FILE_OF_MATRIX, O_RDONLY, S_IRUSR|S_IWUSR);
    if(fd < 0) {
        printf("open %s error\n", FILE_OF_MATRIX);
        return;
    }

    ret = lseek(fd, SOY*idx, SEEK_SET);
    if(ret < 0) {
        printf("lseek %s fixed space error\n", FILE_OF_MATRIX);
        close(fd);
        exit(0);
    }

    ret = read(fd, (char*)&y, SOY);
    if(ret < 0) {
        printf("read %s error\n", FILE_OF_MATRIX);
        close(fd);
        exit(0);
    }

    debug_print_yuan(&y, idx);


    close(fd);
}



