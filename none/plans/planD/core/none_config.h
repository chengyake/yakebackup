#ifndef core_h_cl
#define core_h_cl

//machine limit
#define YUAN_NUM_MAX    (65536)//16bit


//chengyake define
#define YUAN_NUM_OF_ALL                 (11)
#define YUAN_NUM_OF_INFO                (1)
#define YUAN_NUM_OF_INPUT               (2)
#define YUAN_NUM_OF_OUTPUT              (1)
#define YUAN_NUM_OF_NORMAL              (YUAN_NUM_OF_ALL - YUAN_NUM_OF_INFO - YUAN_NUM_OF_INPUT - YUAN_NUM_OF_OUTPUT)
#define YUAN_NUM_OF_MATRIX              (YUAN_NUM_OF_ALL - YUAN_NUM_OF_INFO)

#define YUAN_IDX_OF_ALL_S               (0)
#define YUAN_IDX_OF_ALL_E               (YUAN_NUM_OF_ALL-1)

#define YUAN_IDX_OF_INFO_S              (0)
#define YUAN_IDX_OF_INFO_E              (YUAN_IDX_OF_INFO_S+YUAN_NUM_OF_INFO-1)
#define YUAN_IDX_OF_INPUT_S             (YUAN_IDX_OF_INFO_E+1)
#define YUAN_IDX_OF_INPUT_E             (YUAN_IDX_OF_INPUT_S+YUAN_NUM_OF_INPUT-1)
#define YUAN_IDX_OF_NORMAL_S            (YUAN_IDX_OF_INPUT_E+1)
#define YUAN_IDX_OF_NORMAL_E            (YUAN_IDX_OF_NORMAL_S+YUAN_NUM_OF_NORMAL-1)
#define YUAN_IDX_OF_OUTPUT_S            (YUAN_IDX_OF_NORMAL_E+1)
#define YUAN_IDX_OF_OUTPUT_E            (YUAN_IDX_OF_OUTPUT_S+YUAN_NUM_OF_OUTPUT-1)



//propagation coefficient
#define YUAN_FEEDBACK_TICKS             (10)
#define YUAN_PROPAGATION                (160)//% in percent
#define YUAN_FULLHOLD                   (0x7FFF)
#define YUAN_THRESHOLD                  (YUAN_FULLHOLD*15/16)
//(YUAN_FULLHOLD*YUAN_NUM_OF_INPUT/YUAN_NUM_OF_ALL/100*YUAN_PROPAGATION)


#define YUAN_STATISTICS_NUM             (32)
//#define YUAN_STATISTICS_GRANULARITY     ((2^16)/YUAN_STATISTICS_NUM)
#define YUAN_STATISTICS_GRANULARITY     (2048)





enum none_system_state {
    INIT_STATE,
    SUSPEND_STATE,
    RESUME_STATE,
    MAX_STATE,
};

enum yuan_type {
    YUAN_TYPE_OF_INFO,
    YUAN_TYPE_OF_INPUT,
    YUAN_TYPE_OF_NORMAL,
    YUAN_TYPE_OF_OUTPUT,
};


enum kernel_stage {

    STAGE_A,//acc dis
    //stage input first
    STAGE_B,//ids->sum
    //stage output and feedback
    STAGE_C,//sum->trigger
    STAGE_D,//feedback once
};

struct matrix_info_t {
    unsigned long ticks;
    unsigned int  stage;
    unsigned int  system_state;
    int  fb_latest_co;
};

struct yuan_t {
    unsigned short id;
    unsigned short type;
    int sum;
    unsigned short trigger;
    short proportion[YUAN_NUM_OF_ALL]; //distribute
    unsigned short statistics[YUAN_NUM_OF_ALL][YUAN_STATISTICS_NUM];//inited with half of uint16
};




#endif



