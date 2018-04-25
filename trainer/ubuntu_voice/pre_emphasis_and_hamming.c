#include "pre_emphasis_and_hamming.h"





//Y(x) = L(x) - 0.95*L(x-1)




int pre_emphasis_and_hamming(int idx, unsigned short *data, int length) {

    int i;
    short tmp=0, last;
    static short frame_last;
    last = frame_last;
    frame_last = data[length/2-1];

    tmp = data[0] - last*PRE_EMPHASIS_CO;
    last = data[0];
    data[0] = tmp;
    data[0] = data[0]*hamming[0];

    for(i=1; i<length; i++) {
        tmp = data[i] - last*PRE_EMPHASIS_CO;
        last = data[i];
        data[i] = tmp;
        data[i] = data[i]*hamming[i]/1000;
    }

    return 0;
}














