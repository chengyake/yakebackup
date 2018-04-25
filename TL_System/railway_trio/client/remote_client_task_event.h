#ifndef remote_client_task_event_h
#define remote_client_task_event_h

#include "client_task_queue.h"



int task_process(struct frame_fmt *fhp);

int check_stop_task();
void stop_task_rsp(struct task_t *tth, int flag, int stop_manual);

#endif
