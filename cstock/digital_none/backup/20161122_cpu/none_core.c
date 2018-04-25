#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <linux/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <omp.h> 

#include "none_core.h"

#ifdef USE_GPU
#include <opencl.h>
#endif


struct input_table_t input_data[300];
float sum[NOA], mid[NOA];
struct cell_t *all_table;
int rd;


struct timeval t_start,t_end;
long cost_time = 0;


unsigned short debug_sta;


#ifdef USE_GPU
int ret;

size_t global_work_size[2] = {512, 256};
size_t local_work_size[2] = {16, 16};


static cl_context context;
static cl_command_queue commandQueue;
static cl_program program;
static cl_kernel kernel;
static cl_mem inputBuffer_i;
static cl_mem inputBuffer_q;
static cl_mem inputBuffer_o;
#endif



int init_random() {
    rd = open ("/dev/urandom", O_RDONLY);
    if (rd <= 0) {
        printf("open /dev/urandom error\n");
        exit(0);
    }
    return 0;
}

int get_random(unsigned char *dst, unsigned int num) {
#ifdef RANDOM
    int ret;
    ret = read(rd, dst, num);
    if(ret < 0) {
        printf("get random num error when read /dev/urandom\n");
        return -1;
    }
    return ret;
#else
    int i, *p;
    int seed[2], ret;
    ret = read(rd, &seed, 8);
    srand(seed[1]);
    p = (int *)dst;
    for(i=seed[2]%num; i<num; i+=4) {
        *p = rand();
        p++;
    }
    for(i=0; i<seed[2]%num; i+=4) {
        *p = rand();
        p++;
    }

    return ret;
#endif
}

void close_random () {
    close (rd);
    rd = 0;
}


void init_input() {
    int f;
    int len;
    int ret;

    f = open("./org_data/hs300_20100101_20160531_pre.db",  O_RDONLY, S_IRUSR|S_IWUSR);
    if(f < 0) {
        printf("open db error\n");
        exit(0);
    }
    
    len = lseek(f, 0, SEEK_END);
    if(len < 0) {
        printf("seek db end error\n");
        exit(0);
    }

    ret = lseek(f, 0, SEEK_SET);
    if(ret < 0) {
        printf("seek db set error\n");
        exit(0);
    }

    ret = read(f, input_data, len);
    
    //printf("%s %f %f\n", input_data[299].code, input_data[299].avg[0], input_data[299].avg[1555]);
    close(f);
}




void init_cells() {

    int ret, i;
    int fd;

    if(access(FILE_OF_MATRIX, F_OK|R_OK|W_OK) != 0) { //don't exist
        struct cell_t data;
        fd = open(FILE_OF_MATRIX, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            printf("create %s error\n", FILE_OF_MATRIX);
            exit(0);
        }
        memset(&data, 0, sizeof(data));
        memset(data.sta, 11, NOA*41);
        for(i=0; i<NOA; i++) {
            data.idx = i;
            ret = write(fd, &data, SOC);
            if(ret < 0) {
                printf("write TDataBase.db error\n");
                exit(0);
            }
        }
    } else {
        fd = open(FILE_OF_MATRIX, O_RDWR, S_IRUSR|S_IWUSR);
        if(fd < 0) {
            printf("open %s error\n", FILE_OF_MATRIX);
            exit(0);
        }
    }

    ret = lseek(fd, 0, SEEK_SET);
    if(ret < 0) {
        printf("create %s fixed space error\n", FILE_OF_MATRIX);
        exit(0);
    }

    all_table = (struct cell_t *) mmap(0, NOA*SOC, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(all_table == MAP_FAILED) {
        printf("mmap %s error: %d\n", FILE_OF_MATRIX, errno);
        exit(0);
    }
    close(fd);
    return;

}

void close_cells() {
    if(all_table != NULL) {
        msync((void *)all_table, NOA*SOC, MS_SYNC);
        munmap((void *)all_table, NOA*SOC);
        all_table = NULL;
    }
}

static int read_cl(char *filename, char *text) {

    int fd, len, size;
    fd = open(filename, O_RDONLY);
    if(fd < 0) {
        printf("MU1 Error: open kernel CL file error\n");
        return fd;
    }
    len = lseek(fd, 0, SEEK_END);
    if(len < 0) {
        printf("MU1 Error: get file length error\n");
        close(fd);
        return len;
    }
    lseek(fd, 0, SEEK_SET);
    size = read(fd, text, len);
    if(len != size) {
        printf("MU1 Error: file len %d   %d\n", len, size);
        return -1;
    }
    text[size]='\0';

    printf("MU1: read cl success\n");

    return 0;
}


#ifdef USE_GPU
int32_t init_kernel_platform() {

	cl_uint plat_num;
	cl_platform_id plat_id = NULL;
	cl_uint dev_num = 0;
	cl_device_id *devices;

	ret = clGetPlatformIDs(0, NULL, &plat_num);
	if (ret < 0) {
		printf("MU1 Error: Getting plat_ids!\n");
		return -1;
	}

	if(plat_num > 0)
	{
		cl_platform_id* plat_ids = (cl_platform_id* )malloc(plat_num* sizeof(cl_platform_id));
		ret = clGetPlatformIDs(plat_num, plat_ids, NULL);
		plat_id = plat_ids[0];
		free(plat_ids);
	}

	ret = clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_GPU, 0, NULL, &dev_num);	
	if (dev_num == 0) {
		printf("MU1: No GPU device available.\n");
		printf("MU1: Choose CPU as default device.\n");
		ret = clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_CPU, 0, NULL, &dev_num);	
		devices = (cl_device_id*)malloc(dev_num * sizeof(cl_device_id));
		ret = clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_CPU, dev_num, devices, NULL);
	} else {
		printf("MU1: Choose GPU as default device. dev_num %d\n", dev_num);
		devices = (cl_device_id*)malloc(dev_num * sizeof(cl_device_id));
		ret = clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_GPU, dev_num, devices, NULL);
	}
	
	context = clCreateContext(NULL,1, devices,NULL,NULL,NULL);

	commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
    
	char filename[] = "core.cl";
	char file_context[10*1024]={0};
	const char *source = &file_context[0];

    ret = read_cl(filename, &file_context[0]);

	size_t sourceSize[10] = {strlen(source)};
	cl_program program = clCreateProgramWithSource(context, 1, &source, &sourceSize[0], NULL);
	
	ret = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
    if(ret < 0) {
        printf("MU1 Error: clBuildProgram error\n");
        return 0;
    }

	kernel = clCreateKernel(program, "process_iq", NULL);

	inputBuffer_i = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
            512*1024*4, (void *)(&table_i[0][0]), NULL);
	inputBuffer_q = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
            512*1024*4, (void *)(&table_q[0][0]), NULL);
	inputBuffer_o = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, 
            512*1024*4, (void *)(&table_o[0][0]), NULL);


	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inputBuffer_i);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&inputBuffer_q);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&inputBuffer_o);



    if(devices != NULL) { free(devices);}

    printf("MU1: init cl plat success\n");
    return 0;
}

static int32_t run_kernel() {

    ret = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    //ret = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if(ret < 0) {
        printf("MU1 Error: clEnqueueNDRangeKernel error\n");
    }

    clFinish(commandQueue);

    return ret;

}

int32_t close_kernel_platform() {

	ret = clReleaseKernel(kernel);				
	ret = clReleaseProgram(program);	
	ret = clReleaseMemObject(inputBuffer_i);
	ret = clReleaseMemObject(inputBuffer_q);
	ret = clReleaseMemObject(inputBuffer_o);
	ret = clReleaseCommandQueue(commandQueue);
	ret = clReleaseContext(context);

    printf("release kernel\n");

    return 0;
}

#endif

void input_none(int ticks) {
    int i;
    for(i=0; i<NOI; i++) {
        if(input_data[i].avg[ticks-1]==0.0) {
            sum[i] = 0.0;
        } else {
            sum[i] = (input_data[i].avg[ticks] - input_data[i].avg[ticks-1])*100.0/input_data[i].avg[ticks-1];
            if(sum[i] > 10.0) {
                sum[i] = 10.0;
            } else if(sum[i] < -10.0) {
                sum[i] = -10.0;
            }
        }
    }
}

unsigned short rdata[NOA][NOA];

/*
 *
 *      99% proccess
 *
 */
void mid_to_table(int first) {
    int i, j, k;
    memset(sum, 0, sizeof(sum));
    for(i=0; i<NOA; i++) { if(j>=NOI&&j<NOO+NOI){continue;}
        #pragma omp parallel for
        for(j=NOI; j<NOA; j++) { if(i==j) {continue;}
            if(first) {
                int s=0, y=0;
                for(k=0; k<41; k++) {
                    s += all_table[i].sta[j][k];
                }
                y=rdata[i][j]%s; //posibility error because 256%41

                for(k=0; k<41; k++) {
                    if(y>all_table[i].sta[j][k]) {
                        y-=all_table[i].sta[j][k];
                    } else {
                        all_table[i].dis[j] = k;
                        break;
                    }
                }
            }
            //table to sum
            sum[j]+=mid[i]*(all_table[i].dis[j]-20.0)*0.1;
        }
    }
}

void table_to_sum() {

}

void output_none(int ticks) {
    
    int i,idx=0;
    float maxf=-1000.0;
    for(i=0; i<NOO; i++) {
        if(maxf<sum[i+NOI]) {
            maxf = sum[i+NOI];
            idx = i;
        }
    }
    
    printf("ticks %4d idx %d\n", ticks, idx);
    //check and feedback //if latest; output my stock
    if(ticks >= 1555) {
        printf("active percent %d%% and %s selected\n",debug_sta, input_data[idx].code);
        return;
    }
    float f=0.0;
    if(input_data[idx].avg[ticks]!=0.0) {
        f = (input_data[idx].avg[ticks+1]-input_data[idx].avg[ticks])*100.0/input_data[idx].avg[ticks];
    }
    char fb = (char)f;
    if(fb > 0) {debug_sta++;}
    if(fb != 0) {
        int x, y, z;
        unsigned char max=0;
        printf("fb %f %d\n",f, fb);
        for(x=0; x<NOA; x++) { if(y>=NOI&&y<NOO+NOI) {continue;}
            for(y=NOI; y<NOA; y++) { if(x==y) {continue;}
                int tmp = all_table[x].sta[y][all_table[x].dis[y]]+fb;
                if(tmp >= 1) {
                    all_table[x].sta[y][all_table[x].dis[y]] = tmp;
                } else {
                    all_table[x].sta[y][all_table[x].dis[y]] = 1;
                }
                for(z=0; z<41; z++) {
                    max = max>all_table[x].sta[y][z]?max:all_table[x].sta[y][z];
                }
                
                if(max >= 245) {
                    for(z=0; z<41; z++) {
                        all_table[x].sta[y][z]=(all_table[x].sta[y][z]+1)/2;
                    }
                } else if(max <= 80) {
                    for(z=0; z<41; z++) {
                        all_table[x].sta[y][z]=(all_table[x].sta[y][z])*2;
                    }
                }

            }
        }
    }
}

void sum_to_mid() {
    int i;
    float max=0.0;
    for(i=NOI; i<NOA; i++) {
        max = max>=abs(sum[i]) ? max : abs(sum[i]);
    }
    if(max!=0.0) {
        for(i=NOI; i<NOA; i++) {
            sum[i] = sum[i]*10.0/max;
        }
    }
    memcpy(mid, sum, sizeof(mid));
}

void run() {

    int i;
    int ticks=0;
    
    debug_sta=0;
    memset(sum, 0, sizeof(sum));
    memset(mid, 0, sizeof(mid));
    for(ticks=1; ticks<1556; ticks++) {   //in -1; fb -1; //1556 for output
        get_random((char *)&rdata[0][0], sizeof(rdata));  //71ms
        for(i=0; i<20; i++) {             //20 fixed
            input_none(ticks);
            sum_to_mid();
            mid_to_table(i==0?1:0);       //158ms
            table_to_sum();
        }
        output_none(ticks);               //10s
    }
    msync((void *)all_table, NOA*SOC, MS_ASYNC);
}

void main(int argc, char *argv[]) {
    int counter, times;
    if(argc != 2) {
        printf("Get parameter first please!\n");
        return ;
    }
    times = atoi(argv[1]);
    if(times <=0) {
        printf("Get param error\n");
        return;
    }

    init_random();
    init_input();
    init_cells();

    //init_kernel_platform();
    //gettimeofday(&t_start, NULL);
    for(counter=0; counter<times; counter++) {
        run();
    }
    //gettimeofday(&t_end, NULL);
    //close_kernel_platform();

    //printf("exec digital exec success\n");
    //cost_time = (t_end.tv_sec*1000000l+t_end.tv_usec) - (t_start.tv_sec*1000000l+t_start.tv_usec);
    //printf("run Cost time: %lds+%ldus\n", cost_time/1000000, cost_time%1000000);

    close_cells();
    close_random();
    printf("%d times exec over\n", times);
}














