#ifndef REMOTE_CLIENT_STATUS_AND_POSITION
#define REMOTE_CLIENT_STATUS_AND_POSITION



uint64_t get_report_interval();
void check_and_report_status_and_position_info();
int status_task_process(struct frame_fmt *fhp);

extern int heart_beat;


#endif
