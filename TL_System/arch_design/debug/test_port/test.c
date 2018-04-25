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

/*
 *  bu hao yong; unused
 *  busybox microcom will be better
 *
 */




int dev_fd=0;
char tty_array[64]={0};

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
            printf("Unsupported flow_ctrl\n"); 
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
            printf("Unsupported data size\n");  
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
            printf( "Unsupported parity\n");      
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
            printf( "Unsupported stop bits\n");   
            return -1;  
    } 
    if (tcsetattr(fd,TCSANOW,&options) != 0) {  
        printf( "com set error!\n");    
        return -1;   
    } 
    return 0;

}


int attach_device() {

    int ret;
    int i;

    dev_fd = open (tty_array, O_RDWR); 
    if(dev_fd < 0) {
        printf( "open %s error\n", tty_array);
        exit(0);
    }

    ret = tty_init(dev_fd, B115200, 0, 8, 1, 'N');
    if(ret < 0) {
        printf( "init tty_array %s error\n", tty_array);
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

    printf("attach gps device sucess\n");
    return 0;

}

int deattach_device() {

    close(dev_fd);
    return 0;

}

/*
 * to modem   : \r
 * from modem : \r\n
 */

int main(int argc, char *argv[]) {
    
    int i;
    int len = strlen(argv[1]);
    if(argc != 2) {
        printf("usage: \n\tatport /dev/ttyUSBx or\n\tatport /dev/ttyACMx\n");
        return 0;
    }
    
    memcpy(tty_array, argv[1], len);

    attach_device();

    printf("echo AT > %s\n", tty_array);
    
    /*
    for(i=0; i<len+1; i++) {
        printf("%X ", tty_array[i]);
    }
    printf("\n");
    */

    write(dev_fd, "AT\r\n", 4);
    read(dev_fd, tty_array, 4);
    printf("%s\n", tty_array);

    deattach_device();

    return 0;

}










