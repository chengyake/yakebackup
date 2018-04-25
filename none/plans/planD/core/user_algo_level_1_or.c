#include <stdio.h>
#include <stdlib.h>

#include "user_algo.h"

#include "none_matrix.h"
#include "none_config.h"
#include "none_core.h"
#include "none_utils.h"

#define TRIGGER_INTERVAL      (YUAN_FEEDBACK_TICKS)


unsigned char right[1000]={0};
unsigned char wrong[1000]={0};


void exit_for_debug(int32_t c) {

    static int ex = -1;
    static int ex_ticks = -1;

    if(c > 900 && ex == -1) {
        ex = 0;
        ex_ticks=param->ticks + 1000*1000;
    }
    if(ex == 0 && ex_ticks < param->ticks) {
        if(c > 900) {
            printf("\n\tTrigger exit condition!!\n");
            exit(0);
        } else {
            ex == -1;
        }
    }

}


void write_debug_to_file() {
    char args[64]={0};

    sprintf(args, "echo %d %d >> cw.txt", user_get_correct_percent(), user_get_wrong_percent());
    system(args);
}



static int32_t input_value_at_ticks() {

    int32_t value;
    if(param->ticks%TRIGGER_INTERVAL == 0) {
        value = YUAN_FULLHOLD;
    } else {
        value = 0;
    }
    return value;
}

static unsigned long fb_ticks=0;

static int32_t output_at_ticks() {
    if(param->ticks - fb_ticks <= 7) {
        return 1;
    } else {
        return 0;
    }
}

int32_t user_input_algo_func(struct yuan_t *matrix) {
    int32_t i, r;

    get_random((uint8_t *)&r, sizeof(r));
    for(i=0; i<YUAN_NUM_OF_INPUT; i++) {
        matrix[YUAN_IDX_OF_INPUT_S+i].sum = r&(0x00000001<<i) ? input_value_at_ticks() : 0;
    }

    if(!((r&0x00000001) == (r&0x00000002))) {
        fb_ticks = param->ticks;
    }

   return 0;
}

static unsigned long r_idx=0;
int32_t user_output_algo_func(struct yuan_t *matrix) {
    
    int32_t i;
    long value = 0;
    
    for(i=0; i<YUAN_NUM_OF_OUTPUT; i++) {
        value+=matrix[YUAN_IDX_OF_OUTPUT_S+i].sum;
    }
    value/=YUAN_NUM_OF_OUTPUT;

    int c = user_get_correct_percent();
    int w = user_get_wrong_percent();

    //clear co
    param->fb_latest_co = 0;

#if 1
    if(output_at_ticks()) {
        if (value >= YUAN_THRESHOLD) {
            if(right[r_idx%1000] == 0) {
                param->fb_latest_co = (long)10*(1001-c)*(1001-c)/(w+1);
                param->fb_latest_co = param->fb_latest_co > 50 ? 50 : param->fb_latest_co;
                right[r_idx%1000]=1;
            }
        }
        r_idx++;
        right[r_idx%1000]=0;
        wrong[param->ticks/10%1000+2]=0;
    } else {
        if (value >= YUAN_THRESHOLD) {
            if(wrong[param->ticks/10%1000] == 0) {
                param->fb_latest_co = -w-1 < -50 ? -50 : -w-1;
                wrong[param->ticks/10%1000]=1;
            }
            wrong[param->ticks/10%1000+1]=0;
        }
    }

    if(param->ticks%10000==0) {
        write_debug_to_file();
    }
#endif


    return 0;
}


int32_t user_get_correct_percent() {

    int ret=0, i;
    for(i=0; i<1000; i++) {
       ret += right[i];
    }
    return ret;
}

int32_t user_get_wrong_percent() {

    int ret=0, i;
    for(i=0; i<1000; i++) {
       ret += wrong[i];
    }
    return ret;
}

