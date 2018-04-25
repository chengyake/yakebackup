#ifndef local_h
#define local_h



extern int write_local_server(char *buf, int len);
extern int read_local_server(char *buf, int len);
extern int attach_local_server();
extern int deattach_local_server();


extern int local_fd;



#endif
