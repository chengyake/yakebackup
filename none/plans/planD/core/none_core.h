#ifndef none_core_h
#define none_core_h


extern struct matrix_info_t *param;

extern int32_t init_kernel_platform();
extern int32_t resume_kernel();
extern int32_t suspend_kernel();
extern int32_t step_kernel();
extern int32_t sync_kernel();
extern int32_t set_random_data(uint32_t, uint32_t, long);
extern int32_t close_kernel_platform();




#endif

