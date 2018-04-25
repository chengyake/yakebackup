
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/un.h>  
#include <stdint.h>

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
	char *  name;
	uint64_t kmp;
	uint8_t temp1 = 0x22;
	uint8_t temp2 = 0x33;
	kmp =((uint64_t) temp1 <<16) + (uint64_t) temp2<<8;

	 printf("kmp =%lx\n", kmp);

return 1;
	name = (char *)malloc(6);
    i = error_check(argc, argv);
	strncpy(name, "1234567890", 10);
  printf("name =%s", name);
  free(name);
    printf("after free.");
     
 
    return 0;
}
