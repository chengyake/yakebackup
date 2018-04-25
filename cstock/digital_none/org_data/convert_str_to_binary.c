#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <linux/types.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
//2010-01-01 ~ 2016-05-31

struct table_t {
    unsigned char code[16];
    float avg[1556];
};



void write_db(struct table_t data[]) {

    int i;
    int ret;
    int fd;

    fd = open("tmp.db", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if(fd < 0) {
        printf("create tmp.db error");
        exit(0);
    }
    for(i=0; i<300; i++) {
        ret = write(fd, &data[i], sizeof(struct table_t));
        if(ret < 0) {
            printf("write TDataBase.db error");
            exit(0);
        }
    }
    close(fd);

}

int main() {
    
    FILE *fp;
    int ret, i, j, z;
    char af[64];
    char *str = NULL;
    struct table_t table[300];
    size_t len = 0;
    ssize_t rlen = 0;
    fp = fopen("hs300_20100101_20160531_pre",  "r");
        

    for(z=0; z<300; z++) {
        rlen = getline(&str, &len, fp);
        for(i=0; i<rlen; i++) {
            if(str[i]!=',') {
                table[z].code[i] = str[i];
            } else {
                table[z].code[i] = 0;
                break;
            }
        }

        for(i=0; i<1556; i++) {
            rlen = getline(&str, &rlen, fp);
            for(j=0; j<len; j++) {
                if(str[j]==','){
                    memset(af, 0, 64);
                    memcpy(af, &str[j+1], rlen-j-2);
                    table[z].avg[i]=atof(af);
                    break;
                }
            }
        }
    }
    fclose(fp);
    printf("%s %f %f\n", table[299].code, table[299].avg[0], table[299].avg[1555]);
    
    write_db(table);

    free(str);
}



