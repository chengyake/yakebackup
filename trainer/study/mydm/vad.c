#include <stdio.h>
#include "vad.h"



#define to_active_threshold 200
#define at_zero_threshold 100
#define zero_cross_rate 1


static int vad_state = VAD_IDLE;


int VAD(short *data, int len) {

    int i;
    int sum=0, zero_num=0;
    int energy_flag=0, zero_flag=0;
    //Short-term Energy  2000
    for(i=0; i<len; i++) {
        sum+=(data[i]>0?data[i]:-data[i]);
    }
    
    sum/= len;
    if(sum > to_active_threshold) {
        energy_flag = 1;
    }


    //Short-time Zero-crossing Rate
    int switch_flag=0;
    for(i=0; i<len; i++) {
       if(data[i] > 0 && data[i]-at_zero_threshold > 0) {
            if(switch_flag != 1) {
                zero_num++;
                switch_flag=1;
            }
       }
       if(data[i] < 0 && data[i]+at_zero_threshold < 0) {
            if(switch_flag != -1) {
                zero_num++;
                switch_flag=-1;
            }
       }
    }
    if(zero_num > zero_cross_rate) {
        zero_flag =1;
    }


    //printf("%d, %d\n", sum, zero_num);
    if(vad_state == VAD_IDLE && energy_flag == 1 && zero_flag == 1) {
        vad_state = VAD_ACTIVE;
        return vad_state;
    }

    if(vad_state == VAD_ACTIVE && energy_flag == 0 && zero_flag == 0) {
        vad_state = VAD_IDLE;
        return vad_state;
    }

    return 2;


}



