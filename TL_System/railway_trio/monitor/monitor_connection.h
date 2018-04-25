#ifndef monitor_connect_h
#define monitor_connect_h



extern int attach_monitor_server();
extern int deattach_monitor_server();
extern int write_monitor_server(char *buf, int len);
extern int read_monitor_server(char *buf, int len);


extern int setup_monitor_event_loop();

#endif
