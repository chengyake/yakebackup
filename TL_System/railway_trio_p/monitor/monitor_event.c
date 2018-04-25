#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>

#include "../core/core.h"
#include "monitor.h"

static struct process_manager_t process[PROCESS_NUM]= {
    /*path and bin      arg               retry pid*/
    {"/home/bin/core",            {NULL},   3, 0},   
    {"/home/bin/gps",             {NULL},   3, 0},   
    {"/home/bin/rild-0",          {NULL},   3, 0},   
    {"/home/bin/rild-1",          {NULL},   3, 0},   
    {"/home/bin/Trio_trace_0",    {NULL},   3, 0},   
    {"/home/bin/hal_Trio",        {NULL},   3, 0},   
    {"/home/bin/client",          {NULL},   3, 0},   
    
};

int reset_monitor_database() {
    int i;

    for(i=0; i<PROCESS_NUM; i++) {
        process[i].retry = 3;
    }

    return 0;
}


int setup_all_process() {
    int i;
    int ret = 0;

    for(i=0; i<PROCESS_NUM; i++) {

        if(process[i].pid > 0) {
            kill(process[i].pid, SIGKILL);
            wait(NULL);
            process[i].pid = 0;
        }

        process[i].pid = fork();
        if (process[i].pid  < 0)   
            syslog(LOG_ERR, "%s process error: pid = %d\n", __func__,  process[i].pid);
        else if (process[i].pid == 0) {
            syslog(LOG_INFO, "%s  start %s success: pid %d\n", __func__, process[i].bin, getpid());

            ret = execv(process[i].bin, process[i].arg);
            if(ret < 0) {
                syslog(LOG_ERR, "process execv error: return %d(%s)", errno, strerror(errno));
                exit(-1);
            }
        }
        sleep(1);

    }

    return 0;
}


int reboot_software() {
    int ret;
    time_t now;       
    struct tm *timenow; 

    time(&now);
    timenow = localtime(&now);
    syslog(LOG_INFO, "---------------------------------------\n");
    syslog(LOG_INFO, "software reboot at %s\n", asctime(timenow));
    syslog(LOG_INFO, "---------------------------------------\n");

    ret = setup_all_process();
    if(ret < 0) {
        syslog(LOG_ERR, "re-setup all process error: return %d\n", ret);
        reboot_hardware();
        return ret;
    }

    return ret;
}


#if 0
/* Perform a hard reset now.  */
#define RB_AUTOBOOT 0x01234567
/* Halt the system.  */
#define RB_HALT_SYSTEM  0xcdef0123
/* Enable reboot using Ctrl-Alt-Delete keystroke.  */
#define RB_ENABLE_CAD   0x89abcdef
/* Disable reboot using Ctrl-Alt-Delete keystroke.  */
#define RB_DISABLE_CAD  0
/* Stop system and switch power off if possible.  */
#define RB_POWER_OFF    0x4321fedc
#endif 
int reboot_hardware()
{
    time_t   now;       
    struct   tm     *timenow; 
    time(&now);
    timenow   =   localtime(&now);
    syslog(LOG_INFO, "---------------------------------------\n");
    syslog(LOG_INFO, "hardware reboot at %s\n", asctime(timenow));
    syslog(LOG_INFO, "---------------------------------------\n");
    closelog();
    sync(); 
    return reboot(RB_AUTOBOOT);
}

int register_client_module() {

    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "client register to core\n");
    cmd_rsp.host = IDX_USER_MONITOR;
    cmd_rsp.target = IDX_FRAMEWORK_CORE;
    cmd_rsp.cmd = CORE_MSG_REGISTER_CMD;
    cmd_rsp.len = 0;

    return write_monitor_server((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}

#if 0
int check_process(pid_t pid) {
    char file[256]={0};

    sprintf(file, "/proc/%d", pid);

    if(access(file, F_OK) != 0) { //don't exist
        return -1;
    }
    return 0;
}
#endif

int check_all_process() {
    int i;
    int ret;
    pid_t tmp;

    for(i=0; i<PROCESS_NUM; i++) {

        tmp=waitpid(process[i].pid, NULL, WNOHANG);
        if(tmp==process[i].pid) {
            process[i].pid = 0;
            syslog(LOG_ERR, "process %s don't exsit\n", process[i].bin);
            if(process[i].retry >0) {
                process[i].retry--;
                reboot_software();
                return -1;
            } else {
                reboot_hardware();
                return -2;
            }

        }
    }

    reset_monitor_database();

}

int kill_all_process() {
    int i;
    for(i=0; i<PROCESS_NUM; i++) {
        if(process[i].pid > 0) {
            kill(process[i].pid, SIGKILL);
        }
    }
}

int process_monitor_event(struct cmd_header *chp) {

    int ret;
    struct task_t *t;

    print_monitor_frame(chp);
    switch (chp->cmd) {
        case MONITOR_MSG_REBOOT_SW_CMD:
            setup_all_process();
            break;

        case MONITOR_MSG_REBOOT_HW_CMD:
            reboot_hardware();
            break;
        default:
            ret = 0;
            break;
    }
    return ret;
}


