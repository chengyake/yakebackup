#ifndef none_core_h
#define none_core_h

#define NOI 300
#define NOO 300
#define NOA (1024*2)
struct cell_t {
    unsigned short idx;
    unsigned char sta[NOA][41];
    unsigned char dis[NOA];
};
#define SOC (sizeof(struct cell_t))
struct input_table_t {
    unsigned char code[16];
    float avg[1556];
};



#define     FILE_OF_MATRIX      "matrix.db"


#endif
