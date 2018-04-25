#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>


static int fd;

int32_t init_random() {

    fd = open ("/dev/urandom", O_RDONLY);
    if (fd <= 0) {
        //logerr("open /dev/urandom error\n");
        return -1;
    }
    return 0;
}

int32_t get_random(int8_t *data, uint32_t num) {
    
    int32_t ret;

/*
    if(fd <= 0) {
        init_random();
    }
*/

    ret = read(fd, data, num);
    if(ret < 0) {
        //logerr("get random num error when read /dev/urandom");
        return -1;
    }
    return ret;
}

void close_random () {
    close (fd);
    fd = 0;
}



#define NUM 10

int main() {
    
    int i,j;
    int *p;
    char a[NUM*4]={0};
    int64_t Em;
    uint64_t Emm;
    
    p=(int *)&a[0];
#if 0
    init_random();
    get_random(a, sizeof(a));
#else
    init_random();
    get_random(a, sizeof(a));
    
    for(i=0; i<NUM; i++) {
        p[i]=p[i]/2+0x3FFFFFFF;
    }
#endif

    for(i=0; i<NUM; i++) {
        printf("%d\n", p[i]);
    }
    printf("%d\n", 0x7FFFFFFF);


    //get Em
    Em=0;
    for(i=0; i<NUM; i++) {
        Em+=p[i];
    }
    Em/=NUM;
    
    printf("%ld\n", Em);

    Emm=0;
    for(i=0; i<NUM; i++) {
        Emm+=abs(Em-p[i]);
    }
    Emm = Emm/NUM;


    printf("%lu\n", (uint64_t)Emm*1000/0x7FFFFFFF);





    
    return 0;
}


