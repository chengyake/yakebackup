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
    
    user_output_algo_func(matrix);
    return 0;
}


int32_t close_output_module() {

    return 0;
}










