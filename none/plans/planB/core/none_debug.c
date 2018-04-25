
/*
 *   some thing like <Convergence line>  <matrix ticks>
 *  
 *   output them to stdout or socket
 */

#include <stdint.h>
#include <stdio.h>

#include "none_debug.h"


int init_debug() {

    openlog("None-System", LOG_CON, LOG_USER);

    return 0;
}



int close_debug() {

    closelog();
    return 0;
}


//shoulian xishu


