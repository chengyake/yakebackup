#ifndef none_core_h
#define none_core_h


#define     NOA                 (64)
#define     NOI                 (2)
#define     NOO                 (4)

struct yuan_t {
    unsigned short  idx;
    unsigned char   in;             //0xff useless
    unsigned char   out;
    unsigned short  in1_idx; 
    unsigned short  in2_idx;
    unsigned short  sta1[NOA-NOO];
    unsigned short  sta2[NOA-NOO];
};
#define     SOY                 (sizeof(struct yuan_t))

#define     FILE_OF_MATRIX      "matrix.db"




#endif

