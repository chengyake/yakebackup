
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>
#include <tty_config.h>

int  tty_init(char * device_path,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    struct termios options;
    int fd = -1;
    fd = open (device_path, O_RDWR); // | O_NDELAY
    if (fd < 0) {
        printf ("could not open device: %s. \n", device_path);
        return -1;
    }
    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd, &options);

    /*
     * Enable the receiver and set local mode
     */
    options.c_cflag |= (CLOCAL | CREAD);
    
//=======set default values
	/*
	 * Select 8 data bits, 1 stop bit and no parity bit
	 */
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        	/*
	 * Disable hardware flow control
	 */
	options.c_cflag &= ~CRTSCTS;
        /*
         * Set the baud rates to 19200= B19200...
         */
        cfsetispeed(&options, B19200);
        cfsetospeed(&options, B19200);

        /*
         * Choosing raw input
         */
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

        /*
         * Disable software flow control
         */
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
        /*
         * Disable hardware flow control
         */
        options.c_cflag &= ~CRTSCTS;
	/*
	 * Choosing raw output
	 */
	options.c_oflag &= ~OPOST;
	/*
	 * Set read timeouts
	 */
	//options.c_cc[VMIN] = 0;
	//options.c_cc[VTIME] = 0;
//=======enf of set default values


    /*
     * Set the baud rates to 19200= B19200...
     */
        cfsetispeed(&options, speed);
        cfsetospeed(&options, speed);

        switch(flow_ctrl)
        {
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
                fprintf(stderr,"Unsupported flow_ctrl\n");  
                return -1;   

        }
        options.c_cflag &= ~CSIZE;  
        switch (databits)  
        {    
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
                fprintf(stderr,"Unsupported data size\n");  
                return -1;   
        }  
        switch (parity)  
        {    
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
             fprintf(stderr,"Unsupported parity\n");      
             return -1;   
        }   

        switch (stopbits)  
        {    
            case 1:     
                options.c_cflag &= ~CSTOPB; 
                break;   
            case 2:     
                options.c_cflag |= CSTOPB;
                break;  
            default:     
                fprintf(stderr,"Unsupported stop bits\n");   
            return -1;  
        }  

        
        if (tcsetattr(fd,TCSANOW,&options) != 0)    
        {  
            perror("com set error!\n");    
            return -1;   
        }  
        return fd;
}


