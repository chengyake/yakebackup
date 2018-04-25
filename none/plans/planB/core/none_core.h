#ifndef none_core_h
#define none_core_h

enum none_system_state {
    INIT_STATE,
    SUSPEND_STATE,
    RESUME_STATE,
    MAX_STATE,
};

enum kernel_stage {
    STAGE_Y,
    STAGE_A,
    STAGE_K,
    STAGE_E,//end
};

struct kernel_parameter {
    uint32_t fb;
    uint32_t stack_inited;
    uint32_t stage;
};

extern uint64_t ticks;
extern struct kernel_parameter param;
extern enum none_system_state state;


extern int32_t init_kernel_platform();
extern int32_t resume_kernel();
extern int32_t suspend_kernel();
extern int32_t step_kernel();
extern int32_t sync_kernel();
extern int32_t set_random_data(uint32_t, uint32_t, long);
extern int32_t close_kernel_platform();




#endif

