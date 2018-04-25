#ifndef client_h
#define client_h

#include <time.h>
#include <stdint.h>
#define REMOTE_SERVER_IP   "61.237.239.183"
//#define REMOTE_SERVER_IP   "127.0.0.1"
//#define REMOTE_SERVER_IP   "192.168.0.102"
//#define REMOTE_SERVER_IP   "192.168.0.101"
//#define REMOTE_SERVER_IP   "192.168.0.104"
//#define REMOTE_SERVER_IP   "121.42.27.147"

#define REMOTE_SERVER_PORT  (55555)

#define REPORT_INTERVAL_SEC  (60*1)
#define UPLOAD_INTERVAL_SEC  (10*1)

#define FRAME_TASK_ID_SIZE  (16)
#define FRAME_MD5_SIZE  (16)


/* note: byte alignment */
#pragma pack (1)
struct frame_fmt {
    unsigned char head; //1
    unsigned short len; //2
    unsigned int token; //4

    unsigned char task_type;   //1
            short task_state; //2
    unsigned char task_id[FRAME_TASK_ID_SIZE]; //16

    unsigned char data[0];     //
    unsigned char md5[FRAME_MD5_SIZE];     //16
    unsigned char frame_end;   //1
};
#pragma pack ()
#define FRAME_FMT_SIZE       (sizeof(struct frame_fmt))
#define FRAME_DATA_BEFORE_SIZE  (26)




#define TRACE_FILE_NAME   (256)
#pragma pack (1)
struct task_fmt {
    unsigned short len; //2
    unsigned int start_position; //4
    unsigned int stop_position;  //4
    uint64_t  start_time;    //8
    uint64_t  stop_time;     //8
    unsigned char direction;   //1
    unsigned int  init_position; //4?
    unsigned char file_name_len;   //1
    unsigned char file_name[TRACE_FILE_NAME];
};
#pragma pack ()
#define TASK_DATA_SIZR (sizeof(struct task_fmt))


#pragma pack (1)
struct upload_fmt {
    unsigned int fragment_id; //4
    unsigned short fragment_len; //2
    unsigned char compress_flag; //1?
    unsigned char file_name_len;   //1
    unsigned char file_name[TRACE_FILE_NAME];
    unsigned char file_content[0];
};
#pragma pack ()

#define UPLOAD_TASK_DATA_SIZR (sizeof(struct upload_fmt))


enum task_type {
    NO_TASK = 0,
    REG_TASK,
    TEST_TASK,
    UPLOAD_TASK,
    REPORT_TASK = 5,
    TIME_TASK,
};

enum register_task_state {
    REG_FAILED = -100,
    REG_SUCCESS = 102,
    REG_REQUEST = 103,
    REG_ACTION = 101,
};

enum test_task_state {
    RECV_TASK = 200,
    RECV_TASK_SUCCESS = 201,
    RECV_TASK_FAILED = -200,
    START_TASK = 202,
    START_TASK_SUCCESS = 204,
    START_TASK_STATUS = 212,    //add
    START_TASK_FAILED = -201,
    START_TASK_DOING = 210,
    STOP_TASK = 205,
    STOP_TASK_SUCCESS = 207,
    STOP_TASK_REASON = 211,     //add
    STOP_TASK_SUCCESS_AUTO = 10000,
    STOP_TASK_SUCCESS_MANUAL = 10001,
    STOP_TASK_FAILED = -202,
    CANCEL_TASK = 208,
    CANCEL_TASK_SUCCESS = 209, //tky 207, //209,tky
    CANCEL_TASK_FAILED = -203,
};

enum upload_task_state {
	UPLOAD_REQUEST = 300,
	UPLOAD_AGREE = 301,
	UPLOAD_UPLOADING = 303,
	UPLOAD_LATEST = 304,
	UPLOAD_GOON = 305,
	UPLOAD_SUCCESS = 311,
};

enum time_task_state {
    SET_TIME_REQUEST = 600,
    SET_TIME_SUCCESS = 601,
};


enum action_failed_code {
    ACTION_UNKOWN_ERR = 0,
    ACTION_MD5_ERR = 1,
    ACTION_GSMR1_ERR = 2,
    ACTION_GSMR2_ERR = 3,
    ACTION_TRACE_ERR = 4,
    ACTION_REBOOT_ERR = 5,
    ACTION_REBOOT_GSMR1_ERR = 6,
    ACTION_REBOOT_GSMR2_ERR = 7,
    ACTION_EXPIRED_ERR = 8,
    ACTION_DATABASE_NO_SPACE_ERR = 9,
    ACTION_NO_THIS_TASK = 10,
    ACTION_OTHER_TASK_RUNNING = 11,
    ACTION_RECV_REPEAT_TASK = 12,
};


extern int write_remote_server(char *buf, int len);
extern int read_remote_server(char *buf, int len);
extern int attach_remote_server();
extern int deattach_remote_server();
extern int recv_remote_server();


extern int remote_fd;

#endif
