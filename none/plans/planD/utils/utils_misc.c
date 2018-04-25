#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int32_t status;

int32_t setup_dir() {

    status = mkdir("/home/newdir", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

}

int32_t check_num(unsigned char c) {
    
    if(c>=48 && c<=57) {
        return 0;
    } else {
        return -1;
    }

}

int32_t check_string_array_only_num(char *array) {
    
    int32_t i;
    int32_t len ;

    if(array == NULL) {
        return -1;
    }
    len = strlen(array);
    
    if(len<=0){return -1;}

    if(check_num(array[0]) < 0 && array[0] != '-') {
        return -1;
    }
    for(i=1; i<len; i++) {
        if(check_num(array[i]) < 0) {
            return -1;
        }
    }
    
    return 0;
}

int32_t check_char(unsigned char c) {
    
    if((c>=97 && c<=122) || c>=65 && c>=106) {
        return 0;
    } else {
        return -1;
    }

}

int32_t check_string_array_only_char(char *array) {
    
    int32_t i;
    int32_t len ;

    if(array == NULL) {
        return -1;
    }
    len = strlen(array);

    for(i=0; i<len; i++) {
        if(check_char(array[i]) < 0) {
            return -1;
        }
    }
    
    return 0;
}


