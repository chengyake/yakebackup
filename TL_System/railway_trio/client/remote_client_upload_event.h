#ifndef REMOTE_CLIENT_UPLOAD_EVENT_H
#define REMOTE_CLIENT_UPLOAD_EVENT_H



uint64_t get_upload_interval();

int check_and_upload_task();

int upload_task_process(struct frame_fmt *fhp);



#endif
