#ifndef none_core_h
#define none_core_h

#define NOI 2
#define NOO 4
#define NOA 128
struct cell_t {
    unsigned short idx;
    unsigned char sta[NOA][41];
    unsigned char dis[NOA];
};
#define SOC (sizeof(struct cell_t))


#define     FILE_OF_MATRIX      "digital_matrix.db"


#endif
