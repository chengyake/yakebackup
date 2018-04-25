#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <syslog.h>
#include <sys/un.h>
#include <fcntl.h>

#include "../core/core.h"
#include "gps.h"
#include "gps_device_event.h"


int gps_status = 0;
struct gps_info_t gps_info={0.0, 0.0, 0, 0};


int check_frame_xor(char * buffer) {
    return 0;
}


double latitude_dmsc_to_ddd(struct latitude_t *la) {//little or big end????
    double result = 0.0;
    int d,m,s;
    
    d = (la->degree[1]-0x30) + (la->degree[0]-0x30)*10;
    m = (la->minute[1]-0x30) + (la->minute[0]-0x30)*10;
    s = (la->fraction[3]-0x30) + (la->fraction[2]-0x30)*10 + (la->fraction[1]-0x30)*100 + (la->fraction[0]-0x30)*1000;

    result = d + (double)m/60+ (double)s/60/10000;
    return result;
}

double longitude_dmsc_to_ddd(struct longitude_t *lo) {
    double result = 0.0;
    int d,m,s;

    d = (lo->degree[2]-0x30) + (lo->degree[1]-0x30)*10 + (lo->degree[0]-0x30)*100;
    m = (lo->minute[1]-0x30) + (lo->minute[0]-0x30)*10;
    s = (lo->fraction[3]-0x30) + (lo->fraction[2]-0x30)*10 + (lo->fraction[1]-0x30)*100 + (lo->fraction[0]-0x30)*1000;
    result = d + ((double)m)/60 + ((double)s)/60/10000;

    return result;
}

int process_device_event(unsigned char *tty) {
    static int i=0;
    int ret;

    if(tty[0] == 0xaa) {
        struct sc_t *sc = (struct sc_t *)tty;
        gps_info.latitude = latitude_dmsc_to_ddd(&sc->latitude);
        gps_info.longitude = longitude_dmsc_to_ddd(&sc->longitude);
        gps_info.kmposition =  (sc->kmposition[2]*256*256 + sc->kmposition[1]*256 + sc->kmposition[0])/1000.0;
        //printf("%f\n", gps_info.kmposition);
        gps_info.speed = sc->speed;
    }

    if(tty[0] == 0xca) {

        struct rcats2_t *rcats2 = (struct rcats2_t *) tty;
        gps_info.latitude = latitude_dmsc_to_ddd(&rcats2->latitude);
        gps_info.longitude = longitude_dmsc_to_ddd(&rcats2->longitude);
        gps_info.kmposition = ((double)(((uint64_t)rcats2->kmposition[2])<<16 +((uint64_t)rcats2->kmposition[1])<<8 + 
                (uint64_t)rcats2->kmposition[0]))/(double)1000;
        gps_info.speed = rcats2->speed;
    }

    /*
    printf("\n");
    for(ret=0; ret<39; ret++) {
        printf("%02x", tty[ret]);
    }
    printf("\n");
    */
    
    ret = send_gps_info_to(IDX_DEVICE_TRIO);
    if(ret < 0) {
        syslog(LOG_ERR, "sent gps info to trio error: return %d\n", ret);
        return -1;
    }
    
    
    printf("read =============== %f, %f, %f, %f\n", gps_info.longitude, gps_info.latitude, gps_info.speed, gps_info.kmposition);

    ret = send_gps_info_to(IDX_USER_CLIENT);
    if(ret < 0) {
        syslog(LOG_ERR, "sent gps info to client error: return %d\n", ret);
        return -1;
    }

}
