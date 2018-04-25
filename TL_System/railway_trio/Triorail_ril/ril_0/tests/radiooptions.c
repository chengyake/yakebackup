
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/un.h>  

#define SOCKET_NAME_RIL_DEBUG	"trio-rild-debug-0"	/* from ril.cpp */

enum options {
    RADIO_RESET,
    RADIO_OFF,
    UNSOL_NETWORK_STATE_CHANGE,
    RADIO_ON,
    SETUP_PDP,
    DEACTIVATE_PDP,
    DIAL_CALL,
    ANSWER_CALL,
    END_CALL,
};


static void print_usage() {
    perror("Usage: radiooptions [option] [extra_socket_args]\n\
           0 - RADIO_RESET, \n\
           1 - RADIO_OFF, \n\
           2 - UNSOL_NETWORK_STATE_CHANGE, \n\
           3 - RADIO_ON, \n\
           4 apn- SETUP_PDP apn, \n\
           5 - DEACTIVE_PDP, \n\
           6 number - DIAL_CALL number, \n\
           7 - ANSWER_CALL, \n\
           8 - END_CALL \n");
}

static int error_check(int argc, char * argv[]) {
    if (argc < 2) {
        return -1;
    }
    const int option = atoi(argv[1]);
    if (option < 0 || option > 10) {
        return 0;
    } else if ((option == DIAL_CALL || option == SETUP_PDP) && argc == 3) {
        return 0;
    } else if ((option != DIAL_CALL && option != SETUP_PDP) && argc == 2) {
        return 0;
    }
    return -1;
}

static int get_number_args(char *argv[]) {
    const int option = atoi(argv[1]);
    if (option != DIAL_CALL && option != SETUP_PDP) {
        return 1;
    } else {
        return 2;
    }
}

int main(int argc, char *argv[])
{
    int fd;
    int num_socket_args = 0;
    int i  = 0;
    if(error_check(argc, argv)) {
        print_usage();
        exit(-1);
    }

    /* create a socket */  
    fd = socket(AF_UNIX, SOCK_STREAM, 0);  

    struct sockaddr_un address;  
    address.sun_family = AF_UNIX;  
    strcpy(address.sun_path, SOCKET_NAME_RIL_DEBUG);  
       
    /* connect to the server */  
    int result = connect(fd, (struct sockaddr *)&address, sizeof(address));  
    if(result == -1)  
    {  
        perror("connect failed: ");  
        exit(1);  
    }  
     
    if (fd < 0) {
        perror ("opening radio debug socket");
        exit(-1);
    }

    num_socket_args = get_number_args(argv);
    int ret = send(fd, (const void *)&num_socket_args, sizeof(int), 0);
    if(ret != sizeof(int)) {
        perror ("Socket write error when sending num args");
        close(fd);
        exit(-1);
    }

    for (i = 0; i < num_socket_args; i++) {
        // Send length of the arg, followed by the arg.
        int len = strlen(argv[1 + i]);
        ret = send(fd, &len, sizeof(int), 0);
        if (ret != sizeof(int)) {
            perror("Socket write Error: when sending arg length");
            close(fd);
            exit(-1);
        }
        ret = send(fd, argv[1 + i], sizeof(char) * len, 0);
        if (ret != len * sizeof(char)) {
            perror ("Socket write Error: When sending arg");
            close(fd);
            exit(-1);
        }
    }

    close(fd);
    return 0;
}
