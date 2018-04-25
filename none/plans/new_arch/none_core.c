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
struct yuan_t *som[NOM]={NULL};
static int sum_p[NOA];
static int sum_n[NOA];
static int idata[NOI];

static int debug_c=0, debug_t=0;

int fd, rd;

#if(SLOT==15)
int slot_table[SLOT] = {-128, -64, -32, -16, -8, -4, -2, 0, 2, 4, 8, 16, 32, 64, 128};
#elif(SLOT==21)
int slot_table[SLOT] = {-128,-90, -60 -40, -32, -24, -16, -8, -4, -2, 0, 2, 4, 8, 16, 24, 32, 40, 60, 90, 128};
#endif


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
    srand(seed[1]);
    p = (int *)dst;
    for(i=seed[2]%num; i<num; i+=4) {
        *p = rand();
        p++;
    }
    for(i=0; i<seed[2]%num; i+=4) {
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

    int ret, i;

    if(access(FILE_OF_MATRIX, F_OK|R_OK|W_OK) != 0) { //don't exist
        struct yuan_t data;
        fd = open(FILE_OF_MATRIX, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            printf("create %s error", FILE_OF_MATRIX);
            exit(0);
        }
        memset(&data, 0, sizeof(data));
        memset(data.sta, 0x55, NOA*SLOT);
        for(i=0; i<NOA; i++) {
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

    soa = (struct yuan_t *) mmap(0, SOY*NOA, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(soa == MAP_FAILED) {
        printf("mmap %s error: %d\n", FILE_OF_MATRIX,  errno);
        exit(0);
    }
    for(i=0; i<NOI+NOO; i++) {
        som[i] = &soa[i];
    }



    //printf("init none success\n");
    return;
}

void get_input_data() {
#if(TEST==1)
    int i;
    unsigned char r[2];
    get_random(r, 2);

    if(r[0]&0x01!=0x00) {
        idata[0]=THESHOLD;
    } else {
        idata[0] = 0;
    }


    if(r[1]&0x01!=0x00) {
        idata[1]=THESHOLD;
    } else {
        idata[1] = 0;
    }
    //printf("idata %d %d r: %d, %d\n", idata[0], idata[1], r[0], r[1]);

 #endif
}

void input_none() {
    int i;
    for(i=0; i<NOI; i++) {
        sum_p[i] = idata[i];
    }
}

int is_in_som(int idx, struct yuan_t *som[]) {
    int i;
    for(i=NOI+NOO; i<NOM; i++) {
        if(som[i] != NULL && idx == som[i]->idx){
            return i;
        }
    }
    return 0;
}

void update_som() {
    int i, j=NOI+NOO,k=0; 

    struct yuan_t *som_old[NOM]={NULL};
    memcpy(som_old, som, sizeof(som_old));

    for(i=NOI+NOO; i<NOA; i++) {
        if(sum_n[i] >= THESHOLD) {
            if(k = is_in_som(i, som_old)) {
                som[j] = som_old[k];
                som_old[k] = NULL;
            } else {
                som[j] = &soa[i];
               
            }
            j++;
            if(j>=NOM) {
                goto release;
            }
        }
    }

    for(i=NOI+NOO; i<NOM; i++) {
        if(som_old[i] != NULL) {
            som[j] = som_old[i];
            j++;
            if(j>=NOM) {
                goto release;
            }
        }
    }

    for(;j<NOM; j++) {
        som[j] = NULL;
    }

release:
    return;
}

unsigned short rt[NOM][NOA];
void sumn_to_table(int first) {
    int i;

    memset(sum_p, 0, sizeof(sum_p));

    //gettimeofday(&t_start, NULL);
    //#pragma omp parallel for 
    for(i=0; i<NOM; i++) { if((i>=NOI&&i<NOO+NOI) || som[i] == NULL) {continue;}
        int j, k;
        for(j=NOI; j<NOA; j++) { if(som[i]->idx==j || (som[i]->idx<NOI&&(j>=NOI&&j<NOO+NOI)) )continue;
            #ifndef NON_TRAIN
            if(first) {
                unsigned short s=0;
                int r;
                for(k=0; k<SLOT; k++) {
                    s+=som[i]->sta[j][k];
                }
                r=rt[i][j]%s;
                for(k=0; k<SLOT; k++) {
                    r=r-som[i]->sta[j][k];
                    if(r<=0) {
                        som[i]->dis[j] = k;
                        //#pragma omp atomic  
                        break;
                    }
                }
            }
            #else
            if(first) {
                int max=0;
                for(k=0; k<SLOT; k++) {
                    if(max < som[i]->sta[j][k]) {
                        max = som[i]->sta[j][k];
                        som[i]->dis[j] = k;
                    }
                }
            }
            #endif

            sum_p[j]+=slot_table[som[i]->dis[j]];
        }
    }
    //gettimeofday(&t_end, NULL);
}


void table_to_sump() {
    //nop
}

void feedback(int f) {
    int i,j,k,t;

    //printf("ticks %d fb %d\n", ticks, f);
    for(i=0; i<NOM; i++) { if((i>=NOI&&i<NOO+NOI) || som[i] == NULL){continue;}
        for(j=NOI; j<NOA; j++) { if(som[i]->idx==j || (som[i]->idx<NOI&&(j>=NOI&&j<NOO+NOI))  ) {continue;}
            t = som[i]->dis[j];
            if(som[i]->sta[j][t]+f<1) {
                som[i]->sta[j][t]=1;
            } else {
                som[i]->sta[j][t]+=f;
            }

            unsigned char max=0;
            for(k=0; k<SLOT; k++) {
                max = (max >= som[i]->sta[j][k] ? max : som[i]->sta[j][k]);
            }
            if(max >= 255) {
                for(k=0; k<SLOT; k++) {
                    som[i]->sta[j][k] = (som[i]->sta[j][k]+1)/2;
                }
            } else if(max < 80) {
                for(k=0; k<SLOT; k++) {
                    som[i]->sta[j][k] = som[i]->sta[j][k]*2;
                }
            }
        }
    }
}


void output_none() {
#if(TEST==1)
    int i;
    int oid;
    int success=1;

    if(ticks%1000==999) {
        printf("percent %d\n", debug_c*2);
        debug_c=0;
    }
    
    //printf("%d %d\n", idata[0], sum_p[1]);

    if(sum_p[2] >= THESHOLD) {
        if(idata[0]!=0 && idata[1]!=0) {
            debug_c++;
            feedback(1);
        } else {
            feedback(-1);
        }
    } else {
        if(idata[0]!=0 && idata[1]!=0) {
            feedback(-1);
        }
    }




    /*
    oid = idata[0]==0?0:1 + idata[1]==0?0:2;
    if(oid==0){return;}
    oid=oid+NOI-1;
    //printf("oid: %d sum p: %d %d %d\n", oid, sum_p[2], sum_p[3], sum_p[4]);
    for(i=NOI; i<NOI+NOO; i++) {
        if(i==oid && sum_p[i] < THESHOLD) {
            success = 0;
        }
        if(i!=oid && sum_p[i] >= THESHOLD) {
            success = 0;
        }
    }
    
    if(success) {
        debug_c++;
#ifndef NON_TRAIN
        feedback(1);
    } else {
        feedback(-1);
#endif
    }*/

#endif
}

void prob_print() {
    int i;
    int num=0;
    for(i=NOI+NOO; i<NOA; i++) {
        if(sum_n[i] > THESHOLD) {
            num++;
        }
    }
    printf("num %d\n", num);
}

void run() {

    int i;

    get_random((unsigned char*)&rt[0][0], sizeof(unsigned short)*NOM*NOA);
    get_input_data();
    for(i=0; i<20; i++) {
        input_none();
        memcpy(sum_n, sum_p, sizeof(sum_n));
        //prob_print();
        update_som();
        sumn_to_table(i==0?1:0);
        table_to_sump();
    }
    output_none();

    ticks++;
}

void close_none() {
    if(soa!=NULL) {
        msync((void *)soa, SOY*NOA, MS_ASYNC);
        munmap((void *)soa, SOY*NOA) ;
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
    //printf("run kernel %d times\n", times);

    init_random();
    init_none();

    //gettimeofday(&t_start, NULL);
    for(counter=0; counter<times; counter++) {
        run();
    }

    //gettimeofday(&t_end, NULL);
    close_none();
    close_random();
    //printf("run kernel %d times success\n", times);

    //cost_time = (t_end.tv_sec*1000000l+t_end.tv_usec) - (t_start.tv_sec*1000000l+t_start.tv_usec);

    //printf("Cost time: %lds+%ldus\t", cost_time/1000000, cost_time%1000000);
    printf("run :%d times over\n", ticks);
}







 




