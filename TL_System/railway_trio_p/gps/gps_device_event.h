#ifndef GPS_DEVICE_EVENT_H
#define GPS_DEVICE_EVENT_H




#pragma pack (1)
struct latitude_t {
    unsigned char degree[2];      //2
    unsigned char minute[2];     //2
    unsigned char cut;          //1     '.'
    unsigned char fraction[4];      //4
};
#pragma pack ()

#pragma pack (1)
struct longitude_t {
    unsigned char degree[3];    //3
    unsigned char minute[2];     //2
    unsigned char cut;          //1     '.'
    unsigned char fraction[4];      //4
};
#pragma pack ()

#pragma pack (1)
struct sc_t {
    unsigned char head1;        //1     0xaa
    unsigned char head2;        //1     0x55
    unsigned char local_addr;   //1     0x3a
    unsigned int  time;         //4     
    unsigned short speed;       //2
    unsigned short signal_id;   //2
    unsigned char kmposition[3];//3
    unsigned char incordec;     //1
    unsigned short train_id;    //2
    unsigned char section_id;   //1
    unsigned char station_id;   //1
    struct latitude_t latitude; //9
    struct longitude_t longitude;//10
    unsigned char xor_byte;     //1

};                              //38        500ms
#pragma pack ()



#pragma pack (1)
struct rcats2_t {
    unsigned char head1;        //1     0xca
    unsigned char status;       //1     0xff ok, 0x0f TAX break, 0xf0 GPS break, 0x00 all break;
    unsigned char reserve1;     //1
    unsigned int time;          //4     
    unsigned short speed;       //2
    unsigned short signal_id;   //2
    unsigned char kmposition[3];//3
    unsigned char reserve2;     //1
    unsigned short reserve3;    //2
    unsigned char reserve4;     //1
    unsigned char reserve5;     //1
    struct latitude_t latitude; //9
    struct longitude_t longitude;//10
    unsigned char xor_byte;     //1

};                              //38    450ms
#pragma pack ()






extern struct gps_info_t gps_info;
extern int gps_status;

#endif




