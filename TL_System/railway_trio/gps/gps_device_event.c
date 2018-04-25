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

#define KFLAG_INTERVAL_USEC    217*1000

int gps_status = 0;
struct gps_info_t gps_info={0.0, 0.0, 0.0, 0.0};
struct gps_info_t gps_info_old={0.0, 0.0, 0.0, 0.0};

static int direction = 0;
static struct timeval old_moment;
static struct timeval latest_moment;

void set_led_on() {
    
    system("echo 1 > /sys/class/leds/uart_ok/brightness");
}

void set_led_off() {
    
    system("usleep 10000 && echo 0 > /sys/class/leds/uart_ok/brightness");
}

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

int64_t get_kflag_interval() {
    struct timeval tmp;
    int64_t t;
    if(direction == 0) {
        return 990*1000;
    } else {
        gettimeofday(&tmp, 0);
        t = (tmp.tv_sec - old_moment.tv_sec)*1000*1000 + tmp.tv_usec - old_moment.tv_usec;
        t = KFLAG_INTERVAL_USEC - t;
        return t>1000?t:1000;
    }

}

int check_and_process_kflag() {
    
    int ret;
    struct timeval tmp;
    int64_t t;
    if(direction == 0) {
        return 0;
    }

    gettimeofday(&tmp, 0);

    t = (tmp.tv_sec - old_moment.tv_sec)*1000*1000 + tmp.tv_usec - old_moment.tv_usec;
    if(t >= KFLAG_INTERVAL_USEC) {
        t = (tmp.tv_sec - latest_moment.tv_sec)*1000*1000 + tmp.tv_usec - latest_moment.tv_usec;

        float dis = (gps_info_old.speed<=2?0:gps_info_old.speed)*1000/3600.0*t/1000.0/1000.0;
        gps_info.kmposition = gps_info_old.kmposition + direction*dis/1000.0;
        gps_info.longitude = gps_info_old.longitude;
        gps_info.latitude = gps_info_old.latitude;
        gps_info.speed = gps_info_old.speed;

        ret = send_gps_info_to(IDX_DEVICE_TRIO);
        if(ret < 0) {
            logerr( "sent gps info to trio error: return %d\n", ret);
            return -1;
        }
        ret = send_gps_info_to(IDX_USER_CLIENT);
        if(ret < 0) {
            logerr( "sent gps info to client error: return %d\n", ret);
            return -1;
        }
        gettimeofday(&old_moment, 0);
    }

}

int process_device_event(unsigned char *tty) {
    static int i=0;
    int ret;
    double tmp;

    set_led_on();
    if(tty[0] == 0xaa) {
        struct sc_t *sc = (struct sc_t *)tty;
        if(tty[23] == '.') {
            gps_info.latitude = latitude_dmsc_to_ddd(&sc->latitude);
            gps_info.longitude = longitude_dmsc_to_ddd(&sc->longitude);
        }
        gps_info.kmposition =  (sc->kmposition[2]*256*256 + sc->kmposition[1]*256 + sc->kmposition[0])/1000.0;


        gps_info.speed = sc->speed;
    }

    struct rcats2_t *rcats2 = (struct rcats2_t *) tty;
    if(tty[0] == 0xca) {
        tmp = ((rcats2->kmposition[2]&0x3F)*256*256 + rcats2->kmposition[1]*256 + rcats2->kmposition[0])/1000.0;
        tmp = (rcats2->kmposition[2]&0x80) > 0 ? tmp*-1.0 : tmp;

        switch (tty[1]) {
            case 0xff:
                if(gps_info_old.kmposition == tmp) {
                    set_led_off();
                    return;
                }
                direction = gps_info_old.kmposition > tmp ? -1:1;
                gps_info.latitude = latitude_dmsc_to_ddd(&rcats2->latitude);
                gps_info.longitude = longitude_dmsc_to_ddd(&rcats2->longitude);
                gps_info.kmposition = tmp;
                gps_info.speed = rcats2->speed;
                gps_info_old.kmposition = gps_info.kmposition;
                gps_info_old.latitude = gps_info.latitude;
                gps_info_old.longitude = gps_info.longitude;
                gps_info_old.speed = gps_info.speed;
                //update timer
                //direction = rcats2->kmposition[2]&0x40 > 0 ? 1 : -1;

                gettimeofday(&latest_moment, 0);

                break;

            case 0x0f://lati && longit ok
                gps_info.latitude = latitude_dmsc_to_ddd(&rcats2->latitude);
                gps_info.longitude = longitude_dmsc_to_ddd(&rcats2->longitude);
                gps_info.kmposition = gps_info_old.kmposition;
                gps_info.speed = gps_info_old.speed;
                gps_info_old.latitude = gps_info.latitude;
                gps_info_old.longitude = gps_info.longitude;
                break;

            case 0xf0://kp ok
                if(gps_info_old.kmposition == tmp) {
                    set_led_off();
                    return;
                }
                direction = gps_info_old.kmposition > tmp ? -1:1;
                gps_info.latitude = gps_info_old.latitude;
                gps_info.longitude = gps_info_old.longitude;
                gps_info.kmposition = tmp;
                gps_info.speed = rcats2->speed;
                gps_info_old.kmposition = gps_info.kmposition;
                gps_info_old.speed = gps_info.speed;
                //update timer
                //direction = rcats2->kmposition[2]&0x40 > 0 ? 1 : -1;
                gettimeofday(&latest_moment, 0);
                break;

            case 0x00:
                gps_info.kmposition = gps_info_old.kmposition;
                gps_info.longitude = gps_info_old.longitude;
                gps_info.latitude = gps_info_old.latitude;
                gps_info.speed = gps_info_old.speed;
                break;

            default:
                return 0;

        }

    }

    ret = send_gps_info_to(IDX_DEVICE_TRIO);
    if(ret < 0) {
        logerr( "sent gps info to trio error: return %d\n", ret);
        return -1;
    }

    ret = send_gps_info_to(IDX_USER_CLIENT);
    if(ret < 0) {
        logerr( "sent gps info to client error: return %d\n", ret);
        return -1;
    }
    set_led_off();
    gettimeofday(&old_moment, 0);

}
