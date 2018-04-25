#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
 

#include "sys.h"
#include "type.h"



pthread_t cmdthid;


/**************************cmd func*************************************/
void usr_help() {

    printf("USER HELP:\n");
    printf("----------------------------------------\n");
    printf("cmd\targ1\targ2\targ3\n");
    printf("help\n");
    printf("config\n");
    printf("----------------------------------------\n");
}

void get_config() {

    printf("USER CONFIG:\n");
    printf("----------------------------------------\n");
    printf("ALL_NUM\t:\t%d\nFLASH_PERIOD\t:\t%d\nTHRESHOLD_RATE\t:\t0x%x\n", ALL_NUM, FLASH_PERIOD, THRESHOLD_RATE);
    printf("----------------------------------------\n");
}

//create thread
void create() {

}

//start after load 
void start() {

}

//stop after a period
void suspend() {

}

//start from a new period
void resume() {

}

//stop immediately
void stop() {

}

void destroy() {

}

void saveas() {

}

void load() {

}






static void cmd_thread(void) {
    char *p;
    char cmdline[256];
    char cmd[64];
    char arg1[64];
    char arg2[64];
    char arg3[64];

    printf("This is chengyake's anio system\n");
    while(1) {
        printf("\n>>");

        memset(&cmdline, 0, sizeof(cmdline));
        memset(&arg1, 0, sizeof(arg1));
        memset(&arg2, 0, sizeof(arg2));
        memset(&arg3, 0, sizeof(arg3));

        gets(&cmdline[0]);
        
        p=NULL;
        p = strtok(cmdline, " ");
        if (p) sprintf(&cmd[0], "%s", p);
        p = strtok(NULL, " ");
        if (p) sprintf(&arg1[0], "%s", p);
        p = strtok(NULL, " ");
        if (p) sprintf(&arg2[0], "%s", p);
        p = strtok(NULL, " ");
        if (p) sprintf(&arg3[0], "%s", p);

        if(!strcmp(&cmd[0], "help")) {
        	usr_help();
        } else if(!strcmp(&cmd[0], "config")) {
        	get_config();
        } else {
        	printf("unkown cmd! \n");
            usr_help();
        }


    }
}

int add_cmd_thread()
{
    return pthread_create(&cmdthid,NULL,(void *) cmd_thread,NULL);
}




