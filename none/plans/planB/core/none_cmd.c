#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "none_config.h"
#include "none_cmd.h"
#include "none_matrix.h"
#include "none_debug.h"
#include "none_core.h"

#define CMD_BUFFER_SIZE (256)

static int32_t cmd_loop=1;

extern uint64_t ticks;


static long org_dis_to_percent(int32_t org, long den) {
    
    if(den == 0) return 0;

    return ((long)org)*YUAN_FULLHOLD/den * 100 / YUAN_THRESHOLD;
}

static long org_dis_from_percent(int32_t per, long den) {

    return ((long)per)*YUAN_THRESHOLD/100*den/YUAN_FULLHOLD;
    
}

static int32_t print_matrix_info() {
    
    printf("\tNone System ticks:%lu\n",  ticks);
    printf("\tpropagation coefficient:%u\n", YUAN_PROPAGATION);
    printf("\tConvergence coefficient:%u\n", 10);
    printf("\tAll:%u\t\tInput:%u\t\tNormal:%u\t\tOutput:%u\n", YUAN_NUM_OF_ALL, YUAN_NUM_OF_INPUT, YUAN_NUM_OF_NORMAL, YUAN_NUM_OF_OUTPUT);
    return 0;
}


static long print_yzx(struct yuan_t *yuan, uint32_t d) {
    
    int j;
    long Em=0;
    long yzx=0;

    if(param.stack_inited < YUAN_STACK_DEPTH) {
        return 0;
    }
    //get Em
    for(j=0; j<YUAN_STACK_DEPTH; j++) {
        Em += yuan->depth[d][j];
    }
    Em=Em/YUAN_STACK_DEPTH;
    //get yzx
    for(j=0; j<YUAN_STACK_DEPTH; j++) {
        yzx +=abs(Em - yuan->depth[d][j]);
    }

    yzx=yzx*((unsigned long)1000)/YUAN_STACK_DEPTH/YUAN_FULLHOLD;

    return yzx;
}



static int32_t print_yuan_info(struct yuan_t *p) {

    int32_t i,j;

    if(p == NULL) {
        printf("\t%s: pointer is null", __func__);
        return 0;
    }

    printf("\tID:%u\tType:%u\tSum:%ld\n", p->id, p->type, p->sum*(long)100/YUAN_THRESHOLD);
    
    printf("\t#idx%%\t#dis%%\t");
    for(i=0; i<YUAN_STACK_DEPTH; i++) {
        printf("#dep%d\t", i);
    }
    printf("#yzx\t");
    printf("\n");

    for(i=0; i<YUAN_NUM_OF_ALL; i++) {
        printf("\t%d\t%ld", i, org_dis_to_percent(p->dis[i], p->den));
        for(j=0; j<YUAN_STACK_DEPTH; j++) {
            printf("\t%ld", p->depth[i][j]*(long)1000/YUAN_THRESHOLD);
        }
        printf("\t%ld", print_yzx(p, i));
        printf("\n");
    }

    return 0;
}

static int32_t set_yuan_sum(struct yuan_t *p, int64_t value) {

    if(p == NULL) {
        printf("\t%s: pointer is null", __func__);
        return 0;
    }
    
    printf("\t%ld -> %ld\n", p->sum, value);
    p->sum = value;

    return 0;
}

static int32_t set_yuan_dis_by_idx(struct yuan_t *p, uint8_t dis_idx,  int32_t value) {


    if(p == NULL) {
        printf("\t%s: pointer is null\n", __func__);
        return 0;
    }

    if(dis_idx >= YUAN_NUM_OF_ALL) {
        printf("\t%s: dis_idx > yuan num of all\n", __func__);
        return 0;
    }

    printf("\tdis %d: %d -> %d\n", dis_idx, p->dis[dis_idx], value);

    p->dis[dis_idx] = value;

    return 0;
}



//debug api
static int8_t print_input_array() {

    int32_t i;
    printf("input array:");
    for(i=0; i<YUAN_NUM_OF_INPUT; i++) {
        //printf(" %d", input_array[i]);
    }
    printf("\n");
}

//debug api
static int8_t print_output_array() {

    int32_t i;
    printf("output array:");
    for(i=0; i<YUAN_NUM_OF_INPUT; i++) {
        //printf(" %d", output_array[i]);
    }
    printf("\n");

}

static void cmdline_header()  {

    printf("\n");
    printf("\tNone System Cmdline v0.1\n");

}

static void help_func() {

    printf("\tUsage:\n");
    printf("\tcmd supported by cmdline:\n");
    printf("\tget/set snap/restore suspend/step/resume help exit version\n");

}

/*
 * no arg: print basic info
 * arg +n: print n yuan basic info
*/
static void get_matrix_func(char *yuan_idx) {

    int32_t ret;
    uint32_t idx=0;

    if(yuan_idx == NULL || check_string_array_only_num(yuan_idx) < 0) {//basic info
        print_matrix_info();
        return;
    } else {                                                           //struct enum info

        idx = atol(yuan_idx);
        if(idx < 0 || idx > (YUAN_NUM_OF_ALL-1)) {
            printf("\tprint arg error, check and try gain\n");
            return;
        }

        ret =  print_yuan_info(&matrix[idx]);
        if(ret < 0) {
            printf("\tprint matrix error: idx %u\n", idx);
            return;
        }
        return;
    }

}

/*
 * 99 sum 88: change yuan[99].sum = 88
 * 99 sum 7 88: change yuan[99]. = 88
 */
static void set_matrix_func(char *yuan_idx, char *item, char *arg1, char *arg2) {

    int32_t ret;
    uint32_t idx=0, dis_idx=0;
    long value=0;

    if(yuan_idx == NULL || item == NULL || arg1 == NULL 
            || check_string_array_only_num(yuan_idx) < 0 
            || check_string_array_only_char(item) < 0 
            || check_string_array_only_num(arg1) < 0) {
        printf("\t%s args error\n", __func__);
        return;
    }

    idx = atol(yuan_idx);
    if(strcmp(item, "sum") == 0) {      //change sum
        value = atol(arg1);
        set_yuan_sum(&matrix[idx], value*YUAN_THRESHOLD/100);
        return;
    } else if(strcmp(item, "dis") == 0) {//change dis[]

        if(check_string_array_only_num(arg2) < 0) {
            printf("\t%s distribute args error\n", __func__);
            return;
        }
        dis_idx = atol(arg1);
        value = atol(arg2);
        set_yuan_dis_by_idx(&matrix[idx], dis_idx,  org_dis_from_percent(value, matrix[idx].den));
        set_random_data(idx, dis_idx, org_dis_from_percent(value, matrix[idx].den));
        return;
    }
}

static void snap_matrix_func() {
    int32_t ret;
    ret = make_matrix_snapshot();
    if(ret < 0) {
        printf("\tsnap matrix error\n");
        return;
    }

    printf("\tsnap matrix success\n");
}

static void restore_matrix_func(char *file_name) {

    int32_t ret;

    if(file_name == NULL) {
        printf("\t%s args error\n", __func__);
        return;
    }

    ret = restore_matrix_snapshot(file_name);
    if(ret < 0) {
        printf("\trestore matrix error\n");
        return;
    }

    printf("\trestore matrix %s success\n", file_name);
    
}

static void suspend_kernel_func() {
    int32_t ret = suspend_kernel(); 
    if(ret < 0) {
        printf("\tsuspend kernel error\n");
        return;
    }
    printf("\tsuspend kernel success\n");
}

static void resume_kernel_func() {
    
    int32_t ret = resume_kernel(); 
    if(ret < 0) {
        printf("\tresume kernel error\n");
        return;
    }
    printf("\tresume kernel success\n");

}

static void step_kernel_func() {
    
    int32_t ret = step_kernel(); 
    if(ret < 0) {
        printf("\tstep kernel error\n");
        return;
    }
    printf("\tstep kernel success\n");

}

static void sync_kernel_func() {
    
    int32_t ret = sync_kernel(); 
    if(ret < 0) {
        printf("\tsync kernel error, GPU mey be running\n");
        return;
    }
    printf("\tsync kernel success\n");

}

static void clear_screen() {
    printf("\033[H""\033[J");
}

static void exit_none() {

        cmd_loop = 0;

}



/********************* cmd framework ********************/

static int32_t process_cmdline(char *cmd[]) {

    if(strcmp(cmd[0], "help") == 0) {
        help_func();
        return 0;
    } else if(strcmp(cmd[0], "get") == 0) {
        get_matrix_func(cmd[1]);
        return 0;
    } else if(strcmp(cmd[0], "set") == 0) {
        set_matrix_func(cmd[1], cmd[2], cmd[3], cmd[4]);
        return 0;
    } else if(strcmp(cmd[0], "snap") == 0) {
        snap_matrix_func();
        return 0;
    } else if(strcmp(cmd[0], "restore") == 0) {
        restore_matrix_func(cmd[1]);
        return 0;
    } else if(strcmp(cmd[0], "suspend") == 0) {
        suspend_kernel_func();
        return 0;
    } else if(strcmp(cmd[0], "resume") == 0) {
        resume_kernel_func();
        return 0;
    } else if(strcmp(cmd[0], "step") == 0) {
        step_kernel_func();
        return 0;
    } else if(strcmp(cmd[0], "sync") == 0) {
        sync_kernel_func();
        return 0;
    } else if(strcmp(cmd[0], "clear") == 0) {
        clear_screen();
        return 0;
    } else if(strcmp(cmd[0], "version") == 0) {
        cmdline_header();
        return 0;
    } else if(strcmp(cmd[0], "exit") == 0 || strcmp(cmd[0], "q") == 0) {
        exit_none();
        return 0;
    }

    return -1;
}


int32_t setup_cmd_loop(void) {

    int32_t i, ret;
    char cmd_line_buffer[CMD_BUFFER_SIZE]={0};
    char *cmd_line_p = cmd_line_buffer;
    char *args[5], *arg_p;
    struct func_list_t *handler;

    cmdline_header();


    while(cmd_loop) {

        handler = NULL;
        cmd_line_p = cmd_line_buffer;
        memset(args, 0, sizeof(args));
        memset(cmd_line_p, 0, CMD_BUFFER_SIZE);

        printf("#");
        fgets(cmd_line_p, CMD_BUFFER_SIZE, stdin);

        for(i=0; (arg_p=strtok(cmd_line_p, " ")) != NULL; i++) {
            cmd_line_p=NULL;
            args[i]=arg_p;
        }
        args[i-1][strlen(args[i-1])-1]=0;

        if(args[0] == NULL || strcmp(args[0], "") == 0) {
            //printf("\tNo input detected, Please try help\n");
            continue;//for(;;)
        }

        ret = process_cmdline(args);
        if(ret < 0) {
            printf("\tNo cmd detected, Please try help\n");
            continue;//for(;;)
        }

    }//exit while(cmd_loop)

    return 0;
}






