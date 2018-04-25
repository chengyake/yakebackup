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

#include "none_config.h"
#include "none_matrix.h"
#include "none_debug.h"

struct yuan_t * matrix = NULL;

int32_t init_matrix_database() {

    uint32_t i, j, k;
    int32_t fd;
    int32_t ret;
    int32_t is_new = 0;

    if(access(FILE_OF_MATRIX, F_OK|R_OK|W_OK) != 0) { //don't exist
        uint8_t data = 0;

        fd = open(FILE_OF_MATRIX, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            logerr("create %s error", FILE_OF_MATRIX);
            return fd;
        }
        for(i=0; i<(SIZE_OF_MATRIX); i++) {
            ret = write(fd, &data, 1);
            if(ret < 0) {
                logerr("write TDataBase.db error");
                return ret;
            }
        }
        is_new = 1;
    } else {
        //restore db queue
        fd = open(FILE_OF_MATRIX, O_RDWR, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            logerr("open %s error", FILE_OF_MATRIX);
            return ret;
        }
    }

    ret = lseek(fd, 0, SEEK_SET);
    if(ret < 0) {
        logerr("create %s fixed space error", FILE_OF_MATRIX);
        return ret;
    }

    //mmap and get matrix point
    matrix = (struct yuan_t *) mmap(0, SIZE_OF_MATRIX, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(matrix == MAP_FAILED) {
        logerr("mmap %s error: %d", FILE_OF_MATRIX, errno);
        return -1;
    }

    //fill yuan.id and yuan.type
    if(is_new) {
        for(i=0; i<YUAN_NUM_OF_ALL; i++) {
            matrix[i].id = i;
            if(i>=YUAN_IDX_OF_INFO_S && i<=YUAN_IDX_OF_INFO_E) {
                matrix[i].type = YUAN_TYPE_OF_INFO;
            }
            if(i>=YUAN_IDX_OF_INPUT_S && i<=YUAN_IDX_OF_INPUT_E) {
                matrix[i].type = YUAN_TYPE_OF_INPUT;
            }
            if(i>=YUAN_IDX_OF_NORMAL_S && i<=YUAN_IDX_OF_NORMAL_E) {
                matrix[i].type = YUAN_TYPE_OF_NORMAL;
            }
            if(i>=YUAN_IDX_OF_OUTPUT_S && i<=YUAN_IDX_OF_OUTPUT_E) {
                matrix[i].type = YUAN_TYPE_OF_OUTPUT;
            }
#if 1
            for(j=0; j<YUAN_NUM_OF_ALL; j++) {
                for(k=0; k<YUAN_STATISTICS_NUM; k++) {
                    matrix[i].statistics[j][k]=YUAN_FULLHOLD;
                }
            }
#endif
        }
    }

    close(fd);
 
    return 0;
}

static int32_t get_snapshot_file_name_by_time(int8_t *data) {
    
    time_t tmp;
    struct tm *p;
    
    if(data == NULL) {
        logerr("params of get_current_time_string() error");
        return -1;
    }

    tmp = time(NULL);
    p = localtime(&tmp);
    sprintf(data, "%4d%02d%02d_%02d%02d%02d.snap",
        (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    return 0;

}


int32_t make_matrix_snapshot() {
    
    int32_t fd;
    int32_t ret;
    int8_t file_name[32]={0};

    msync((void *)matrix, SIZE_OF_MATRIX, MS_ASYNC);
    get_snapshot_file_name_by_time(&file_name[0]);
    
    if(access(file_name, F_OK|R_OK|W_OK) == 0) { //don't exist
        logerr("snapshot capture %s error", FILE_OF_MATRIX);
        return -1;
    } else {
        fd = open(file_name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            logerr("create %s error when snap capture", file_name);
            return fd;
        }
        ret = write(fd, matrix, SIZE_OF_MATRIX);
        if(ret < 0) {
            logerr("write TDataBase.db error");
            return ret;
        }
    }
    
    loginfo("snap capture %s success", file_name);
    return 0;
}

int32_t restore_matrix_snapshot(char *file_name) {

    int32_t fd;
    int32_t ret;
    if(file_name == NULL) {
        logerr("restore capture %s error: File name is null", file_name);
        return -1;
    }
 
    loginfo("restore %s to matrix", file_name);
    
    if(access(file_name, F_OK|R_OK|W_OK) != 0) { //don't exist
        logerr("restore capture %s error:No this file exist", file_name);
        return -1;
    } else {
        fd = open(file_name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            logerr("create %s error when snap capture", file_name);
            return fd;
        }
        ret = read(fd, matrix, SIZE_OF_MATRIX);
        if(ret < 0) {
            logerr("write data of %s to matrix error", file_name);
            return ret;
        }
    }
    
    msync((void *)matrix, SIZE_OF_MATRIX, MS_ASYNC);

    loginfo("restore capture %s success", file_name);
    return 0;
}


int32_t close_matrix_database() {
    //msync and munmap
    msync((void *)matrix, SIZE_OF_MATRIX, MS_ASYNC);
    munmap((void *)matrix, SIZE_OF_MATRIX) ;
    return 0;
}



struct yuan_t *get_matrix_by_id(uint32_t id) {
    if(id > YUAN_NUM_OF_ALL) {
        printf("Error: idx range is too big\n");
    }
    return &matrix[id];
}















