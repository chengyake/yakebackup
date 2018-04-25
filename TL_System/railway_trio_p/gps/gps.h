#ifndef GPS_H
#define GPS_H

#include <stdint.h>

struct gps_info_t {
    double longitude;
    double latitude;
    double speed;
    double kmposition;
};



#define GPS_DEV_TTY     "/dev/ttyUSB3"

#endif
