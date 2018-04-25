#ifndef matrix_h
#define matrix_h

#include <stdint.h>
#include "none_config.h"

extern struct yuan_t * matrix;

#define SIZE_OF_YUAN        (sizeof(struct yuan_t))
#define FILE_OF_MATRIX      "matrix_runtime.db"
#define SIZE_OF_MATRIX      (SIZE_OF_YUAN*YUAN_NUM_OF_ALL)

extern int32_t init_matrix_database();
extern int32_t make_matrix_snapshot();
extern int32_t restore_matrix_snapshot(char *file_name);
extern int32_t close_matrix_database();


#endif



