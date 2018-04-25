
#include "/home/chengyake/none/plans/d_plan/core/none_config.h"


__kernel void process_matrix(__global void *in, __global void *random, __global void *parameter) {
    int i,j;
    int idx = get_global_id(0);
    __global struct yuan_t* matrix = (__global struct yuan_t *)in;
    __global unsigned int *random_p = (__global unsigned int *)random; 
    __global struct matrix_info_t *param = (__global struct matrix_info_t *)parameter; 



    //acc dis
    if(param->stage == STAGE_A) {
        for(i=0; i<YUAN_NUM_OF_MATRIX; i++) {
            int ge = 0;
            unsigned int sum = 1;
            for(j=0; j<YUAN_STATISTICS_NUM; j++) {
                sum += abs(matrix[idx].statistics[i][j]);
            }
            sum = random_p[YUAN_NUM_OF_MATRIX*idx+i] % sum;
            for(j=0; j<YUAN_STATISTICS_NUM; j++) {
                if(sum > abs(matrix[idx].statistics[i][j])) {
                    ge++;
                    sum -= abs(matrix[idx].statistics[i][j]);
                } else {
                    break;
                }
            }
            matrix[idx].proportion[i] = (ge - (YUAN_STATISTICS_NUM/2)) * YUAN_STATISTICS_GRANULARITY + (sum % YUAN_STATISTICS_GRANULARITY);
        }
        //average
#if 0
        int all = 1;
        for(i=0; i<YUAN_NUM_OF_MATRIX; i++) {
            all += abs(matrix[idx].proportion[i]);
        }

        for(i=0; i<YUAN_NUM_OF_MATRIX; i++) {
            matrix[idx].proportion[i] = (YUAN_FULLHOLD) * matrix[idx].proportion[i]/all;
        }
#endif
    }

    //is trigger?
    if(param->stage == STAGE_B) {
        if(matrix[idx].type == YUAN_TYPE_OF_INPUT || matrix[idx].type == YUAN_TYPE_OF_NORMAL) {
            matrix[idx].trigger= abs(matrix[idx].sum) > YUAN_THRESHOLD ? 1 : 0;
        }
    }

    //get sum
    if(param->stage == STAGE_C ) {
        if(matrix[idx].type > YUAN_TYPE_OF_INPUT) {
            matrix[idx].sum = 0;
            for(i=0; i<YUAN_NUM_OF_MATRIX; i++) {
                if(matrix[i].trigger != 0) {
                    matrix[idx].sum += matrix[i].proportion[idx];
                }
            }
        }
    }



    //feedback
    if(param->stage == STAGE_D) {
        if(matrix[idx].type == YUAN_TYPE_OF_INPUT || matrix[idx].type == YUAN_TYPE_OF_NORMAL) {
            int ge, tmp;
            for(i=0; i<YUAN_NUM_OF_MATRIX; i++) {
                
                unsigned short max = 0; //, min = YUAN_FULLHOLD;
                for(j=0; j<YUAN_STATISTICS_NUM; j++) {
                    max = matrix[idx].statistics[i][j] > max ? matrix[idx].statistics[i][j] : max ;
                }
                
                if(max > 0xFFFF-1000) {
                    for(j=0; j<YUAN_STATISTICS_NUM; j++) {
                        matrix[idx].statistics[i][j] = matrix[idx].statistics[i][j]/2+10;
                    }
                }
                
                if(max < 0xFFFF/3) {
                    for(j=0; j<YUAN_STATISTICS_NUM; j++) {
                        matrix[idx].statistics[i][j] = matrix[idx].statistics[i][j]*2;
                    }
                }

                //ge = (matrix[idx].proportion[i]+2^15)/YUAN_STATISTICS_GRANULARITY;
                ge = (matrix[idx].proportion[i]+(int)32768)/YUAN_STATISTICS_GRANULARITY;
                tmp = matrix[idx].statistics[i][ge] + param->fb_latest_co;
                if(tmp <= 10) {
                    matrix[idx].statistics[i][ge] = 10;
                } else if(tmp > 0xFFFF) {
                    matrix[idx].statistics[i][ge] = 0xFFFE;
                } else {
                    matrix[idx].statistics[i][ge] = tmp;
                }
            }
        }
    }
}


