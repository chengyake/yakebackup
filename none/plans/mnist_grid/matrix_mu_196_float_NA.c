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
#define NOF     100
#define LOOP    2

unsigned char *data, *label;

int answer;
float ia[NOI];
float wa[NOO][NOI];
float ba[NOO];
float wa1[NOO][NOI];
float ba1[NOO];
float oa[NOO];

float maxw[NOO][NOI];
float maxb[NOO];
float *wp;

float latest_score=0;
float score_terminal[2]={0.0,0.0};

float stage=1.0;


int tt=0;


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
    int i,j;
    static int c=0;
    //memcpy(&ia[0], &data[16+14*14*c], NOI);
    for(i=0; i<14; i++) {
        for(j=0; j<14; j++) {
            ia[i*14+j] = (data[16+28*28*c+i*2*28+j*2] + data[16+28*28*c+i*2*28+j*2 + 1] + 
                data[16+28*28*c+(i*2+1)*28+j*2] + data[16+28*28*c+(i*2+1)*28+j*2+1])/4.0/255.0;
        }
    }
    answer = label[8+c];
    c++;
    if(c>=NOF*tt+NOF) {c = NOF*tt;}
    return 0;
}

int update_input_data_idx(int idx) {
    int i,j;
    int c=idx;
    //memcpy(&ia[0], &data[16+14*14*c], NOI);
    for(i=0; i<14; i++) {
        for(j=0; j<14; j++) {
            ia[i*14+j] = (data[16+28*28*c+i*2*28+j*2] + data[16+28*28*c+i*2*28+j*2 + 1] + 
                data[16+28*28*c+(i*2+1)*28+j*2] + data[16+28*28*c+(i*2+1)*28+j*2+1])/4.0/255.0;
        }
    }
    answer = label[8+c];
    return 0;
}

void close_input_data() {
    free(label);
    free(data);

}

int init_random_parameters() {
    
    int i,j;

    int wat[NOO][NOI];
    int bat[NOO];
    get_random((unsigned char*)&wat[0], sizeof(wat));
    get_random((unsigned char*)&bat[0], sizeof(bat));
    for(i=0; i<NOO; i++) {
        for(j=0; j<NOI; j++) {
            wa[i][j]=1.0*(wat[i][j]%20000-10000)/10000.0;
            wa1[i][j]=0.0;

        }
    }

    for(i=0; i<NOO; i++) {
        ba[i]=1.0*(bat[i]%20000-10000)/10000.0;
        ba1[i]=0.0;
    }

    return 0;
}

float non_line_convert(float l) {
    if(l < -1.0)
        return -1.0;
    if(l > 1.0)
        return 1.0;
    if(l >=0) {
        if(l > 0.5)
            return sqrt(l);
        else
            return l*l;
    } else {
        if(l < -0.5)
            return -sqrt(-l);
        else
            return -l*l;
    }
}

//chengyake
//return sqrt(oa[result])/(sum+11.0);         //  86/56
//return sqrt(sqrt(oa[result]))/(sum+11.0);   //  84/51
//return (oa[result])/(sum/4+11.0);           //  86/57
//return (oa[result])/(sum/2+11.0);           //  84/58
//return oa[result]/(sum+11.0);               //  82/53
//return oa[result]/(sum+11.0);               //  82/53
//return oa[result]/(sum-oa[result]+11.0)+oa[result];       //  82/53

float _calc_(int result) {
    int i, j, t;
    float softmax=0.0;
    //#pragma omp parallel for
    for(i=0; i<NOO; i++) {
        oa[i]=0.0;
        for(j=0; j<NOI; j++) {
            oa[i]+=ia[j]*(wa[i][j]);
        }
        oa[i]/=NOI;
        oa[i]+=(ba[i]/10);

        softmax+=pow(2.71828, oa[i]*50);
    }

    return pow(2.71828, oa[result]*50)/softmax;
}

float calc_once() {
    int i;
    int ret;
    float score=0.0;
    for(i=0; i<NOF; i++) {
        update_input_data();
        score +=  _calc_(answer);
    }
    return score/NOF;
}


int best_w_parameter(float *p, float level) {
    int i;
    int maxi=-11;
    float maxs=-1.0*NOF, s;
    float org = *p;

    if(level>0.09 && level < 0.11) {
        org = 0.0;
    }
    for(i=-10; i<11; i++) {
        if(org+i*level < -1.0)
            *p = -1.0;
        else if(org+i*level > 1.0)
            *p = 1.0;
        else
            *p=org+i*level;

        s = calc_once();
        if(maxs < s) {
            maxs = s;
            maxi = i;
        }
    }

    //if(maxs > latest_score) {
    if(org+maxi*level < -1.0)
        *p = -1.0;
    else if(org+maxi*level > 1.0)
        *p = 1.0;
    else
        *p=org+maxi*level;
    latest_score = maxs;
    //}

    printf("w latest_score:%2.10f, maxs:\n", latest_score);
    return 0;
}


int best_b_parameter(float *p, float level) {
    int i;
    int maxi=-1;
    float maxs=-1.0*NOF, s;
    float org = *p;

    if(level>0.09 && level < 0.11) {
        org = 0.0;
    }
    for(i=-10; i<11; i++) {
        if(org+i*level<-1.0)
            *p = -1.0;
        else if(org+i*level>1.0)
            *p = 1.0;
        else
            *p=org+i*level;

        s = calc_once();
        if(maxs < s) {
            maxs = s;
            maxi = i;
        }
    }
    //if(maxs > latest_score) {
    if(org+maxi*level < -1.0)
        *p = -1.0;
    else if(org+maxi*level > 1.0)
        *p = 1.0;
    else
        *p=org+maxi*level;
    latest_score = maxs;
    //}
    printf("b latest_score:%2.10f, maxs:\n", latest_score);
    return 0;
}


int iter_parameters(float level) {
    int i, j;
    for(i=0; i<NOO; i++) {
        for(j=0; j<NOI; j++) {
            printf("stage %4f image pixel [%2d, %3d]    ", stage, i, j);
            best_w_parameter(&wa[i][j], level);
        }
    }

    for(i=0; i<NOO; i++) {
        printf("stage %4f b parameters [%d]    ", stage, i);
        best_b_parameter(&ba[i], level);
    }
}

void test_input() {
    int i, j;
    update_input_data_idx(10);
    for(i=0; i<NOI; i++) {
        printf("%3f ", ia[i]);
        if(i%14==0) {
            printf("\n");
        }
    }
    printf("\nanswer is %d\n", answer);
}

int test_train(int idx) {

    update_input_data_idx(idx);
    int i, j;

    for(i=0; i<NOO; i++) {
        oa[i]=0.0;
        for(j=0; j<NOI; j++) {
            oa[i]+=(ia[j])*(wa1[i][j]);
        }
        oa[i]/=NOI;
        oa[i]+=(ba1[i]/10);
    }
    
    
    for(i=0; i<NOO; i++) {
        if(i==answer) continue;
        if(oa[answer] < oa[i]) {
            return 0;
        }
    }


    return 1;
}


void test_train_image() {
    int i;
    int cn=0;
    printf("\n----------NOF-%d---------------\n", NOF);
    for(i=0; i<NOF*LOOP; i++) {
        if(test_train(i)>0) {
            cn++;
        }

    }
    printf("train result: %f\n", 1.0*cn/NOF/LOOP);
    cn=0;
    for(i=NOF*LOOP; i<60000; i++) {
        if(test_train(i)>0) {
            cn++;
        }

    }
    printf("test  result: %f\n", cn/(60000.0-NOF*LOOP));

}

int main() {
    
    int i, j, z;
    int times;

    init_random();
    init_input_data();
    init_random_parameters();

    //train
    for(z=0; z<LOOP; z++) {
        stage=1.0;
        for(i=0; i<3; i++) {
            times=100/(i+1);
            stage*=0.1;
            for(j=0; j<50; j++) {
                iter_parameters(stage);
                printf("run %d times and get correct rate: %f\n", j+1, latest_score);
                if(score_terminal[0] == latest_score || latest_score > 0.995) {
                    break;
                } else {
                    score_terminal[0] = latest_score;
                }
            }
        }
        for(i=0; i<NOO; i++) {
            for(j=0; j<NOI; j++) {
                wa1[i][j]+=wa[i][j];
            }
            ba1[i]+=ba[i];
        }

        memset(wa, 0, sizeof(wa));
        memset(ba, 0, sizeof(ba));
        tt++;
    }

    for(i=0; i<NOO; i++) {
        for(j=0; j<NOI; j++) {
            wa1[i][j]/=LOOP;
        }
        ba1[i]/=LOOP;
    }

    test_train_image();

    close_input_data();
    close_random();
}












