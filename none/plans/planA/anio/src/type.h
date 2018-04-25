#ifndef tpye_h

typedef unsigned char   u8;
typedef unsigned int    u32;
typedef signed long     s64;
typedef signed long     s32;


typedef enum{
    ACTIVE=1,
    NEGATIVE,
    INPUT,
    OUTPUT,
} unit_type;


typedef struct {
    unit_type   type;
    u32         id;
    s64         idata;
    s32         odata;
    s32         ip[ALL_NUM];
    s32         op[ALL_NUM];
} unit;






#endif
