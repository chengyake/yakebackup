#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>


static int fd;

void init_random ()
{
    fd = open ("/dev/urandom", O_RDONLY);
    if (fd <= 0)
    {
        printf("open /dev/urandom error\n");
    }
}

unsigned int get_random ()
{
    unsigned int n = 0;
    read (fd, &n, sizeof (n));
    return n;
}

void close_random ()
{
    close (fd);
}


#if 0
int main()
{   
    int i;
    init_random();
    for(i=0; i<100; i++)
    {
        printf("%x\n",get_random());
    }
    release_random();

    return 0;
}
#endif
