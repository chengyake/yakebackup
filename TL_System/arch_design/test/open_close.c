#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>




int main(int argc, char **argv) {
    
    int i,j;
    unsigned long count=0;

    for(i=0; i<100000; i++) {
        for(j=0; j<100; j++) {
            open_close();
            printf("\ropen close times: %lu", count++);
        }
    }
    printf("\n");
}

int open_close() {

    int fd;
    long fpos;

    fd = open("open_lseek_close", O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        printf("open file open_file_test failed!\n%s\n", strerror(errno));
        return -1;
    }

    fpos = lseek(fd, 0, SEEK_END);
    if (fpos < 0) {
        printf("lseek failed!\n%s\n", strerror(errno));
        close(fd);
        return -1;
    }
    
    close(fd);
}
