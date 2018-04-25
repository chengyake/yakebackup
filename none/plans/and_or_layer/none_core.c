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
#include <omp.h> 

#include "none_core.h"


//#define NON_TRAIN
#define TEST    (1)

struct timeval t_start,t_end;
long cost_time = 0;



static unsigned int ticks=0;

struct yuan_t *soa=NULL;
static unsigned char idata[NOI];

static int debug_c=0, debug_w=0;

int fd, rd;


int init_random() {
    rd = open ("/dev/urandom", O_RDONLY);
    if (rd <= 0) {
        printf("open /dev/urandom error\n");
        return -1;
    }
    return 0;
}
#define RANDOM
int get_random(unsigned char *dst, unsigned int num) {
#ifdef RANDOM
    int ret;
    ret = read(rd, dst, num);
    if(ret < 0) {
        printf("get random num error when read /dev/urandom");
        return -1;
    }
    return ret;
#else
    int i, *p;
    int seed[2], ret;
    ret = read(rd, &seed, 8);
    srand(seed[0]);
    p = (int *)dst;
    for(i=seed[1]%num; i<num; i+=4) {
        *p = rand();
        p++;
    }
    for(i=0; i<seed[1]%num; i+=4) {
        *p = rand();
        p++;
    }

    return ret;
#endif
}

void close_random () {
    close (rd);
    rd = 0;
}

void init_none() {

    int ret, i, j;

    if(access(FILE_OF_MATRIX, F_OK|R_OK|W_OK) != 0) { //don't exist
        struct yuan_t data;
        fd = open(FILE_OF_MATRIX, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            printf("create %s error", FILE_OF_MATRIX);
            exit(0);
        }
        memset(&data, 0, sizeof(data));
        data.in=0xFF;
        data.out=0xFF;
        memset(data.sta1, 0x55, sizeof(data.sta1));
        memset(data.sta2, 0x55, sizeof(data.sta2));
        for(i=0; i<NOL*SOL; i++) {
            data.idx = i;
            ret = write(fd, &data, SOY);
            if(ret < 0) {
                printf("write TDataBase.db error");
                exit(0);
            }
        }
    } else {
        fd = open(FILE_OF_MATRIX, O_RDWR, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            printf("open %s error", FILE_OF_MATRIX);
            exit(0);
        }
    }

    ret = lseek(fd, 0, SEEK_SET);
    if(ret < 0) {
        printf("create %s fixed space error", FILE_OF_MATRIX);
        exit(0);
    }

    soa = (struct yuan_t *) mmap(0, SOY*NOL*SOL, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(soa == MAP_FAILED) {
        printf("mmap %s error: %d\n", FILE_OF_MATRIX,  errno);
        exit(0);
    }

    //printf("init none success\n");
    return;
}

unsigned long int rt1[NOL*SOL];
unsigned long int rt2[NOL*SOL];
void get_input_data() {
    int i;
    for(i=0; i<NOI; i++) {
        idata[i] = (rt1[i]&0x08)==0 ? 1 : 0;
    }
}

void update_idx() {
    int r1, r2;
    int i, j;

    //hide layer && output layer
    for(i=SOL; i<NOL*SOL; i++) {
        unsigned int sum1=0, sum2=0;
        for(j=0; j<SOL; j++) {
            sum1+=soa[i].sta1[j];
            sum2+=soa[i].sta2[j];
        }
        r1=rt1[i]%sum1;
        r2=rt2[i]%sum2;

        for(j=0; j<SOL; j++) {
            if(r1 > soa[i].sta1[j]) {
                r1 = r1 - soa[i].sta1[j];
            } else {
                soa[i].in1_idx=j;
                break;
            }
        }

        for(j=0; j<SOL; j++) {
            if(r2 > soa[i].sta2[j]) {
                r2 = r2 - soa[i].sta2[j];
            } else {
                soa[i].in2_idx=j;
                break;
            }
        }
    }

}


void input_none() {
    int i;
    for(i=0; i<NOI; i++) {
        soa[i].out= idata[i];
    }
    for(i=NOI; i<SOL; i++) {
        soa[i].out= i%2==1 ? 1 : 0;
    }


}
void feedback(int f) {
    int i,j;
    unsigned short max=0;
    
    if(f==0) {return;}

    for(i=SOL; i<NOL*SOL; i++) {
        if(soa[i].sta1[soa[i].in1_idx] + f >= 1) {
            soa[i].sta1[soa[i].in1_idx] += f;
        } else {
            soa[i].sta1[soa[i].in1_idx] = 1;
        }
        max=0;
        for(j=0; j<SOL; j++) {
            max = max>=soa[i].sta1[j] ? max : soa[i].sta1[j];
        }

        if(max > 65435) {
            for(j=0; j<SOL; j++) {
                soa[i].sta1[j] = (soa[i].sta1[j]+1)/2;
            }
        }
        if(max < 10240) {
            for(j=0; j<SOL; j++) {
                soa[i].sta1[j] = soa[i].sta1[j]*2;
            }
        }
    }

    for(i=SOL; i<(NOL-1)*SOL; i++) {
        if(soa[i].sta2[soa[i].in2_idx] + f >= 1) {
            soa[i].sta2[soa[i].in2_idx] += f;
        } else {
            soa[i].sta2[soa[i].in2_idx] = 1;
        }
        max=0;
        for(j=0; j<SOL; j++) {
            max = max>=soa[i].sta2[j] ? max : soa[i].sta2[j];
        }

        if(max > 65435) {
            for(j=0; j<SOL; j++) {
                soa[i].sta2[j] = (soa[i].sta2[j]+1)/2;
            }
        }
        if(max < 10240) {
            for(j=0; j<SOL; j++) {
                soa[i].sta2[j] = soa[i].sta2[j]*2;
            }
        }
    }


}

void in_to_out() {

    int i;
    //clear input
    for(i=0; i<SOL; i++) {
        soa[i].out = 0xFF;
    }
    for(i=SOL; i<NOL*SOL; i++) {
        soa[i].out = soa[i].in;
    }


}


void idx_to_in(int line) {

    int i=line, j;
    unsigned char in1, in2;

    //for(i=1; i<NOL-1; i++) {
    if(line < NOL-1) {
        for(j=0; j<SOL; j++) {
            int idx=i*SOL+j;
            int pre_idx1=(i-1)*SOL+soa[idx].in1_idx;
            int pre_idx2=(i-1)*SOL+soa[idx].in2_idx;

            if(soa[pre_idx1].out != 0xFF || soa[pre_idx2].out != 0xFF) {
                in1 = (soa[pre_idx1].out==0xFF) ? 0 : soa[pre_idx2].out;
                in2 = (soa[pre_idx2].out==0xFF) ? 0 : soa[pre_idx2].out;
                if(j%3==0) {                                  //and
                    soa[idx].in = in1&&in2;
                } else if (j%3 == 1) {                        //or
                    soa[idx].in = in1||in2;
                } else {                                      //not
                    soa[idx].in = (soa[pre_idx1].out==0xFF) ? 0xFF : (in1==0?1:0);
                }
            } else {
                soa[idx].in = 0xFF;
            }
        }
    }

    //for(i=NOL-1; i<NOL; i++) {
    if(line == (NOL-1)) {
        for(j=0; j<SOL; j++) {
            int idx=i*SOL+j;
            int pre_idx1=(i-1)*SOL+soa[idx].in1_idx;
            soa[idx].in  = soa[pre_idx1].out;
        }
    }
}


void output_none() {
    int i;
    int id;
    unsigned char success=1;

    if(ticks%1000==999) {
        printf("percent right:%d wrong:%d\n", debug_c, debug_w);
        debug_c=0;
        debug_w=0;
    }

    if(idata[0]==1 && idata[1]==1 && soa[(NOL-1)*SOL+0].out==1) { 
        success = 1;
    } else {
        success = 0;
    }

    if(success == 1) {
            feedback(40);
            debug_c++;
    } else {
            feedback(-40);
            debug_w++;
    }

}

void clear_env() {
    int i;
    for(i=0; i<NOL*SOL; i++) {
        soa[i].in =0xFF;
        soa[i].out=0xFF;
    }
}

void run() {

    int i;
    //clear in & out
    clear_env();
    get_random((unsigned char*)&rt1[0], sizeof(rt1));
    get_random((unsigned char*)&rt2[0], sizeof(rt2));
    get_input_data();
    input_none();
    update_idx();
    for(i=1; i<NOL; i++) {
        idx_to_in(i);
        in_to_out();
    }
    output_none();

    ticks++;
}

void close_none() {
    if(soa!=NULL) {
        msync((void *)soa, SOY*NOL*SOL, MS_ASYNC);
        munmap((void *)soa, SOY*NOL*SOL);
    }
    close(fd);
}

void main(int argc, char *argv[]) {
    int counter, times;
    if(argc != 2) {
        printf("Get parameter first please!\n");
        return ;
    }
    times = atoi(argv[1]);
    if(times <=0) {
        printf("Get param error\n");
        return;
    }

    init_random();
    init_none();

    for(counter=0; counter<times; counter++) {
        run();
    }

    close_none();
    close_random();

    printf("run :%d times over\n", ticks);
}







 




