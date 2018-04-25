
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
//#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>

#include "none_config.h"
#include "none_debug.h"
#include "none_matrix.h"
#include "none_core.h"

uint64_t ticks;
struct kernel_parameter param;
enum none_system_state state;

static char *rdp;
static sem_t suspend_sem;

static cl_context context;
static cl_command_queue commandQueue;
static cl_kernel kernel;
static cl_program program;
static cl_mem inputBuffer;
static cl_mem randomBuffer;
static cl_mem parameterBuffer;


static int32_t start_kernel_loop_in_thread(void);
static int32_t set_random_to_kernel(void);

static int read_cl(char *filename, char *text) {

    int fd, len, size;
    fd = open(filename, O_RDONLY);
    if(fd < 0) {
        printf("Error: open kernel CL file error\n");
        return fd;
    }
    len = lseek(fd, 0, SEEK_END);
    if(len < 0) {
        printf("Error: create TDataBase.db fixed space error\n");
        close(fd);
        return len;
    }
    lseek(fd, 0, SEEK_SET);
    size = read(fd, text, len);
    if(len != size) {
        printf("Error: file len %d   %d\n", len, size);
        return -1;
    }
    text[size]='\0';

    return 0;
}

int32_t init_kernel_platform() {

    cl_int ret;
	cl_uint plat_num;
	cl_platform_id plat_id = NULL;
	cl_uint dev_num = 0;
	cl_device_id *devices;

	ticks = 0;
	state = INIT_STATE;


	param.fb = 0;
	param.stage = STAGE_E;

    rdp = (char *)malloc(sizeof(int32_t)*YUAN_NUM_OF_ALL*YUAN_NUM_OF_ALL);
    if(rdp == NULL) {
		printf("Error: Getting random data space!\n");
    }


	ret = clGetPlatformIDs(0, NULL, &plat_num);
	if (ret < 0) {
		printf("Error: Getting plat_ids!\n");
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
		printf("\tNo GPU device available.\n");
		printf("\tChoose CPU as default device.\n");
		ret = clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_CPU, 0, NULL, &dev_num);	
		devices = (cl_device_id*)malloc(dev_num * sizeof(cl_device_id));
		ret = clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_CPU, dev_num, devices, NULL);
	} else {
		devices = (cl_device_id*)malloc(dev_num * sizeof(cl_device_id));
		ret = clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_GPU, dev_num, devices, NULL);
	}
	
	context = clCreateContext(NULL,1, devices,NULL,NULL,NULL);
	
	//commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
	commandQueue = clCreateCommandQueueWithProperties(context, devices[0], 0, NULL);
    
	char filename[] = "core_kernel.cl";
	char file_context[10*1024]={0};
	const char *source = &file_context[0];

    ret = read_cl(filename, &file_context[0]);

	size_t sourceSize[10] = {strlen(source)};
	cl_program program = clCreateProgramWithSource(context, 1, &source, &sourceSize[0], NULL);
	
	ret = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
    if(ret < 0) {
        printf("Error: clBuildProgram error\n");
        return 0;
    }

	kernel = clCreateKernel(program, "process_matrix", NULL);

	inputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
            YUAN_NUM_OF_ALL * SIZE_OF_YUAN,(void *) matrix, NULL);
	randomBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, 
            sizeof(int32_t)*YUAN_NUM_OF_ALL*YUAN_NUM_OF_ALL, (void *) rdp, NULL);
	parameterBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, 
            sizeof(struct kernel_parameter),(void *) &param, NULL);

	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inputBuffer);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&randomBuffer);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&parameterBuffer);

    start_kernel_loop_in_thread();
    set_random_to_kernel();

    if(devices != NULL) { free(devices);}

    return 0;
}

int32_t set_random_data(uint32_t y, uint32_t d, long value) {
    
    if(y<0 || y >= YUAN_NUM_OF_ALL || d < 0 || d >= YUAN_NUM_OF_ALL) {
        return -1;
    }

    rdp[YUAN_NUM_OF_ALL*y + d] = value;
    set_random_to_kernel();
    return 0;
}

static int32_t set_input_to_kernel() {
    
    cl_int ret;
    char* p = (char*) clEnqueueMapBuffer(commandQueue, inputBuffer, CL_TRUE, CL_MAP_WRITE, 0, 
            YUAN_NUM_OF_INPUT*SIZE_OF_YUAN, 0, NULL, NULL, &ret);
    if(ret < 0) {
        printf("Error: clEnqueueMapBuffer failed. when set input\n");
        return ret;
    }

    memcpy(p, (char *)matrix, YUAN_NUM_OF_INPUT*SIZE_OF_YUAN);

    ret = clEnqueueUnmapMemObject(commandQueue, inputBuffer, p, 0, NULL, NULL);
	clFlush(commandQueue);
}

static int32_t set_random_to_kernel() {
    cl_int ret;

    char* p = (char*) clEnqueueMapBuffer(commandQueue, randomBuffer, CL_TRUE, CL_MAP_WRITE, 0, 
            YUAN_NUM_OF_ALL*YUAN_NUM_OF_ALL*sizeof(int32_t), 0, NULL, NULL, &ret);
    if(ret < 0) {
        printf("Error: clEnqueueMapBuffer failed. when set random\n");
        return ret;
    }

    memcpy(p, (char *)rdp, YUAN_NUM_OF_ALL*YUAN_NUM_OF_ALL*sizeof(int32_t));

    ret = clEnqueueUnmapMemObject(commandQueue, inputBuffer, p, 0, NULL, NULL);
	clFlush(commandQueue);
}

static int32_t set_parameter_to_kernel() {
    cl_int ret;
    char* p = (char*) clEnqueueMapBuffer(commandQueue, parameterBuffer, CL_TRUE, CL_MAP_WRITE, 0, 
            sizeof(struct kernel_parameter), 0, NULL, NULL, &ret);
    if(ret < 0) {
        printf("Error: clEnqueueMapBuffer failed. when set parameter：w\n");
        return ret;
    }

    memcpy(p, (char *)&param, sizeof(struct kernel_parameter));

    ret = clEnqueueUnmapMemObject(commandQueue, inputBuffer, p, 0, NULL, NULL);
	clFlush(commandQueue);
}

static int32_t run_kernel() {

    cl_int ret;
    size_t global_work_size[1] = {YUAN_NUM_OF_ALL};

    if(ticks%YUAN_FB_TICKS == 1) {
        ret = get_random(rdp, sizeof(int32_t)*YUAN_NUM_OF_ALL*YUAN_NUM_OF_ALL);
        if (ret < 0) {
            printf("Error: Getting random data!\n");
            return -1;
        }
        set_random_to_kernel();
        param.stage = STAGE_K;
        set_parameter_to_kernel();
        ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
        if(ret < 0) {
            printf("Error: clEnqueueNDRangeKernel K error\n");
        }
    }

    param.stage = STAGE_A;
    set_parameter_to_kernel();
    ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    if(ret < 0) {
        printf("Error: clEnqueueNDRangeKernel A error\n");
    }

    param.stage = STAGE_Y;
    set_parameter_to_kernel();
    ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    if(ret < 0) {
        printf("Error: clEnqueueNDRangeKernel Y error\n");
    }




    return ret;

}

static int32_t get_output_from_kernel() {

    cl_int ret;
    char* p = (char*) clEnqueueMapBuffer(commandQueue, inputBuffer, CL_TRUE, CL_MAP_READ,
            (YUAN_NUM_OF_INPUT+YUAN_NUM_OF_NORMAL)*SIZE_OF_YUAN,
            YUAN_NUM_OF_OUTPUT*SIZE_OF_YUAN, 0, NULL, NULL, &ret);
    if(ret < 0) {
        printf("Error: clEnqueueMapBuffer failed. when get output\n");
        return ret;
    }

    memcpy((char *)(&matrix[YUAN_NUM_OF_INPUT+YUAN_NUM_OF_NORMAL]), p, YUAN_NUM_OF_OUTPUT*SIZE_OF_YUAN);

    ret = clEnqueueUnmapMemObject(commandQueue, inputBuffer, p, 0, NULL, NULL);
	clFlush(commandQueue);

    return 0;
}

static int32_t get_matrix_from_kernel() {

    cl_int ret;
    char* p = (char*) clEnqueueMapBuffer(commandQueue, inputBuffer, CL_TRUE, CL_MAP_READ, 0,
            YUAN_NUM_OF_ALL*SIZE_OF_YUAN, 0, NULL, NULL, &ret);
    if(ret < 0) {
        printf("Error: clEnqueueMapBuffer failed. when get matrix\n");
        return ret;
    }
    memcpy((char *)matrix, p, YUAN_NUM_OF_ALL*SIZE_OF_YUAN);

    ret = clEnqueueUnmapMemObject(commandQueue, inputBuffer, p, 0, NULL, NULL);
	clFlush(commandQueue);

    return 0;
}

int32_t suspend_kernel() {

    loginfo("suspend kernel");
    state = SUSPEND_STATE;
    return 0;

}

int32_t step_kernel() {
    sem_post(&suspend_sem);
    return 0;
}

int32_t resume_kernel() {

    loginfo("resume kernel");
    state = RESUME_STATE;
    sem_post(&suspend_sem);
    return 0;
}

int32_t sync_kernel() {

    if(state == RESUME_STATE) {
        return -1;
    }

    return get_matrix_from_kernel();
}

static void kernel_loop_hread_func() {

    while(1) {
        ticks++;

        input_algo_callback_by_core();
        set_input_to_kernel();

        if(state != RESUME_STATE || param.fb) {
            sem_wait(&suspend_sem);
        }
        run_kernel();

        output_algo_callback_by_core();
        //get_output_from_kernel();
        get_matrix_from_kernel();

    }
}

static int32_t start_kernel_loop_in_thread() {
    int ret;
	pthread_t pid;
    
	ret = pthread_create(&pid, NULL, (void *)kernel_loop_hread_func, NULL);
	if(ret != 0) {
        loginfo("kernel loop pthread create error: %d", ret);
    }

    return ret;
}



int32_t close_kernel_platform() {

    cl_int ret;
	ret = clReleaseKernel(kernel);				
	ret = clReleaseProgram(program);	
	ret = clReleaseMemObject(inputBuffer);
	ret = clReleaseMemObject(randomBuffer);
	ret = clReleaseMemObject(parameterBuffer);
	ret = clReleaseCommandQueue(commandQueue);
	ret = clReleaseContext(context);
    if(rdp != NULL) {
        free(rdp);
    }
    printf("\tRelease kernel\n");
    loginfo("release kernel");
    return 0;

}





















