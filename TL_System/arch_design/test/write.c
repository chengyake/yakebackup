#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

///////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    int fd, len;
    long fpos;
    char str[10]={0,1,2,3,4,5,6,7,8,9};

    //open
    fd = open("test", O_RDWR | O_CREAT, 0644);
    if (fd == -1)
    {
        printf("open file open_file_test failed!\n%s\n", strerror(errno));
        return -1;
    }
    printf("open open_file_test ok!\n");
    
    //write 1
    gets(str);
    len = write(fd, str, sizeof(str));
    fpos = lseek(fd, 0, SEEK_CUR);
    printf("current position:%ld %d\n", fpos, len);
    
    //write 2
    gets(str);
    len = write(fd, str, sizeof(str));
    fpos = lseek(fd, 0, SEEK_CUR);
    printf("current position:%ld %d\n", fpos, len);

    //write 3
    gets(str);
    len = write(fd, str, sizeof(str));
    fpos = lseek(fd, 0, SEEK_CUR);
    printf("current position:%ld %d\n", fpos, len);




    fpos = lseek(fd, 0, SEEK_CUR);
    if (fpos == -1)
    {
        printf("lseek failed!\n%s\n", strerror(errno));
        close(fd);
        return -1;
    }
    printf("file size:%ld\n", fpos);
    close(fd);
}
