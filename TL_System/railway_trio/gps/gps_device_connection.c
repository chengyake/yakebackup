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
	options.c_iflag &=~(INLCR|IGNCR|ICRNL);
	options.c_oflag &=~(ONLCR|OCRNL);
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
            logerr("Unsupported flow_ctrl\n"); 
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
            logerr("Unsupported data size\n");  
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
            logerr( "Unsupported parity\n");      
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
            logerr( "Unsupported stop bits\n");   
            return -1;  
    } 
    if (tcsetattr(fd,TCSANOW,&options) != 0) {  
        logerr( "com set error!\n");    
        return -1;   
    } 
    return 0;

}


int attach_device() {

    int ret;
    int i;

    dev_fd = open (GPS_DEV_TTY, O_RDWR); 
    if(dev_fd < 0) {
        logerr( "open GPS_DEV_TTY %s error\n", GPS_DEV_TTY);
        gps_status = REPORT_GPS_OPEN_ERR;
        return dev_fd;
    }

    ret = tty_init(dev_fd, B115200, 0, 8, 1, 'N');
    if(ret < 0) {
        logerr( "init GPS_DEV_TTY %s error\n", GPS_DEV_TTY);
        gps_status = REPORT_GPS_UNKNOWN_ERR ;
        return ret;
    }
    
    //fcntl(dev_fd, F_SETFL, O_NONBLOCK);

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
    loginfo("attach gps device sucess");
    return 0;

}

int deattach_device() {

    close(dev_fd);
    return 0;

}


int check_frame_sum(unsigned char*data, unsigned int num) {
    
    unsigned int i;
    unsigned char sum=0;
    
    if(data == NULL) {
        logerr("check frame sum error:\n");
        return -1;
    }

    for(i=0; i<num-1; i++) {
        sum ^= data[i];   
    }

    if(sum == data[num-1])  {
        logdebug("check frame sum pass!!\n");
        return 0;
    } else {
        logdebug("check frame sum error: we get sum %02x\n", sum);
        return -1;

    }

    return 0;

}

int recv_device() {
    int ret;
    static int idx=0;
    static unsigned char buf[64]={0};

    logdebug("select gps device success");

    while (buf[0] != 0xca && buf[0] != 0xaa) {
        idx = 0;
        memset(buf, 0, sizeof(buf));
        ret = read(dev_fd, &buf[idx++], 1);
        if(ret <= 0) {
            logerr( "read gps device error1: %d\n", errno);
            gps_status = REPORT_GPS_UNKNOWN_ERR ;
            goto try_next;
        }
    }

    while (idx < 20) {
        ret = read(dev_fd, &buf[idx], 20-idx);
        if(ret <= 0) {
            logerr( "read gps device error2:  %d\n", errno);
            gps_status = REPORT_GPS_UNKNOWN_ERR ;
            goto try_next;
        }
        idx += ret;
    }

    //ignore 20bytes frame
    if(check_frame_sum(&buf[0], 20) == 0) {
        process_device_event(&buf[0]);
        goto try_next;
    }

    while (idx < 39) {
        ret = read(dev_fd, &buf[idx], 39-idx);
        if(ret <= 0) {
            logerr( "read gps device error2:  %d\n", errno);
            gps_status = REPORT_GPS_UNKNOWN_ERR ;
            goto try_next;
        }
        idx += ret;
    }


    if(check_frame_sum(&buf[0], 39) == 0) {
        gps_status = 0;
        process_device_event(&buf[0]);
    } 

try_next:
    memset(buf, 0, sizeof(buf));
    idx = 0;
    return 0;

}





