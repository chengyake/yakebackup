#include <stdio.h>

#include "wav.h"
#include "vad.h"
#include "fft.h"
#include "mel_filter.h"

#define 

static int vad_state = VAD_IDLE;

int main() {


    int idx=0, ret;
    while(idx<427) {
        short org_data[256]={0};
        unsigned int power_data[256] = {0};
        unsigned int mel_data[24] = {0};
        int mfcc_data[12] = {0};

        get_wav_data(idx, org_data, 256);

        ret = VAD(org_data, 256);
        if(ret != vad_state && ret != 2) {
            //printf("%d state:%d\n", idx, ret);
            vad_state = ret;
        }

        if(vad_state == 1) {
            pre_emphasis_and_hamming(idx, org_data, 256);

            get_power_spectrum(org_data, power_data, 256);//fft

            mel_filter(power_data, mel_data);

            get_dct(mel_data, mfcc_data);

        }


        idx++;

    }

}









