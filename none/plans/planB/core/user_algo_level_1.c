#include <stdio.h>

#include "user_algo.h"

#include "none_matrix.h"
#include "none_config.h"
#include "none_core.h"
#include "none_utils.h"



#define TRIGGER_INTERVAL_MAX      (10)
#define TRIGGER_INTERVAL_MIN      (5)

static int32_t trigger;



static int32_t input_value_at_ticks() {

    int32_t value, i;
    if(ticks%10 == 1) {
        value = YUAN_FULLHOLD;
    } else {
        value = 0;
    }
    return value;
}

static int32_t output_at_ticks() {

    if(ticks%10 == 2) {
        return 1;
    } else {
        return 0;
    }
}


int32_t user_input_algo_func(struct yuan_t *matrix) {
    int32_t i;

    for(i=0; i<YUAN_NUM_OF_INPUT; i++) {
        matrix[0].sum = input_value_at_ticks();
    }
   return 0;
}


int32_t user_output_algo_func(struct yuan_t *matrix) {
    
    int32_t i;
    long value = 0;
    
    for(i=0; i<YUAN_NUM_OF_OUTPUT; i++) {
        value+=matrix[YUAN_NUM_OF_INPUT+YUAN_NUM_OF_NORMAL].sum;
    }
    value/=YUAN_NUM_OF_OUTPUT;
    
    if(value > YUAN_THRESHOLD) {
        trigger = 1;
    } else {
        trigger = 0;
    }

    return 0;
}

int32_t user_output_trigger_feedback_func(struct yuan_t *matrix) {

    if(trigger && output_at_ticks() > 0) {
        return 1;
    }
    return 0;
}


int32_t user_feedback_algo_func(struct yuan_t *matrix) {


    return 0;
}







