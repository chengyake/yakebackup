#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>

#include "../core/core.h"

#include "remote_client_connection.h"
#include "remote_client_task_event.h"
#include "hal-triorail.h"
#include "client_task_queue.h"
#include "local_client_event.h"


static int token=0; 
static struct tele_switch_t tele;
static int dial_first=0;

struct gps_info_t gps={0.0, 0.0, 0, 0};
int module_status = REPORT_STATUS;
static int gps_f=0, gprs_f=0, trio0_f=0, trio1_f=0, trace_f=0;

int update_tele_state(enum tele_state_t state) {

    if(state == tele.state) {
        return 0;
    }
    tele.state = state;

    if(state != TELE_WAIT) {
        tele.begin = time(NULL);
    }

    return 0;
}



int update_module_status() {
    
    if(trio0_f != 0) {
        syslog(LOG_ERR, "get trio 0 status error");
        return trio0_f;
    }
    if(trio1_f != 0) {
        syslog(LOG_ERR, "get trio 1 status error");
        return trio1_f;
    }
    if(trace_f != 0) {
        syslog(LOG_ERR, "get tarce status error");
        return trace_f;
    }
    if(gps_f != 0) {
        syslog(LOG_ERR, "get gps status error");
        return gps_f;
    }
    if(gprs_f != 0) {
        syslog(LOG_ERR, "get gprs status error");
        return gprs_f;
    }

    return REPORT_STATUS;
}

int register_client_module() {

    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "client register to core");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = IDX_FRAMEWORK_CORE;
    cmd_rsp.cmd = CORE_MSG_REGISTER_CMD;
    cmd_rsp.len = 0;

    return write_local_server((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}

int get_gps_status() {

    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "client register to core");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = IDX_DEVICE_GPS;
    cmd_rsp.cmd = GPS_MSG_GET_STATUS_CMD;
    cmd_rsp.len = 0;

    return write_local_server((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}


int get_gprs_status() {
#if 0
    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "client register to core");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = IDX_DEVICE_GPRS;
    cmd_rsp.cmd = GPS_MSG_GET_STATUS_CMD;
    cmd_rsp.len = 0;

    return write_local_server((char *)(&cmd_rsp), CMD_HEADER_SIZE);
#endif
}


int get_trio0_status() {

    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "client register to core");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = IDX_DEVICE_TRIO;
    cmd_rsp.cmd = TRIO_MSG_0_STATUS_REQ;
    cmd_rsp.len = 0;

    return write_local_server((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}
int get_trio1_status() {

    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "client register to core");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = IDX_DEVICE_TRIO;
    cmd_rsp.cmd = TRIO_MSG_1_STATUS_REQ;
    cmd_rsp.len = 0;

    return write_local_server((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}

int get_trace_status() {

    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "client register to core");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = IDX_DEVICE_TRIO;
    cmd_rsp.cmd = TRIO_MSG_0_TRACE_STATUS_REQ;
    cmd_rsp.len = 0;

    return write_local_server((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}

static int register_client_module_rsp() {
    
    syslog(LOG_INFO, "register client to core success");
    return 0;
}


int dial_tele0(unsigned char*num, unsigned int len) {
    
    int i;
    unsigned char bf[256];
    
    if(len > 15) {
        syslog(LOG_ERR, "dial tele0 error");
        return -1;
    }

    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "#dial");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = IDX_DEVICE_TRIO;
    cmd_rsp.cmd = TRIO_MSG_0_DIAL_REQ;
    cmd_rsp.len = 24+len+1;//'\0'
    memcpy(bf, &cmd_rsp, sizeof(struct cmd_header));

    RIL_dial_req dial_req;
    dial_req.token = token++;
    dial_req.clir = 0;
    dial_req.addressLen = len;
    memcpy(&bf[CMD_HEADER_SIZE], &dial_req, sizeof(dial_req));

    memcpy(&bf[CMD_HEADER_SIZE+sizeof(RIL_dial_req)], num, len+1);

    return write_local_server(&bf[0], CMD_HEADER_SIZE + cmd_rsp.len);
}


int hangup_tele0(unsigned char *num, unsigned int len) {

    unsigned char bf[256];

    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "#hanup");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = IDX_DEVICE_TRIO;
    cmd_rsp.cmd = TRIO_MSG_0_HANGUP_REQ;
    cmd_rsp.len = sizeof(RIL_hangup_req);
    memcpy(bf, &cmd_rsp, sizeof(struct cmd_header));

    RIL_hangup_req hangup_req;
    hangup_req.token = token++;
    hangup_req.callID = 1; //callID

    memcpy(&bf[sizeof(struct cmd_header)], &hangup_req, sizeof(hangup_req));

    return write_local_server(&bf[0], CMD_HEADER_SIZE+sizeof(hangup_req));
}

int send_task_cfg(struct task_t *t) {
   unsigned char bf[256];

    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "#send task cfg");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = IDX_DEVICE_TRIO;
    cmd_rsp.cmd = TRIO_MSG_0_TASK_CFG_CMD;
    cmd_rsp.len = sizeof(RIL_task_config_ind);
    memcpy(bf, &cmd_rsp, CMD_HEADER_SIZE);

    t->cfg.token = token++;

    memcpy(&bf[sizeof(struct cmd_header)], &t->cfg, sizeof(RIL_task_config_ind));

    return write_local_server(&bf[0], CMD_HEADER_SIZE+sizeof(RIL_task_config_ind));

}


int start_trace(struct task_t * t) {
    
    int file_name_len;
    char file_name[128];
    unsigned char bf[512]={0};

    if(t == NULL) {
        syslog(LOG_ERR, "start trace error");
        return -1;
    }
    
    if(send_task_cfg(t) < 0) {
        return -1;
    }
     
    file_name_len = get_upload_file_name(t, &file_name[0]);

    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "#start trace");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = IDX_DEVICE_TRIO;
    cmd_rsp.cmd = TRIO_MSG_0_TRACE_START_REQ;
    cmd_rsp.len = 16+file_name_len+1 + t->task.file_name_len + t->task.file_name_len;//'\0 eof'
    memcpy(&bf[0], &cmd_rsp, sizeof(struct cmd_header));

    RIL_trace_start_req trace_start_req;
    trace_start_req.token = token++;
    trace_start_req.traceFileNameLen = file_name_len+1;
    trace_start_req.railLineLen = t->task.file_name_len;
    trace_start_req.railwayBureauLen = t->task.file_name_len;

    memcpy(&bf[CMD_HEADER_SIZE], &trace_start_req, sizeof(RIL_trace_start_req));
    memcpy(&bf[CMD_HEADER_SIZE+sizeof(RIL_trace_start_req)], file_name, trace_start_req.traceFileNameLen);
    memcpy(&bf[CMD_HEADER_SIZE+sizeof(RIL_trace_start_req)+trace_start_req.traceFileNameLen], t->task.file_name, trace_start_req.railLineLen);
    memcpy(&bf[CMD_HEADER_SIZE+sizeof(RIL_trace_start_req)+trace_start_req.traceFileNameLen+trace_start_req.railLineLen],
            t->task.file_name, trace_start_req.railwayBureauLen);

    
    write_local_server(&bf[0], sizeof(struct cmd_header) + cmd_rsp.len);

    return 0;
}


int stop_trace(unsigned char *file, unsigned int len) {
    
    int i;
    unsigned char bf[512]={0};

    if(len > 255) {
        syslog(LOG_ERR, "stop trace error");
        return -1;
    }

    if(tele.state == TELE_ONLINE) {
        struct task_t *t = get_running_task();
        update_tele_state(TELE_WAIT);
        hangup_tele0(&t->phone_num[0], t->phone_len);
    }

    struct cmd_header cmd_rsp;
    syslog(LOG_INFO, "#stop trace");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = IDX_DEVICE_TRIO;
    cmd_rsp.cmd = TRIO_MSG_0_TRACE_STOP_REQ;
    cmd_rsp.len = sizeof(RIL_trace_end_req) + len + 1;//'\0'
    memcpy(&bf[0], &cmd_rsp, sizeof(struct cmd_header));

    RIL_trace_end_req trace_stop_req;
    trace_stop_req.token = token++;
    trace_stop_req.traceFileNameLen = len+1;

    memcpy(&bf[sizeof(struct cmd_header)], &trace_stop_req, sizeof(RIL_trace_end_req));

    memcpy(&bf[CMD_HEADER_SIZE+sizeof(RIL_trace_end_req)], file, len+1);


    return write_local_server(&bf[0], CMD_HEADER_SIZE + cmd_rsp.len);
}




int get_client_status_rsp(struct cmd_header *chp) {
    struct cmd_header cmd_rsp;

    syslog(LOG_INFO, "get client statue rsp");
    cmd_rsp.host = IDX_USER_CLIENT;
    cmd_rsp.target = chp->host;
    cmd_rsp.cmd = CLIENT_MSG_GET_STATUS_RSP;
    cmd_rsp.len = 0;

    return write_local_server((char *)(&cmd_rsp), CMD_HEADER_SIZE);
}


int64_t get_local_interval() {
    int64_t tmp;
    struct task_t *t;
    
    t = get_running_task();

    if(t == NULL) {
        return REPORT_INTERVAL_SEC;
    }

    tmp = time(NULL) - tele.begin; 
    if(tmp < 0) {
        syslog(LOG_ERR, "get local interval err");
        return REPORT_INTERVAL_SEC;
    }


    if(tele.state == TELE_IDLE) {
        return REPORT_INTERVAL_SEC;
    }
    if(tele.state == TELE_WAIT) {
        return 1;
    }


    if(tele.state == TELE_OFFLINE) {
        tmp = t->offline_interval - tmp;
        return (tmp>0?tmp:1);
    }
    if(tele.state == TELE_ONLINE) {
        tmp = t->online_interval - tmp;
        return (tmp>0?tmp:1);
    }
}


int check_and_process_local_task() {
    int64_t tmp;
    struct task_t *t = get_running_task();

    if(t == NULL) {
        return 0;
    }

    if(t->status == TASK_TESTING_AND_UPLOADING) {

        tmp = time(NULL) - tele.begin;
        if(tmp < 0) {
            syslog(LOG_ERR, "get gps data error: len = 0");
            return -1;
        }

        if(t->online_interval == 0) {
            if(tele.state == TELE_OFFLINE) {
                    update_tele_state(TELE_WAIT);
                    dial_tele0(t->phone_num, t->phone_len);
            }
        } else {
            if(tele.state == TELE_OFFLINE) {
                if(tmp >= t->offline_interval || dial_first) {
                    dial_first = 0;
                    update_tele_state(TELE_WAIT);
                    dial_tele0(t->phone_num, t->phone_len);
                }
            } else if(tele.state == TELE_ONLINE) {
                if(tmp >= t->online_interval) {
                    update_tele_state(TELE_WAIT);
                    hangup_tele0(t->phone_num, t->phone_len);
                }
            }
        }
    }

    return 0;
}


int process_local_event(struct cmd_header *chp) {

    int ret=0;
    struct task_t *t;

    print_local_frame(chp);
    uint32_t *rsp = (uint32_t *)&chp->data[0];
    switch (chp->cmd) {

        case GPS_MSG_GET_GPS_INFO_RSP:
            if(chp->len <= 0) {
                syslog(LOG_ERR, "get gps data error: len = 0");
                break;
            }
            memcpy(&gps, &chp->data[0], chp->len);
            printf("---------%llf, %llf, %llf, %llf\n", gps.longitude, gps.latitude, gps.speed, gps.kmposition);
            break;
        case TRIO_MSG_0_DIAL_RSP:
            if(rsp[1] == 0) {
                update_tele_state(TELE_ONLINE);
            } else {
                update_tele_state(TELE_OFFLINE);
            }
            break;

        case TRIO_MSG_0_HANGUP_RSP:
            update_tele_state(TELE_OFFLINE);
            break;

        case TRIO_MSG_0_NO_CAR_IND:
            update_tele_state(TELE_OFFLINE);
            break;

        case TRIO_MSG_0_TRACE_START_RSP:
            dial_first = 1;
            update_tele_state(TELE_OFFLINE);
            break;
        case TRIO_MSG_0_TRACE_STOP_RSP:
            update_tele_state(TELE_IDLE);
            break;

            //get module status rsp
        case GPS_MSG_GET_STATUS_RSP:
            if(rsp[0] != 0) {
                gps_f = rsp[0];
            } else {
                gps_f = 0;
            }
        case GPRS_MSG_GET_STATUS_RSP:
            if(rsp[0] != 0) {
                gprs_f = rsp[0];
            } else {
                gprs_f = 0;
            }
        case TRIO_MSG_0_STATUS_RSP:
            if(rsp[0] != 0) {
                trio0_f = rsp[0];
            } else {
                trio0_f = 0;
            }
        case TRIO_MSG_1_STATUS_RSP:
            if(rsp[0] != 0) {
                trio1_f = rsp[0];
            } else {
                trio1_f = 0;
            }
        case TRIO_MSG_0_TRACE_STATUS_RSP:
            if(rsp[0] != 0) {
                trace_f = rsp[0];
            } else {
                trace_f = 0;
            }
            break;
        default:
            ret = 0;
            break;
    }
    return ret;
}


