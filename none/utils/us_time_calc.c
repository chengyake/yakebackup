#include <stdio.h>
#include <stdlib.h> 
#include <sys/time.h>

#define TNUM 1000
int main(int argc, char **argv)
{
    struct timeval start,stop,diff;
	unsigned int *p = malloc(sizeof(unsigned int)*TNUM);
    gettimeofday(&start,0);
    test(p, 0);
    test(p, 2);
    gettimeofday(&stop,0);
    timeval_subtract(&diff,&start,&stop);
    printf("总计用时:%d 微秒\n",diff.tv_usec);
    free(p);
}

int test(unsigned int *p, unsigned int n)
{
	unsigned long i=0;
	for(i=0; i<TNUM; i++)
    {
        p[i]=n+i;
    }
}


int timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y)
{
    int nsec;

    if ( x->tv_sec>y->tv_sec )
        return -1;

    if ( (x->tv_sec==y->tv_sec) && (x->tv_usec>y->tv_usec) )
        return -1;

    result->tv_sec = ( y->tv_sec-x->tv_sec );
    result->tv_usec = ( y->tv_usec-x->tv_usec );

    if (result->tv_usec<0)
    {
        result->tv_sec--;
        result->tv_usec+=1000000;
    }

    return 0;
}


