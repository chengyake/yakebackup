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
#define NOF     50

unsigned char *data, *label;

int answer;
int ia[NOI];
int wa[NOI][NOO];
int ba[NOO];
int oa[NOO];

int maxw[NOI][NOO];
int maxb[NOO];
int *wp;

float latest_score=0;
float score_terminal[2]={0.0,0.0};

float stage=1.0;





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
                data[16+28*28*c+(i*2+1)*28+j*2] + data[16+28*28*c+(i*2+1)*28+j*2+1]);
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
            wa[i][j]=wa[i][j]%20000-10000;

        }
    }

    for(i=0; i<NOO; i++) {
        ba[i]=ba[i]%20000-10000;
    }

    return 0;
}

#if 1
float _calc_(int result) {
    int i, j;
    long int sum=0;

    //#pragma omp parallel for
    for(j=0; j<NOO; j++) {
        oa[j]=0;
        for(i=0; i<NOI; i++) {
            oa[j]+=ia[i]*(wa[i][j])/256;
            //printf("oa %d\n", ia[i]*wa[i][j]);
        }
        oa[j]/=NOI;
        oa[j]+=ba[j]/10;
        sum+=oa[j];
    }
    //printf("---%ld   %ld    = %f\n", oa[result], sum, oa[result]/(sum+10000.0*255));
    return oa[result]/(sum+30000.0*NOO);

    //return (oa[result]*oa[result])/((sum+(NOO/10.0)+10)*(sum+(NOO/10.0)+10));   
}
#else

#endif

float calc_once() {
    int i;
    int ret;
    float score_sum=0.0;
    for(i=0; i<NOF; i++) {
        update_input_data(i);
        score_sum += _calc_(answer);
    }
    return score_sum/NOF;
}


int best_parameter(int *p, float level) {
    int i;
    int maxi=-11;
    float maxs=-1.0*NOF, s;
    float org = *p;

    if(level>0.09 && level < 0.11) {
        org = 0;
    }
    for(i=-10; i<11; i++) {
        if(org+10000*i*level < -10000)
            *p = -10000;
        else if(org+10000*i*level > 10000)
            *p = 10000;
        else
            *p=org+10000*i*level;

        s = calc_once();
        if(maxs < s) {
            maxs = s;
            maxi = i;
        }
    }

    //if(maxs > latest_score) {
    if(org+10000*maxi*level < -10000)
        *p = -10000;
    else if(org+10000*maxi*level > 10000)
        *p = 10000;
    else
        *p=org+10000*maxi*level;
    latest_score = maxs;
    //}

    printf("w latest_score:%2.10f, maxs:\n", latest_score);
    return 0;
}

int iter_parameters(float level) {
    int i, j;
    for(i=0; i<NOI; i++) {
        for(j=0; j<NOO; j++) {
            printf("stage %4f image pixel [%3d, %2d]    ", stage, i, j);
            best_parameter(&wa[i][j], level);
        }
    }

    for(i=0; i<NOO; i++) {
        printf("stage %4f b parameters [%d]    ", stage, i);
        best_parameter(&ba[i], level);
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
    int maxs=-10000*44;

    for(j=0; j<NOO; j++) {
        oa[j]=0;
        for(i=0; i<NOI; i++) {
            oa[j]+=(ia[i])*(wa[i][j])/256;
        }
        oa[j]+=ba[j]/10;
        if(maxs < oa[j]) {
            maxs=oa[j];
            maxi=j;
        }
    }

    return (maxi==answer);
}


void test_train_image() {
    int i;
    int cn=0;
    printf("\n----------NOF-%d---------------\n", NOF);
    for(i=0; i<NOF; i++) {
        if(test_train(i)>0) {
            cn++;
        }

    }
    printf("train result: %f\n", 1.0*cn/NOF);
    cn=0;
    for(i=NOF; i<60000; i++) {
        if(test_train(i)>0) {
            cn++;
        }

    }
    printf("test  result: %f\n", cn/(60000.0-NOF));

}

int main() {
    
    int i, j;

    init_random();
    init_input_data();
    init_random_parameters();
    
    //train
    for(i=0; i<3; i++) {
        stage*=0.1;
        for(j=0; j<2000; j++) {
            iter_parameters(stage);
            printf("run %d times and get correct rate: %f\n", j, latest_score);
            if(score_terminal[0] == latest_score) {
                break;
            } else {
                score_terminal[0] = latest_score;
            }
        }
    }

    test_train_image();

    close_input_data();
    close_random();
}












