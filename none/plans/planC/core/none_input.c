#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "none_input.h"
#include "none_matrix.h"
#include "user_algo.h"



int32_t init_input_module() {
    return 0;
}


int32_t input_algo_callback_by_core() {
    
    user_input_algo_func(matrix);
    return 0;
}


int32_t close_input_module() {

    return 0;
}




