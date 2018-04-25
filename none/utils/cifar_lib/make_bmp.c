#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int bmp_write(unsigned char *image, int xsize, int ysize, char *filename)
{
    unsigned char header[54] = {
      0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0,
        54, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0
    };
  
    long file_size = (long)xsize * (long)ysize * 3 + 54;
    header[2] = (unsigned char)(file_size &0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;
  
    long width = xsize;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) &0x000000ff;
    header[20] = (width >> 16) &0x000000ff;
    header[21] = (width >> 24) &0x000000ff;
  
    long height = ysize;
    header[22] = height &0x000000ff;
    header[23] = (height >> 8) &0x000000ff;
    header[24] = (height >> 16) &0x000000ff;
    header[25] = (height >> 24) &0x000000ff;

    char fname_bmp[128];
    sprintf(fname_bmp, "%s.bmp", filename);
  
    FILE *fp;
    if (!(fp = fopen(fname_bmp, "wb")))
      return -1;
     
    fwrite(header, sizeof(unsigned char), 54, fp);
    fwrite(image, sizeof(unsigned char), (size_t)(long)xsize * ysize * 3, fp);
  
    fclose(fp);
    return 0;
}


int main() {
    
    int v = 6;
    int fd;
    int i, j;
    unsigned char idx;
    //s, h, b, g, r
    unsigned char data[32*32*3]={0};
    unsigned char image[32][32][3]={0};
    


    fd = open("data_batch_1.bin", O_RDWR, S_IRUSR);
    
    
    lseek(fd, 3073*v, SEEK_SET);
    read(fd, &idx, 1);
    printf("we get idx %d\n", idx);
    read(fd, (unsigned char *)&data[0], 32*32*3);
    
    //(R*299 + G*587 + B*114 + 500) / 1000


    /*
    //fill b, g, r
    for(i=0; i<32; i++) {
        for(j=0; j<32; j++) {
            image[31-i][31-j][2] = data[i*32+j];
            image[31-i][31-j][1] = data[i*32+j+32*32];
            image[31-i][31-j][0] = data[i*32+j+32*32*2];
        }
    }*/

    //fill b, g, r
    for(i=0; i<32; i++) {
        for(j=0; j<32; j++) {
            int r,g,b,s;
            r=data[i*32+j];
            g=data[i*32+j+32*32];
            b=data[i*32+j+32*32*2];
            s=(r*299+g*587+b*114+500)/1000;
            s=s>255? 255:s;
            
            #if 0
            if(s > 192) s = 255;
            if(s > 128 && s < 192) s = 192;
            if(s > 64 && s < 128) s = 128;
            if(s >= 0 && s < 64) s = 64;
            #endif
            #if 0
            if(s > 224) s = 255;
            if(s > 192 && s < 224) s = 224;
            if(s > 160 && s < 192) s = 192;
            if(s > 128 && s < 160) s = 160;
            if(s > 96 && s < 128) s = 128;
            if(s > 64 && s < 96) s = 96;
            if(s > 32 && s < 64) s = 64;
            if(s > 0 && s < 32) s = 32;
            #endif


            image[31-i][j][2] = s;
            image[31-i][j][1] = s;
            image[31-i][j][0] = s;
        }
    }


    /*
    for(i=0; i<32; i++) {
        image[10][i][0] = 255;//blue
        if(i > 10) {
            image[16][i][1] = 255;//green
        }
        if(i < 22) {
            image[24][i][2] = 255;//red
        }
    }
    */

    bmp_write((unsigned char *)&image[0][0][0], 32, 32, "test");

}






