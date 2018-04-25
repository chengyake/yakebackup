#ifndef LOCAL_CLIENT_EVENT_H
#define LOCAL_CLIENT_EVENT_H



enum tele_state_t {
        TELE_IDLE = 0,
        TELE_WAIT,
        TELE_ACCESS,
        TELE_ONLINE,
        TELE_OFFLINE,
        TELE_MAX,
};

struct tele_switch_t {
    int state;
    int64_t begin;

};



extern int get_gps_status();
extern int get_gprs_status();
extern int get_trio0_status();
extern int get_trio1_status();
extern int get_trace_status();



extern int system_status;
extern struct gps_info_t gps;
extern int module_status;



#endif
