#ifndef none_core_h
#define none_core_h


#define     NOL                 (4)
#define     SOL                 (8)
#define     NOI                 (2)
#define     NOO                 (1)

struct yuan_t {
    unsigned short  idx;
    unsigned char   in;             //0xff useless
    unsigned char   out;
    unsigned short  in1_idx; 
    unsigned short  in2_idx;
    unsigned short  sta1[SOL];
    unsigned short  sta2[SOL];
};
#define     SOY                 (sizeof(struct yuan_t))

#define     FILE_OF_MATRIX      "matrix.db"




#endif

