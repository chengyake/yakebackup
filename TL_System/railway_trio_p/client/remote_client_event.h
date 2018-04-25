#ifndef remote_client_event_h
#define remote_client_event_h




uint64_t get_remote_interval(uint64_t pass);

int check_and_process_remote_task();

int process_remote_event(struct frame_fmt *fhp);


#endif
