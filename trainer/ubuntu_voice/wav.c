#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#include "wav.h"

//ffmpeg -i hello.aac -acodec pcm_s16le -ac 1 -ar 8000 -vn out.wav
#pragma pack (2)
struct wav_header {
    char riff[4];
    int  riff_length;
    //
    char wave[4];
    char fmt[4];
    int  fmt_length;
    short type;
    short channel;
    int  sample;
    int  bps;
    short adjust;
    short bits;
    //
    char list[4];
    int  list_size;
    char no_null[26];
    //
    char data[4];
    int data_size;
};
#pragma pack ()

int get_wav_info(char *name) {

    int fd;
    struct wav_header header;
    
    if(name == NULL) {
        printf("NULL point in name\n");
    }

    fd = open(name, O_RDONLY, S_IRUSR|S_IWUSR);
    if(fd < 0) {
        printf("open %s error\n", name);
        return fd;
    }
    
    read(fd, &header, sizeof(struct wav_header));

    
    printf("%c%c%c%c\n", header.riff[0], header.riff[1], header.riff[2], header.riff[3]);
    printf("riff length: %d\n", header.riff_length);
    printf("\t%c%c%c%c:\n", header.wave[0], header.wave[1], header.wave[2], header.wave[3]);
    printf("\t%c%c%c:\n", header.fmt[0], header.fmt[1], header.fmt[2]);
    printf("\t\tfmt length\t:%d\n", header.fmt_length);
    printf("\t\ttype\t:%d\n", header.type);
    printf("\t\tchannel\t:%d\n", header.channel);
    printf("\t\tsample\t:%d\n", header.sample);
    printf("\t\tbps\t:%d\n", header.bps);
    printf("\t\tadjust\t:%d\n", header.adjust);
    printf("\t\tbits\t:%d\n", header.bits);

    printf("\t%c%c%c%c:\n", header.list[0], header.list[1], header.list[2], header.list[3]);
    printf("\t\tlist length \t:%d\n", header.list_size);

    printf("\t%c%c%c%c:\n", header.data[0], header.data[1], header.data[2], header.data[3]);
    printf("\t\tdata length \t:%d\n", header.data_size);

    close(fd);
}


int get_wav_data(int idx, short *data, int length) {

    int fd;
    struct wav_header header;
    
    fd = open("hello.wav", O_RDONLY, S_IRUSR|S_IWUSR);
    if(fd < 0) {
        printf("open %s error\n", "hello.wav");
        return fd;
    }
    
    lseek(fd, sizeof(struct wav_header) + idx*length/2, SEEK_SET);

    read(fd, data, length);

    close(fd);
}

/*
int main() {
    int i;
    short data[54614]={0};
    //get_wav_info("hello.wav");
    get_data(0, data, 54614);
    for(i=0; i<54164; i++) {
        printf("%d\n", data[i]);
    }
    //printf("get over\n");
    return 0;
}
*/


/*
RIFF
riff length: 54684
	WAVE:
	fmt:
		fmt length	:16
		type	:1
		channel	:1
		sample	:8000
		bps	:16000
		adjust	:2
		bits	:16
	LIST:
		list length 	:26
	data:
		data length 	:54614
*/
