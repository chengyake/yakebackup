#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#include <sys/stat.h>
#include <sys/types.h>
//#include <fcntl.h>
//#include <unistd.h>
#include <syslog.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>




int32_t get_snapshot_filename_by_time(int8_t *data) {
    
    time_t tmp;
    struct tm *p;
    
    if(data == NULL) {
        logerr("params of get_current_time_string() error");
        return -1;
    }

    tmp = time(NULL);
    p = localtime(&tmp);
    sprintf(data, "%4d%02d%02d_%02d%02d.db",
        (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min);

    return 0;

}


time_t get_current_sec() {
    return time(NULL);
}



