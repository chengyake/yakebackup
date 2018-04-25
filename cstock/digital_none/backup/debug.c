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

#include "../none_core.h"




int get_em(struct cell_t *y, int to_target) {
    
    int i;
    int sum=0;
    int avg=0;
    int max=0;
    unsigned int tmp[41];
    for(i=0; i<41; i++) {
        max=max>y->sta[to_target][i] ? max :y->sta[to_target][i];
    }
    for(i=0; i<41; i++) {
        tmp[i] = y->sta[to_target][i]*256/max;
        sum+= tmp[i];
    }
    avg = sum/41;
    sum = 0;
    for(i=0; i<41; i++) {
       sum += (avg - tmp[i])*(avg - tmp[i]);
    }
    
    return sum;
}

void debug_print_cell(struct cell_t *y, int to_target) {

    int i;
    if(y== NULL || to_target <= 0 && to_target >= NOA) {
        printf("print yuan parameter error\n");
        return;
    }

    printf("\tidx   :%d\n", y->idx);
    printf("\ttarget:%d\tdis    :%d\n", to_target, y->dis[to_target]);
    printf("\temv   :%d\n\tsta   :", get_em(y,to_target));

    for(i=0; i<41; i++) {
        printf("%d ", y->sta[to_target][i]);
    }
    printf("\n");
}

void main(int argc, char *argv[]) {

    int fd, ret, idx, to;
    struct cell_t y;

    if(argc != 3) {
        printf("Get parameter first please!\n");
        return ;
    }
    idx = atoi(argv[1]);
    if(idx <0 || idx >= NOA) {
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

    ret = lseek(fd, SOC*idx, SEEK_SET);
    if(ret < 0) {
        printf("lseek %s fixed space error\n", FILE_OF_MATRIX);
        close(fd);
        exit(0);
    }

    ret = read(fd, &y, SOC);
    if(ret < 0) {
        printf("read %s error\n", FILE_OF_MATRIX);
        close(fd);
        exit(0);
    }

    debug_print_cell(&y, to);


    close(fd);
}



