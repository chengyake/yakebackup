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




void debug_print_yuan(struct yuan_t *y) {

    int i;
    int t;
    if(y== NULL) {
        printf("print yuan parameter error\n");
        return;
    }
    if( (y->idx%SOL)%3 == 0) {
        printf("\tidx:\t%d\ttyp:\t%s\n", y->idx, "and");
    } else if ((y->idx%SOL)%3==1) {
        printf("\tidx:\t%d\ttyp:\t%s\n", y->idx, "or");
    } else {
        printf("\tidx:\t%d\ttyp:\t%s\n", y->idx, "not");
    }
    printf("\tlin:\t%d\trdx:\t%d\n", y->idx/SOL, y->idx%SOL);
    printf("\tin :\t%d\tout:\t%d\n", y->in, y->out);
    printf("\tid1:\t%d\tid2:\t%d\n\tsta1:", y->in1_idx, y->in2_idx);

    for(i=0; i<SOL; i++) {
        printf("%d ", y->sta1[i]);
    }
    printf("\n\tsta2:");

    for(i=0; i<SOL; i++) {
        printf("%d ", y->sta2[i]);
    }
    printf("\n");

}

void main(int argc, char *argv[]) {

    int fd, ret, line, idx;
    struct yuan_t y;

    if(argc != 3) {
        printf("Get 2 parameteres first please!\n");
        return ;
    }
    line = atoi(argv[1]);
    if(line <0 || line >= NOL) {
        printf("Get param idx error\n");
        return;
    }

    idx = atoi(argv[2]);
    if(idx <0 || idx >= SOL) {
        printf("Get param idx error\n");
        return;
    }

    fd = open(FILE_OF_MATRIX, O_RDONLY, S_IRUSR|S_IWUSR);
    if(fd < 0) {
        printf("open %s error\n", FILE_OF_MATRIX);
        return;
    }

    ret = lseek(fd, SOY*(line*SOL+idx), SEEK_SET);
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

    debug_print_yuan(&y);


    close(fd);
}



