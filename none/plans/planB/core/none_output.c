#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>

#include "none_core.h"
#include "none_output.h"
#include "none_matrix.h"
#include "user_algo.h"


int32_t init_output_module() {
    return 0;
}

int32_t output_algo_callback_by_core() {
    
    int i;

    user_output_algo_func(matrix);

    //user_feedback_algo_func(); //sending flag to OpenCl kernel replace this callback temporarily
    if (user_output_trigger_feedback_func() > 0) {
        param.fb = 1;
        param.stack_inited++;
    } else {
        param.fb = 0;
    }

    //clear output result
    for(i=0; i<YUAN_NUM_OF_OUTPUT; i++) {
        matrix[YUAN_NUM_OF_INPUT + YUAN_NUM_OF_NORMAL + i].sum = 0;
    }

    return 0;
}


int32_t close_output_module() {

    return 0;
}










