#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <linux/types.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
//Train     2010-12-02 ~ 2015-12-31   1236
//Test      2016-01-01 ~ 2016-12-31    244

//#define  NOLINE     1480
#define  NOLINE     1236
#define  NOSTOCK    27
#define  NOITEM     4

struct table_t {
    unsigned char date[16];
    float data[NOSTOCK][NOITEM];
};

struct table_t table[NOLINE];

int main() {

    FILE *fp;
    int ret, i, j, k,l;
    char af[64];
    size_t len = 0;
    ssize_t rlen = 0;
    fp = fopen("data.txt",  "r");

    memset(table, 0, sizeof(table));

    for(l=0; l<NOLINE; l++) {
        char *str = NULL;
        rlen = getline(&str, &len, fp);
        for(i=0; i<rlen; i++) {
            if(str[i]!=',') {
                table[l].date[i] = str[i];
            } else {
                table[l].date[i] = 0;
                break;
            }
        }
        k=0;
        j=i++;
        for(; i<rlen; i++) {
            if(str[i]==',') {
                memset(af, 0, 64);
                memcpy(af, &str[j+1], i-j-1);
                table[l].data[k/NOITEM][k%NOITEM] = atof(af);
                j=i;
                k++;
                if(k>=NOITEM*NOSTOCK) {
                    break;
                }
            }
        }
        free(str);
    }
    fclose(fp);
    printf("%s %f %f\n", table[0].date, table[0].data[0][0], table[NOLINE-1].data[NOSTOCK-1][3]);
}



