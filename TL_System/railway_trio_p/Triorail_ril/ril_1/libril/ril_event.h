

// Max number of fd's we watch at any one time.  Increase if necessary.
#define MAX_FD_EVENTS 8

typedef void (*ril_event_cb)(int fd, short events, void *userdata);

struct ril_event {
    struct ril_event *next;
    struct ril_event *prev;

    int fd;
    int index;
    bool persist;
    struct timeval timeout;
    ril_event_cb func;
    void *param;
};

// Initialize internal data structs
void ril_event_init();

// Initialize an event
void ril_event_set(struct ril_event * ev, int fd, bool persist, ril_event_cb func, void * param);

// Add event to watch list
void ril_event_add(struct ril_event * ev);

// Add timer event
void ril_timer_add(struct ril_event * ev, struct timeval * tv);

// Remove event from watch list
void ril_event_del(struct ril_event * ev);

// Event loop
void ril_event_loop();

