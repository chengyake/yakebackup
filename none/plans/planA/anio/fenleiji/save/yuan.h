#ifndef YUAN_H
#define YUAN_H

#include <stdio.h>

//custom define
#define DBG 1

#define SA_NUM  (5)
#define SN_NUM  (5)
#define I_NUM   (10)
#define A_NUM   (100)
#define N_NUM   (100)
#define O_NUM   (10)
#define H_NUM   (10)

#define CTRL_FLASH_HZ   (10000)
#define OUTPUT      (0x7FFFFFFF)
#define THRESHOLD       (OUTPUT/100)

/*
some define
c/f = 1/10000
i/f = 1/1000
b/f = 1/100

input s = full
output&feedback = 4*output
*/


//I/O feedback action

//custom define end

//Type MAP
#define YUAN_START (0)
#define SA_START  (YUAN_START)
#define SA_END    (SA_NUM-1)
#define SN_START  (SA_END+1)
#define SN_END    (SN_START+SN_NUM-1)
#define I_START   (SN_END+1)
#define I_END     (I_START+I_NUM-1)
#define A_START   (I_END+1)
#define A_END     (A_START+A_NUM-1)
#define N_START   (A_END+1)
#define N_END     (N_START+N_NUM-1)
#define O_START   (N_END+1)
#define O_END     (O_START+O_NUM-1)
#define H_START   (O_END+1)
#define H_END     (H_START+H_NUM-1)
#define YUAN_END  (H_END)
#define YUAN_NUM  (YUAN_END+1)

enum system_status
{
    SYS_INIT,
    SYS_RUN,
    SYS_SUS,
    SYS_STOP,
}; 

/********************************************************/
enum yuan_type
{
	SA = 1,
	SN,
	I,
	A,
	N,
	O,
	H,
};

struct feather
{
    int pdata;
    unsigned int pc;
    unsigned int ps[100];
    unsigned int pbs[100];
};

struct yuan
{
	unsigned int id;
	enum yuan_type type;
    int idata;
    int odata;
    struct feather *iwing;
    struct feather *owing;
};

#endif

