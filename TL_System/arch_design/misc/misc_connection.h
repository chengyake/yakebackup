#ifndef misc_connection_h
#define misc_connection_h

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <syslog.h>



extern int misc_write_msg(char *buf, int len);
extern int misc_read_msg(char *buf, int len);
extern int clients_data_changed();
extern int setup_evnet_loop();



extern int misc_loop;

#endif
