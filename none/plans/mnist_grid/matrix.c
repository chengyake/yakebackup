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

#define NOI     784
#define NOO     10
#define NOF     1000

unsigned char *data, *label;

unsigned char ia[NOI], answer;
unsigned int wa[NOI][NOO];
unsigned int ba[NOO];
unsigned int oa[NOO];

unsigned int maxw[NOI][NOO];
unsigned int maxb[NOO];
unsigned int *wp;


unsigned int best_flag=0;
float latest_score=0;




int rd, fd;
int init_random() {
    rd = open ("/dev/urandom", O_RDONLY);
    if (rd <= 0) {
        printf("open /dev/urandom error\n");
        return -1;
    }
    return 0;
}
//#define RANDOM
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


void init_input_data() {

    int ret;
    label = (unsigned char *)malloc(60008);
    if(access("./data/train-labels.idx1-ubyte", F_OK|R_OK|W_OK) != 0) { //don't exist
        printf("access label error\n");
        exit(0);
    } else {
        fd = open("./data/train-labels.idx1-ubyte", O_RDONLY, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            printf("open labels error\n");
            exit(0);
        }
    }

    ret = lseek(fd, 0, SEEK_SET);
    if(ret < 0) {
        printf("create lseek error\n");
        exit(0);
    }
    
    ret = read(fd, label, 60008);

    close(fd);

    //map data
    data = (unsigned char *)malloc(47040016);
    if(access("./data/train-images.idx3-ubyte", F_OK|R_OK|W_OK) != 0) { //don't exist
        printf("create images error\n");
        exit(0);
    } else {
        fd = open("./data/train-images.idx3-ubyte", S_IRUSR|S_IWUSR);
        if(fd < 0) {
            printf("open images error\n");
            exit(0);
        }
    }

    ret = lseek(fd, 0, SEEK_SET);
    if(ret < 0) {
        printf("lseek images fixed space error\n");
        exit(0);
    }

    ret = read(fd, data, 47040016);
    close(fd);

    return;
}

int update_input_data() {
    static int c=1;
    memcpy(&ia[0], &data[16+28*28*c], NOI);
    answer = label[8+c];
    c++;
    if(c>=60000) {c = 0;}
    return 0;
}

void close_input_data() {
    free(label);
    free(data);

}

int init_random_parameters() {
    
    int i,j;
    get_random((unsigned char*)&wa[0], sizeof(wa));
    get_random((unsigned char*)&ba[0], sizeof(ba));
    for(i=0; i<NOI; i++) {
        for(j=0; j<NOO; j++) {
            wa[i][j]%=20000;
        }
    }

    for(i=0; i<NOO; i++) {
        ba[i]%=60;
    }

    return 0;
}

//one frame per change
float _calc_(int result) {
    int i, j, t;
    int sum=0;

    //#pragma omp parallel for
    for(j=0; j<NOO; j++) {
        oa[j]=0;
        for(i=0; i<NOI; i++) {
            oa[j]+=(1.0*ia[i])*(wa[i][j]/10000.0);
        }
        oa[j]+=ba[j];
        sum+=oa[j];
    }

    return 1.0*oa[result]/sum;
}


//1000 scores
float calc_once() {
    int i;
    int ret;
    float score_sum=0.0;
    //printf("calc once\n");
    for(i=0; i<NOF; i++) {
        update_input_data();
        score_sum += _calc_(answer);
    }
    //printf("once score:%f\n", score_sum);
    return score_sum;

}


int best_w_parameter(unsigned int *p) {
    int i;
    int maxi=-1;
    float maxs=0, s;
    unsigned int org = *p;
    for(i=0; i<21; i++) {
        *p=i*1000;
        s = calc_once();
        if(maxs < s) {
            maxs = s;
            maxi = i;
        }
    }
    
    //if(maxs > latest_score-0.001) {
        *p=maxi*1000;
        latest_score = maxs;
    //}
    printf("w latest_score:%f, maxs:%f\n", latest_score, maxs);
    return 0;
}


int best_b_parameter(unsigned int *p) {
    int i;
    int maxi=-1;
    float maxs=0, s;
    unsigned int org = *p;
    for(i=0; i<21; i++) {
        *p=i*3;
        s = calc_once();
        if(maxs < s) {
            maxs = s;
            maxi = i;
        }
    }
    
    //if(maxs > latest_score-0.001) {
        *p=maxi*3;
        latest_score = maxs;
    //}

    printf("b latest_score:%f\n", latest_score);
    return 0;
}


int iter_parameters() {
    int i, j;
    for(i=0; i<NOI; i++) {
        for(j=0; j<NOO; j++) {
            printf("image pixel [%3d, %2d]    ", i, j);
            best_w_parameter(&wa[i][j]);
        }
    }

    for(i=0; i<NOO; i++) {
        printf("b parameters [%d]    ", i);
        best_b_parameter(&ba[i]);
    }
}

void test_input() {
    int i, j;
    update_input_data();
    for(i=0; i<NOI; i++) {
        printf("%d ", ia[i]);
        if(i%28==0) {
            printf("\n");
        }
    }
    printf("answer is %d\n", answer);
}



int main() {
    
    int i;
    
    init_random();
    //map data to memory
    init_input_data();
    //fill parameters
    init_random_parameters();
    
    for(i=0; i<10000; i++) {  
        iter_parameters();
        printf("correct rate: %f\n", latest_score);
    }
    close_input_data();
    close_random();
}












