#include <stdio.h>
#include <math.h>

#include "fft.h"

#define NUM 256 //需要处理的离散信号的总个数
#define PI 3.1415926535897932384626433832795028841971  


typedef struct complex
{  
    float real;       
    float imag;    
} complex;  




//精度0.0001弧度  
void conjugate_complex(int n,complex in[],complex out[])  
{  
    int i = 0;  
    for(i=0;i<n;i++)  
    {  
        out[i].imag = -in[i].imag;  
        out[i].real = in[i].real;  
    }   
}  

void c_abs(complex f[],float out[],int n)  
{  
    int i = 0;  
    float t;  
    for(i=0;i<n;i++)  
    {  
        t = f[i].real * f[i].real + f[i].imag * f[i].imag;  
        out[i] = sqrt(t);  
    }   
}  


void c_plus(complex a,complex b,complex *c)  
{  
    c->real = a.real + b.real;  
    c->imag = a.imag + b.imag;  
}  

void c_sub(complex a,complex b,complex *c)  
{  
    c->real = a.real - b.real;  
    c->imag = a.imag - b.imag;   
}  

void c_mul(complex a,complex b,complex *c)  
{  
    c->real = a.real * b.real - a.imag * b.imag;  
    c->imag = a.real * b.imag + a.imag * b.real;     
}  

void c_div(complex a,complex b,complex *c)  
{  
    c->real = (a.real * b.real + a.imag * b.imag)/(b.real * b.real +b.imag * b.imag);  
    c->imag = (a.imag * b.real - a.real * b.imag)/(b.real * b.real +b.imag * b.imag);  
}  

#define SWAP(a,b)  tempr=(a);(a)=(b);(b)=tempr  

void Wn_i(int n,int i,complex *Wn,char flag)  
{
    Wn->real = cos(2*PI*i/n);  
    if(flag == 1)  
        Wn->imag = -sin(2*PI*i/n);  
    else if(flag == 0)  
        Wn->imag = -sin(2*PI*i/n);  
}

void _fft(int N,complex f[])  
{  
    complex t,wn;//中间变量  
    int i,j,k,m,n,l,r,M;  
    int la,lb,lc;  
    /*----计算分解的级数M=log2(N)----*/  
    for(i=N,M=1;(i=i/2)!=1;M++);   
    /*----按照倒位序重新排列原信号----*/  
    for(i=1,j=N/2;i<=N-2;i++)  
    {  
        if(i<j)  
        {  
            t=f[j];  
            f[j]=f[i];  
            f[i]=t;  
        }  
        k=N/2;  
        while(k<=j)  
        {  
            j=j-k;  
            k=k/2;  
        }  
        j=j+k;  
    }  

    /*----FFT算法----*/  
    for(m=1;m<=M;m++)  
    {  
        la=pow(2,m); //la=2^m代表第m级每个分组所含节点数       
        lb=la/2;    //lb代表第m级每个分组所含碟形单元数  
        //同时它也表示每个碟形单元上下节点之间的距离  
        /*----碟形运算----*/  
        for(l=1;l<=lb;l++)  
        {  
            r=(l-1)*pow(2,M-m);     
            for(n=l-1;n<N-1;n=n+la) //遍历每个分组，分组总数为N/la  
            {  
                lc=n+lb;  //n,lc分别代表一个碟形单元的上、下节点编号       
                Wn_i(N,r,&wn,1);//wn=Wnr  
                c_mul(f[lc],wn,&t);//t = f[lc] * wn复数运算  
                c_sub(f[n],t,&(f[lc]));//f[lc] = f[n] - f[lc] * Wnr  
                c_plus(f[n],t,&(f[n]));//f[n] = f[n] + f[lc] * Wnr  
            }  
        }  
    }  
}  

//傅里叶逆变换  
void ifft(int N,complex f[])  
{  
    int i=0;  
    conjugate_complex(N,f,f);  
    _fft(N,f);  
    conjugate_complex(N,f,f);  
    for(i=0;i<N;i++)  
    {  
        f[i].imag = (f[i].imag)/N;  
        f[i].real = (f[i].real)/N;  
    }  
}  

void fft(short *data, int n) {
    

    int i;
    complex cdata[NUM];
    for(i=0; i<NUM; i++) {
        cdata[i].real = data[i];
        cdata[i].imag = 0;
    }    
    _fft(n, cdata);

    for(i=0; i<NUM; i++) {
        data[i] = sqrt(cdata[i].imag*cdata[i].imag + cdata[i].real*cdata[i].real);
    }
}

void get_power_spectrum(short *data, unsigned int *out, int n)  {

    int i;
    complex cdata[NUM];
    for(i=0; i<NUM; i++) {
        cdata[i].real = data[i];
        cdata[i].imag = 0;
    }    
    _fft(n, cdata);

    for(i=0; i<NUM; i++) {
        data[i] = cdata[i].imag*cdata[i].imag + cdata[i].real*cdata[i].real;
    }
}



/*
int main() {

    int i;
    complex data[NUM];
    for(i=0; i<NUM; i++) {
        data[i].real = y[i]; //x[i];
        data[i].imag = 0;
    }
    _fft(NUM, data);


    for(i=0; i<NUM; i++) {
        //printf("%f\n", data[i].real);
        //printf("%f\n", data[i].imag);
        printf("%f\n", sqrt(data[i].imag*data[i].imag + data[i].real*data[i].real));
    }

    return 0;

}
*/



