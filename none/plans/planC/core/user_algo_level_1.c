#include <stdio.h>

#include "user_algo.h"

#include "none_matrix.h"
#include "none_config.h"
#include "none_core.h"
#include "none_utils.h"



#define TRIGGER_INTERVAL      (10)



unsigned char right[1000]={0};
unsigned char wrong[1000]={0};


static int32_t input_value_at_ticks() {

    int32_t value;
    if(param->ticks%TRIGGER_INTERVAL == 0) {
        value = YUAN_FULLHOLD;
    } else {
        value = 0;
    }
    return value;
}

static int32_t output_at_ticks() {

    if(param->ticks%TRIGGER_INTERVAL == 0) {
        return 1;
    } else {
        return 0;
    }
}


int32_t user_input_algo_func(struct yuan_t *matrix) {
    int32_t i;

    for(i=0; i<YUAN_NUM_OF_INPUT; i++) {
        matrix[YUAN_IDX_OF_INPUT_S+i].sum = input_value_at_ticks();
    }
   return 0;
}


int32_t user_output_algo_func(struct yuan_t *matrix) {
    
    int32_t i;
    char random;
    long value = 0;
    
    for(i=0; i<YUAN_NUM_OF_OUTPUT; i++) {
        value+=matrix[YUAN_IDX_OF_OUTPUT_S+i].sum;
    }
    value/=YUAN_NUM_OF_OUTPUT;

    get_random(&random, sizeof(char));

    if(random % 10 == 0) {
        if(value > YUAN_THRESHOLD) {
            if(output_at_ticks()) {
                param->fb_latest_co = 100;
            } else {
                param->fb_latest_co = -10;
            }
        } else {
            if(output_at_ticks()) {
                param->fb_latest_co = -100;
            } else {
                param->fb_latest_co = 10;
            }
        }

    }

#if 1
    if(output_at_ticks() ) {
        if (value > YUAN_THRESHOLD) {
            right[param->ticks/10%1000]=1;
        } else {
            right[param->ticks/10%1000]=0;
        }
    } else {
        if (value > YUAN_THRESHOLD) {
            wrong[param->ticks/10%1000]=1;
        } else {
            wrong[param->ticks/10%1000]=0;
        }
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

