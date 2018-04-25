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
#include <math.h>

#define NOI     (14*14)
#define NOO     10
#define NOF     20

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
float score_terminal[2]={0.0,0.0};

int stage=0;


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

int update_input_data(int idx) {
    int i,j;
    int c=idx;
    //memcpy(&ia[0], &data[16+14*14*c], NOI);
    for(i=0; i<14; i++) {
        for(j=0; j<14; j++) {
            ia[i*14+j] = (data[16+28*28*c+i*2*28+j*2] + data[16+28*28*c+i*2*28+j*2 + 1] + 
                data[16+28*28*c+(i*2+1)*28+j*2] + data[16+28*28*c+(i*2+1)*28+j*2+1])/4;
        }
    }
    answer = label[8+c];
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
            wa[i][j]%=10000;
        }
    }

    for(i=0; i<NOO; i++) {
        ba[i]%=200;
    }

    return 0;
}

float non_line_convert(float l) {
    if(l < 0.0)
        return 0.0;
    if(l > 1.0)
        return 1.0;
    if(l > 0.5)
        return sqrt(l);
    else
        return l*l;
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
    for(i=0; i<NOF; i++) {
        update_input_data(i);
        score_sum += _calc_(answer);
    }
    return score_sum;

}


int best_w_parameter(unsigned int *p, int level) {
    int i;
    int maxi=-1;
    float maxs=0, s;
    unsigned int org = *p;

    if(level==1) {
        for(i=0; i<21; i++) {
            *p=i*500;
            s = calc_once();
            if(maxs < s) {
                maxs = s;
                maxi = i;
            }
        }
        *p=maxi*500;
        latest_score = maxs;
    } else {
        for(i=-10; i<11; i++) {
            if(org + i*500/level > 10000) 
                *p = 10000;
            else if(org + i*500/level <= 0)
                *p = 0;
            else 
                *p = org + i*500/level;
            s = calc_once();
            if(maxs < s) {
                maxs = s;
                maxi = i;
            }
        }
        if(org + maxi*500/level > 10000) 
            *p = 10000;
        else if(org + maxi*500/level <= 0)
            *p = 0;
        else 
            *p=org + maxi*500/level;
        latest_score = maxs;
    }
    printf("w latest_score:%f, maxs:%f\n", latest_score, maxs);
    return 0;
}


int best_b_parameter(unsigned int *p, int level) {
    int i;
    int maxi=-1;
    float maxs=0, s;
    unsigned int org = *p;
    if(level==1) {
        for(i=0; i<21; i++) {
            *p=i*10;
            s = calc_once();
            if(maxs < s) {
                maxs = s;
                maxi = i;
            }
        }
        *p=maxi*10;
        latest_score = maxs;
    } else {
        for(i=-10; i<11; i++) {
            if(org+i>200)
                *p = 200;
            else if(org+i<=0)
                *p = 0;
            else 
                *p=org + i;
            s = calc_once();
            if(maxs < s) {
                maxs = s;
                maxi = i;
            }
        }
        if(org+maxi>200)
            *p = 200;
        else if(org+maxi<=0) 
            *p=0;
        else
            *p=org + maxi;
        latest_score = maxs;
    }
    printf("b latest_score:%f\n", latest_score);
    return 0;
}


int iter_parameters(int level) {
    int i, j;
    for(i=0; i<NOI; i++) {
        for(j=0; j<NOO; j++) {
            printf("stage %d image pixel [%3d, %2d]    ", stage, i, j);
            best_w_parameter(&wa[i][j], level);
        }
    }

    for(i=0; i<NOO; i++) {
        printf("stage %d b parameters [%d]    ", stage, i);
        best_b_parameter(&ba[i], level);
    }
}

void test_input() {
    int i, j;
    update_input_data(10);
    for(i=0; i<NOI; i++) {
        printf("%3d ", ia[i]);
        if(i%14==0) {
            printf("\n");
        }
    }
    printf("\nanswer is %d\n", answer);
}

int test_train(int idx) {

    update_input_data(idx);
    int i, j, t;
    int maxi=0;
    float maxs=0;

    for(j=0; j<NOO; j++) {
        oa[j]=0;
        for(i=0; i<NOI; i++) {
            oa[j]+=(1.0*ia[i])*(wa[i][j]/10000.0);
        }
        oa[j]+=ba[j];
        if(maxs < oa[j]) {
            maxs=oa[j];
            maxi=j;
        }
    }

    return (maxi==answer);
}


int main() {
    
    int i;
    int correct=0;
    init_random();
    init_input_data();
    init_random_parameters();
    
    //train
    stage=1;
    for(i=0; i<10000; i++) {  
        iter_parameters(1);
        printf("correct rate: %f\n", latest_score);
        if(score_terminal[0] == score_terminal [1] 
                && score_terminal[1] == latest_score) {
            break;
        } else {
            score_terminal[0] = score_terminal[1];
            score_terminal[1] = latest_score;
        }
    }

    //train
    stage=2;
    score_terminal[0]=0.0;
    score_terminal[1]=0.0;
    for(i=0; i<10000; i++) {  
        iter_parameters(10);
        printf("correct rate: %f\n", latest_score);
        if(score_terminal[0] == score_terminal [1] 
                && score_terminal[1] == latest_score) {
            break;
        } else {
            score_terminal[0] = score_terminal[1];
            score_terminal[1] = latest_score;
        }
    }


    //train
    stage=3;
    score_terminal[0]=0.0;
    score_terminal[1]=0.0;
    for(i=0; i<10000; i++) {  
        iter_parameters(100);
        printf("correct rate: %f\n", latest_score);
        if(score_terminal[0] == score_terminal [1] 
                && score_terminal[1] == latest_score) {
            break;
        } else {
            score_terminal[0] = score_terminal[1];
            score_terminal[1] = latest_score;
        }
    }


    //test
    for(i=NOF; i<60000; i++) {
        if(test_train(i)>0) {
            correct++;
        }

    }
    
    printf("\n--------------------------\n");
    printf("train result: %f\n", latest_score/NOF);
    printf("test  result: %f\n", correct/(60000.0-NOF));
    close_input_data();
    close_random();
}












