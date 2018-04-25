#ifndef vad_h
#define vad_h

enum vad_state {
    VAD_IDLE=0,
    VAD_ACTIVE,
};


int VAD(short *data, int len);


#endif
