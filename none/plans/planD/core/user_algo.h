#ifndef USER_ALGO_H
#define USER_ALGO_H

#include "none_matrix.h"


extern int init_output_statistics(); //temp for debug
extern int get_statistics_debug();

extern int user_input_algo_func(struct yuan_t *);
extern int user_output_algo_func();
extern int user_get_correct_percent();
extern int user_get_wrong_percent();



#endif