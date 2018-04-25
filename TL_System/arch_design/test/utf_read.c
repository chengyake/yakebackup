#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char **argv)
{   
    int i;
    int fd, len;
    char str[128];

    fd = open("test", O_RDWR, 0644);
    if (fd == -1)
    {
        printf("open file open_file_test failed!\n%s\n", strerror(errno));
        return -1;
    }
    printf("open open_file_test ok!\n");
    
    len = read(fd, str, sizeof(str));

    for(i=0; i<len; i++) {
        printf("0x%x ", str[i]);
    }

    printf("\n");
    close(fd);
}
