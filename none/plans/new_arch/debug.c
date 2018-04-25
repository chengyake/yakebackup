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


int get_em(struct yuan_t *y, int to_target) {
    
    int i;
    int sum=0;
    int avg=0;
    for(i=0; i<SLOT; i++) {
        sum+= y->sta[to_target][i];
    }
    avg = sum/SLOT;
    sum = 0;
    for(i=0; i<SLOT; i++) {
       sum += (avg - y->sta[to_target][i])*(avg - y->sta[to_target][i]);
    }
    
    return sum;
}

void debug_print_yuan(struct yuan_t *y, int to_target) {

    int i;
    if(y== NULL || to_target <= 0 && to_target >= NOA) {
        printf("print yuan parameter error\n");
        return;
    }

    printf("\tidx   :%d\n", y->idx);
    printf("\ttarget:%d\tdis    :%d\n", to_target, y->dis[to_target]);
    printf("\temv   :%d\tsta    :", get_em(y,to_target));

    for(i=0; i<SLOT; i++) {
        printf("%d ", y->sta[to_target][i]);
    }
    printf("\n");
}

void main(int argc, char *argv[]) {

    int fd, ret, idx, to;
    struct yuan_t y;

    if(argc != 3) {
        printf("Get parameter first please!\n");
        return ;
    }
    idx = atoi(argv[1]);
    if(idx <0 || idx >= NOM) {
        printf("Get param idx error\n");
        return;
    }

    to = atoi(argv[2]);
    if(to <=0 || to >= NOA) {
        printf("Get param target error\n");
        return;
    }
    printf("get yuan %d to %d\n", idx, to);


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

    ret = read(fd, &y, SOY);
    if(ret < 0) {
        printf("read %s error\n", FILE_OF_MATRIX);
        close(fd);
        exit(0);
    }

    debug_print_yuan(&y, to);


    close(fd);
}



