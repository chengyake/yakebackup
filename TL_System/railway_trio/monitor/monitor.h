#ifndef MONITOR_H
#define MONITOR_H


#define CHECK_PROCESS_INTERVAL  (30*1)

#define PROCESS_NUM (7)


struct process_manager_t {
    char bin[32];
    char *arg[4];
    uint32_t retry;
    pid_t pid;
};








#endif
