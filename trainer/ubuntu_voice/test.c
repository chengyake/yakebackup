#include <stdio.h>
#include <alsa/asoundlib.h>  

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_CAPTURE_SEC    (10000000)  //10s

long loops;
int fd;
snd_pcm_t *handle;
snd_pcm_hw_params_t *params;



void make_file_name_by_time(char *data) {
    
    time_t tmp;
    struct tm *p;
    
    if(data == NULL) {
        printf("params of %s error", __func__);
        exit(-1);
    }

    tmp = time(NULL);
    p=localtime((time_t *)&tmp);
    sprintf(&data[0], "%4d%02d%02d_%02d%02d%02d.wav",
        (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    
}


int open_file() {
    char file_name[256]={0};
    
    make_file_name_by_time(&file_name[0]);

    fd = open(&file_name[0], O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if(fd < 0) {
        printf("%s error", __func__);
    }

    return fd;
}


int main() {

    int ret;

    //fill name
    char *pcm_name;
    //pcm_name = strdup("plughw:0,0");
    pcm_name = strdup("hw:0,0");

    //open
    //ret = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);
    ret = snd_pcm_open(&handle, pcm_name, SND_PCM_STREAM_CAPTURE, 0);
    if (ret <0 ) {
        printf("unable to open pcm device: %s\n", snd_strerror(ret));
        exit(1);
    }
    
    //get
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    
    //fmt
    int dir;
    unsigned int interval = 44100;
    snd_pcm_uframes_t frames = 32;
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, 2);
    snd_pcm_hw_params_set_rate_near(handle, params, &interval, &dir);
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
    
    //set
    ret = snd_pcm_hw_params(handle, params);
    if (ret <0 ) {
        printf("unable to set hw parameters: %s\n", snd_strerror(ret));
        exit(1);
    }
    
    //get buffer size
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    int size = frames * 2 * 2;//16 bit 2 channel
    char *buffer = (char *) malloc(size);

    snd_pcm_hw_params_get_period_time(params, &interval, &dir);


    //while (1) {

        loops = ALSA_CAPTURE_SEC / interval;
        fd = open_file();
        while (loops > 0) {
            loops--;
            ret = snd_pcm_readi(handle, buffer, frames);
            if (ret == -EPIPE) {
                /* EPIPE means overrun */
                printf("read failed\n");
                snd_pcm_prepare(handle);
            } else if (ret <0) {
                printf("read error from: %s\n", snd_strerror(ret));
            } else if (ret != (int)frames) {
                printf("try read again, now read %d frames\n", ret);
            }
            ret = write(fd, buffer, size);
            if (ret != size)
                printf("write error: wrote %d bytes\n", ret);
        }
        close(fd);
    //}

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);

    return 0;
}

