#include <math.h>

#include "dct.h"

#define PI 3.14159

/*
const char dct_arg[] = {
    100,98,95,90,83,75,66,56,44,32,20,7,-7,-20,-32,-44,-56,-66,-75,-83,-90,-95,-98,-100,
    99,92,79,61,38,13,-13,-38,-61,-79,-92,-99,-99,-92,-79,-61,-38,-13,13,38,61,79,92,99,
    98,83,56,20,-20,-56,-83,-98,-98,-83,-56,-20,20,56,83,98,98,83,56,20,-20,-56,-83,-98,
    97,71,26,-26,-71,-97,-97,-71,-26,26,71,97,97,71,26,-26,-71,-97,-97,-71,-26,26,71,97,
    95,56,-7,-66,-98,-90,-44,20,75,100,83,32,-32,-83,-100,-75,-20,44,90,98,66,7,-56,-95,
    92,38,-38,-92,-92,-38,38,92,92,38,-38,-92,-92,-38,38,92,92,38,-38,-92,-92,-38,38,92,
    90,20,-66,-100,-56,32,95,83,7,-75,-98,-44,44,98,75,-7,-83,-95,-32,56,100,66,-20,-90,
    87,0,-87,-87,0,87,87,0,-87,-87,0,87,87,0,-87,-87,0,87,87,0,-87,-87,0,87,
    83,-20,-98,-56,56,98,20,-83,-83,20,98,56,-56,-98,-20,83,83,-20,-98,-56,56,98,20,-83,
    79,-38,-99,-13,92,61,-61,-92,13,99,38,-79,-79,38,99,13,-92,-61,61,92,-13,-99,-38,79,
    75,-56,-90,32,98,-7,-100,-20,95,44,-83,-66,66,83,-44,-95,20,100,7,-98,-32,90,56,-75,
    71,-71,-71,71,71,-71,-71,71,71,-71,-71,71,71,-71,-71,71,71,-71,-71,71,71,-71,-71,71,
};
*/

void get_dct(unsigned int *data, int *out) {

    int i, j;
    for(i=0; i<12; i++) {
        out[i]=0;
        for(j=1; j<=24; j++) {
            out[i] += data[j]*cos(PI*(j-0.5)*i/24);
        }
    }
}












