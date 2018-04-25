#include <stdio.h>
#include <string.h>

#include "none_debug.h"
#include "none_utils.h"
#include "none_matrix.h"
#include "none_input.h"
#include "none_output.h"
#include "none_core.h"
#include "none_cmd.h"


static int32_t init_none_system() {

    int ret = init_debug();
    if(ret < 0) {
        logerr("init debug error %d", ret);
        return -1;
    }

    loginfo("----Startting None System----");

    ret = init_utils();
    if(ret < 0) {
        logerr("init utils error %d", ret);
        return -1;
    }

    ret = init_matrix_database();
    if(ret < 0) {
        logerr("init matrix database error %d", ret);
        return -1;
    }

    ret = init_input_module(); 
    if(ret < 0) {
        logerr("init input thread error %d", ret);
        return -1;
    }

    ret = init_output_module(); 
    if(ret < 0) {
        logerr("init output thread error %d", ret);
        return -1;
    }

    ret = init_kernel_platform();
    if(ret < 0) {
        logerr("init kernel paltform error %d", ret);
        return -1;
    }

    return 0;
}

static int32_t run_none_system() {
    
    int ret;
    
    syslog(LOG_INFO, "into cmd process loop");
    ret = setup_cmd_loop();
    if(ret < 0) {
        logerr("cmd loop error %d", ret);
        return -1;
    }

}

static int32_t close_none_system() {

    int ret = close_kernel_platform();
    if(ret < 0) {
        logerr("close kernel paltform error %d", ret);
        return -1;
    }

    ret = close_output_module(); 
    if(ret < 0) {
        logerr("close output thread error %d", ret);
        return -1;
    }

    ret = close_input_module(); 
    if(ret < 0) {
        logerr("close input thread error %d", ret);
        return -1;
    }

    ret = close_matrix_database();
    if(ret < 0) {
        logerr("close matrix database error %d", ret);
        return -1;
    }

    ret = close_utils();
    if(ret < 0) {
        logerr("close utils error %d", ret);
        return -1;
    }

    loginfo("----Close None System----");

    ret = close_debug();
    if(ret < 0) {
        logerr("close debug error %d", ret);
        return -1;
    }
    printf("\n\tNone System Exit now!\n\n");
    return 0;
}



int main() {

    int32_t ret;
    ret = init_none_system();
    if(ret < 0) {
        return -1;
    }

    ret = run_none_system();
    if(ret < 0) {
        return -1;
    }

    ret = close_none_system();
    if(ret < 0) {
        return -1;
    }

}


