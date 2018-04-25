/*
 *  CPU:     input and output process
 *  GPU:     internal yuan process and dispatch
 *
 *
 *                      CPU                 |                            GPU
 *                                          |
 *          input_data      Random_data---------------------------------------------------
 *              |                           |                                            |
 *              |1                          |                                            |
 *              |                           |                                            |
 *              v                           |                                            |
 *          yuan.sum(input)        acc      |                   if threshold trigger     v
 *   ------>yuan.sum(normal) -----------------> yuan.sum(n&o)-------------------------->yuan.dis--------------
 *   |      yuan.sum(output)        4       |                             5              ^                   |
 *   |          |                           |                                            |                   |
 *   |          |2                          |                                            |                   |
 *   |          |                           |                                            |                   |
 *   |          v           if feedback trigger, update                                  |                   |
 *   |      output_process <----------------------------> yuan.depth----------------------                   |
 *   |                                 3    |                                                                |
 *   |                                      |                                                                |
 *   |                                      |                               6                                |
 *   ---------------------------------------------------------------------------------------------------------
 *                                          |
 *                                          |
 *
 *
 *  Algo:
 *
 *          step 3:
 *              fifo:   1 random + N recent
 *
 *          step 4:
 *              accumulator
 *
 *          step 5:
 *              Random&depth, : yzx of N and precent, replace random by depth Em
 *
 *
 *
 *
 *  xianshixian,houyouhua
 *
 *
 *
 *
 */
//#include "none_config.h"
//#include "none_matrix.h"


#define YUAN_NUM_OF_ALL        (10)
#define YUAN_NUM_OF_INPUT      (1)
#define YUAN_NUM_OF_OUTPUT     (1)
#define YUAN_NUM_OF_NORMAL     (YUAN_NUM_OF_ALL - YUAN_NUM_OF_INPUT - YUAN_NUM_OF_OUTPUT)

#define YUAN_STACK_DEPTH    (10)


//propagation coefficient
#define YUAN_FB_TICKS       (10)
#define YUAN_PROPAGATION    (160)//% in percent
#define YUAN_FULLHOLD       (0x7FFFFFFF)
#define YUAN_THRESHOLD      ((long)YUAN_FULLHOLD*YUAN_NUM_OF_INPUT/YUAN_NUM_OF_ALL/100*YUAN_PROPAGATION)

enum kernel_stage {
    STAGE_Y,
    STAGE_A,
    STAGE_K,
    STAGE_E,//end
};

enum yuan_type {
    YUAN_TYPE_OF_INPUT,
    YUAN_TYPE_OF_NORMAL,
    YUAN_TYPE_OF_OUTPUT,
};

struct yuan_t {
    unsigned int    id;
    unsigned int    type;
    long     sum;
    long     den;//denominator
    int     trigger;
    int     dis[YUAN_NUM_OF_ALL]; //distribute
    int     depth[YUAN_NUM_OF_ALL][YUAN_STACK_DEPTH];//Em? random?
};

struct kernel_parameter {
    unsigned int fb;
    unsigned int stack_inited;
    unsigned int stage;
};



__kernel void process_matrix(__global void *in, __global void *random, __global void *parameter) {
    int i,j;
	int idx = get_global_id(0);
    __global struct yuan_t* yuan_p = (__global struct yuan_t *)in;
	__global int *rd = (__global int *)random; 
	__global struct kernel_parameter *param = (__global struct kernel_parameter *)parameter; 

    //fill dis
    if(param->stage == STAGE_K) {
        if(yuan_p[idx].type != YUAN_TYPE_OF_OUTPUT) {
            for(i=0; i<YUAN_NUM_OF_ALL; i++) {
                yuan_p[idx].dis[i] = rd[YUAN_NUM_OF_ALL*idx+i];
            }
            ////according to yzx, chose dis from depth&dis
            if(param->stack_inited > YUAN_STACK_DEPTH) {
                long Em=0;
                long yzx=0;
                for(i=0; i<YUAN_NUM_OF_ALL; i++) {
                    //get Em
                    for(j=0; j<YUAN_STACK_DEPTH; j++) {
                        Em += yuan_p[idx].depth[i][j];
                    }
                    Em=Em/YUAN_STACK_DEPTH;
                    //get yzx
                    for(j=0; j<YUAN_STACK_DEPTH; j++) {
                        yzx +=abs(Em - yuan_p[idx].depth[i][j]);
                    }
                    yzx=yzx*((unsigned long)1000)/YUAN_STACK_DEPTH/YUAN_FULLHOLD;

                    //chose dis
                    if(((unsigned int)rd[YUAN_NUM_OF_ALL*idx])%1000 > yzx) {
                        yuan_p[idx].dis[i] = Em;
                    }
                }
            }
            yuan_p[idx].den=0;
            for(i=0; i<YUAN_NUM_OF_ALL; i++) {
                yuan_p[idx].den+=abs(yuan_p[idx].dis[i]);
            }
        }
    }
    
    if(param->stage == STAGE_A) {
        yuan_p[idx].trigger = abs(yuan_p[idx].sum) >= YUAN_THRESHOLD ? 1 : 0;
    }

    //normal operation
    if(param->stage == STAGE_Y) {
        //feedback
        if(param->fb > 0 && yuan_p[idx].type != YUAN_TYPE_OF_OUTPUT) {
            for(i=0; i<YUAN_NUM_OF_ALL; i++) {
                for(j=0; j<YUAN_STACK_DEPTH-1; j++) {
                    yuan_p[idx].depth[i][j]=yuan_p[idx].depth[i][j+1];
                }
                yuan_p[idx].depth[i][j]=yuan_p[idx].dis[i];
            }
        }

        //get sum
        if(yuan_p[idx].type != YUAN_TYPE_OF_INPUT) {
            yuan_p[idx].sum = 0;
            for(i=0; i<YUAN_NUM_OF_ALL; i++) {
                yuan_p[idx].sum+=(yuan_p[i].trigger == 0 ? 0 : 
                        yuan_p[i].den==0?0:(yuan_p[i].dis[idx]*(long)YUAN_FULLHOLD/yuan_p[i].den));
            }
        }
    }
    
}


