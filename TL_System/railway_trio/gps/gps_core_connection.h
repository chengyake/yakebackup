#ifndef gps_core_connection_h
#define gps_core_connection_h



extern int write_core_server(char *buf, int len);
extern int recv_core_server();
extern int attach_core_server();
extern int deattach_core_server();

extern int core_fd;




#endif
