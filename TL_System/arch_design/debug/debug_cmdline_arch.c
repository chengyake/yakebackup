#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "debug_cmd.h"

#define CMD_BUFFER_SIZE (256)
//#define CMD_TYPE_NUM (64)

int cmd_loop=1;

struct func_list_t {
    const char *name;
    void (*usage)();
    int (*func)(char *arg1, char *arg2, char *arg3, char *arg4);
    char* ((*ecode)(int code));
};

struct func_list_t *get_cmd_handler(const char *name);


void help_usage();
int help_func(char *arg1, char *arg2, char *arg3, char *arg4);
char* default_ecode(int code);

static struct func_list_t func_list[] = {
	/*name***usage*******body********ecode*******/
    {"help", help_usage, help_func, default_ecode},
    {"time", time_usage, time_func, default_ecode},
    {"exit", exit_usage, exit_func, NULL},
};


int unkown_tips() {

    printf("unkown cmd or cmd args, please run >>help\n");

}

int cmd_header() {

    printf("\n");
    printf("system version 0.1\n");
    printf("\n");

}

void help_usage() {
    int i;
    printf("Debug CMD info:\n");
    for(i=0; i<sizeof(func_list)/sizeof(struct func_list_t); i++) {
    	printf("%s ", func_list[i].name);
    }
    printf("\n\n");
    printf("You can help xxx for more help:\n");
}

int help_func(char *arg1, char *arg2, char *arg3, char *arg4) {

    struct func_list_t *handler;

    if(arg1==NULL) {
        help_usage();
        return 0;
    } else {
        handler = get_cmd_handler(arg1);
        if(handler!= NULL && handler->usage != NULL) {
        	handler->usage();
        	return 0;
        }
    }
    
    printf("Usage of %s not found\n", arg1);
    return 0;

}

char* default_ecode(int code) {
	    if(code > 0) {
	    	return NULL;
        }
        return strerror(-code);
}


struct func_list_t *get_cmd_handler(const char *name) {
    
    int i;
    struct func_list_t *ret_p = NULL;

    for(i=0; i<sizeof(func_list)/sizeof(struct func_list_t); i++) {
        if(strcmp(name, func_list[i].name) == 0) {
            ret_p = &func_list[i];
            break;
        }
    }

    return ret_p;

}



int setup_cmd_loop(void) {

    int i;
    int ret=0;
    char cmd_line_buffer[CMD_BUFFER_SIZE]={0};
    char *cmd_line_p = cmd_line_buffer;
    char *args[5], *arg_p;
    struct func_list_t *handler;

    //output cmdline header
    cmd_header();

    while(cmd_loop) {

        handler = NULL;
        cmd_line_p = cmd_line_buffer;
        memset(args, 0, sizeof(args));
        memset(cmd_line_p, 0, CMD_BUFFER_SIZE);

        printf(">>");
        fgets(cmd_line_p, CMD_BUFFER_SIZE, stdin);

        for(i=0; (arg_p=strtok(cmd_line_p, " ")) != NULL; i++) {
            cmd_line_p=NULL;
            args[i]=arg_p;
        }
        args[i-1][strlen(args[i-1])-1]=0;

        if(args[0] == NULL || strcmp(args[0], "") == 0) {
            printf("No input detected, Please try >>help\n");
            continue;//for(;;)
        }

        handler = get_cmd_handler(args[0]);
        if(handler == NULL) {
            printf("No cmd detected, Please try >>help\n");
            continue;//for(;;)
        }

        if(handler->func == NULL) {
            printf("you should never get here: CODE ERROR in debug proccess\n");
        }
        ret = handler->func(args[1], args[2], args[3], args[4]); 
        if(ret<0) {

            if(handler->usage != NULL) {
                handler->usage();
            }
            if(handler->ecode != NULL) {
                printf("\nERROR %d: %s\n", ret, handler->ecode(ret));
            }

        }

        printf("\n");
    }//exit while(cmd_loop)

    printf("debug cmd-loop exit: flag = 0\n");
    return 0;
}


