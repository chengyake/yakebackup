#ifndef none_core_h
#define none_core_h


#define     SLOT                15
#define     NOA                 256
struct yuan_t {
    unsigned int idx;
    unsigned short sta[NOA][SLOT];
    unsigned char dis[NOA];
};
#define     NOI                 1
#define     NOO                 1
#define     NOM                 (32)
#define     SOY                 (sizeof(struct yuan_t))
#define     THESHOLD            127*1
#define     FILE_OF_MATRIX      "matrix.db"




#endif