#ifndef core_h
#define core_h


//machine args



//chengyake define
#define YUAN_NUM_OF_ALL        (10)
#define YUAN_NUM_OF_INPUT      (1)
#define YUAN_NUM_OF_OUTPUT     (1)
#define YUAN_NUM_OF_NORMAL     (YUAN_NUM_OF_ALL - YUAN_NUM_OF_INPUT - YUAN_NUM_OF_OUTPUT)

#define YUAN_STACK_DEPTH    (10)


//propagation coefficient
#define YUAN_FB_TICKS       (10)
#define YUAN_PROPAGATION    (160)//% in percent
#define YUAN_FULLHOLD       (0x7FFFFFFF)
#define YUAN_THRESHOLD      ((long)YUAN_FULLHOLD*YUAN_NUM_OF_INPUT/YUAN_NUM_OF_ALL/100*YUAN_PROPAGATION)













#endif
