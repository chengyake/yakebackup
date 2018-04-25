#ifndef client_connection_h
#define client_connection_h


extern int check_md5_fcs(char *data, int len, char *fcs);
extern int make_md5_fcs(char *data, int len, char *fcs);

/*DEBUG API*/
extern void debug_print_header(struct frame_header *p);
extern void debug_print_data(struct frame_header *p);
extern void debug_print_tail(struct frame_header *p);
extern void debug_print_frame(struct frame_header *p);



extern int setup_event_loop();







#endif
