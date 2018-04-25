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

#include <termios.h>

#include "../core/core.h"
#include "gps.h"
#include "gps_device_connection.h"
#include "gps_device_event.h"

int dev_fd=0;


static int tty_init(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity) {

    struct termios options;

    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd, &options);

    //Enable the receiver and set local mode
    options.c_cflag |= (CLOCAL | CREAD);
#if 1
    //Select 8 data bits, 1 stop bit and no parity bit
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    //Disable hardware flow control
    options.c_cflag &= ~CRTSCTS;
    //Set the baud rates to 19200= B19200...
    cfsetispeed(&options, B19200);
    cfsetospeed(&options, B19200);

    //Choosing raw input
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    //Disable software flow control
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    //Disable hardware flow control
    options.c_cflag &= ~CRTSCTS;
    //Choosing raw output
    options.c_oflag &= ~OPOST;
    //Set read timeouts
    //options.c_cc[VMIN] = 0;
    //options.c_cc[VTIME] = 0;
#endif //=======enf of set default values

    //Set the baud rates to 19200= B19200...
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    switch(flow_ctrl) {
        case 0 ://Disable  flow control
            options.c_cflag &= ~CRTSCTS;
            break;   
        case 1 ://enable hardware flow control
            options.c_cflag |= CRTSCTS;
            break;
        case 2 ://enable software flow control
            options.c_cflag |= IXON | IXOFF | IXANY;
            break;
        default:
            syslog(LOG_ERR,"Unsupported flow_ctrl\n"); 
            return -1;   

    }
    options.c_cflag &= ~CSIZE;  
    switch (databits) {    
        case 5    :  
            options.c_cflag |= CS5;  
            break;  
        case 6    :  
            options.c_cflag |= CS6;  
            break;  
        case 7    :      
            options.c_cflag |= CS7;  
            break;  
        case 8:      
            options.c_cflag |= CS8;  
            break;    
        default:     
            syslog(LOG_ERR,"Unsupported data size\n");  
            return -1;   
    }  
    switch (parity) {
        case 'n':  
        case 'N': //no parity bit
            options.c_cflag &= ~PARENB;   
            options.c_iflag &= ~INPCK;      
            break;   
        case 'o':    
        case 'O'://odd
            options.c_cflag |= (PARODD | PARENB);   
            options.c_iflag |= INPCK;               
            break;   
        case 'e':   
        case 'E'://even
            options.c_cflag |= PARENB;         
            options.c_cflag &= ~PARODD;         
            options.c_iflag |= INPCK;        
            break;  
        case 's':  
        case 'S': //space
            options.c_cflag &= ~PARENB;  
            options.c_cflag &= ~CSTOPB;  
            break;   
        default:    
            syslog(LOG_ERR, "Unsupported parity\n");      
            return -1;   
    }   
    switch (stopbits) {    
        case 1:     
            options.c_cflag &= ~CSTOPB; 
            break;   
        case 2:     
            options.c_cflag |= CSTOPB;
            break;  
        default:     
            syslog(LOG_ERR, "Unsupported stop bits\n");   
            return -1;  
    } 
    if (tcsetattr(fd,TCSANOW,&options) != 0) {  
        syslog(LOG_ERR, "com set error!\n");    
        return -1;   
    } 
    return 0;

}


int attach_device() {

    int ret;
    int i;

    dev_fd = open (GPS_DEV_TTY, O_RDWR); 
    if(dev_fd < 0) {
        syslog(LOG_ERR, "open GPS_DEV_TTY %s error\n", GPS_DEV_TTY);
        gps_status = REPORT_GPS_OPEN_ERR;
        return dev_fd;
    }

    ret = tty_init(dev_fd, B115200, 0, 8, 1, 'N');
    if(ret < 0) {
        syslog(LOG_ERR, "init GPS_DEV_TTY %s error\n", GPS_DEV_TTY);
        gps_status = REPORT_GPS_UNKNOWN_ERR ;
        return ret;
    }
    
    fcntl(dev_fd, F_SETFL, O_NONBLOCK);

    /*
    printf("0x33 \n");
    for(i=0; i<1000; i++)
    {   
        int len;
        //printf("0x55\n");
        unsigned char tmp[38]={0x55, 0};
        len = read(dev_fd, &tmp[0], 38);
        for(i=0; i<len&&i<38; i++) {
            printf("%02x ", tmp[i]);
        }
        printf("-\n");
        sleep(1);
    }*/

    return 0;

}

int deattach_device() {

    close(dev_fd);
    return 0;

}



int recv_device() {
    int ret;
    static int idx=0;
    unsigned char buf[64]={0};

    while (buf[0] != 0xca && buf[0] != 0xaa) {
        idx = 0;
        memset(buf, 0, sizeof(buf));
        ret = read(dev_fd, &buf[idx++], 1);
        if(ret < 0) {
            syslog(LOG_ERR, "read gps device error1: return %d\n", ret);
            gps_status = REPORT_GPS_UNKNOWN_ERR ;
            return -1;
        }
    }

    while (idx < 39) {
        ret = read(dev_fd, &buf[idx], 39-idx);
        if(ret < 0) {
            syslog(LOG_ERR, "read gps device error2: return %d\n", ret);
            gps_status = REPORT_GPS_UNKNOWN_ERR ;
            return -1;
        }
        idx += ret;
    }


    gps_status = 0;
    process_device_event(&buf[0]);
    return 0;

}





