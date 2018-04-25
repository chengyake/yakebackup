#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>
#include <omp.h> 
#include <math.h>
#include <malloc.h>
#include <opencl.h>

#define OPENCL_GPU
/*
   input              hide          output
   .                ...             
   .                ...             
   .                ...             
   .                ...             
   .                ...             
   . wa1      b1    ...             
   .                ...  w2    b2   Y
   .                ...                           
   .                ...             
   .                ...             
   .                ...             
   .                ...             
   .                ...             
   .                ...             

   1120             1120x3        continues

   ia               ha             oa


   60 days -> 1 day
*/




#define NOD     (60)
#define NOI     (1120)
#define NOH     (1120/10)
#define NOF     (1024)
//#define NOF     (1236-NOD-1)
//#define NOF     (10)       
#define NOT     (1480-NOD-1)






float iat[NOT][NOI+1];
float ia[NOI];
float ha[NOH];
float oa;


float params[NOH*NOI+NOH+NOH+1];
float *wa1 = &params[0];
float *ba1 = &params[NOH*NOI];
float *wa2 = &params[NOH*NOI+NOH];
float *ba2 = &params[NOH*NOI+NOH+NOH];

float os[NOF];

float latest_score=0.0;
float stage=1.0;
char dbname[128]={0};



struct timeval t_start,t_end;
long cost_time = 0;


int calc_input_data_idx(int id, float *array);

#ifdef  OPENCL_GPU
size_t global_work_size[2] = {32, 32};
size_t local_work_size[2] = {16, 16};


int ret;

static cl_context context;
static cl_command_queue commandQueue;
static cl_program program;
static cl_kernel kernel;
static cl_mem inputBuffer_d;
static cl_mem inputBuffer_p;
static cl_mem inputBuffer_o;

static int read_cl(char *filename, char *text) {

    int fd, len, size;
    fd = open(filename, O_RDONLY);
    if(fd < 0) {
        printf("GPU Error: open kernel CL file error\n");
        exit(0);
    }
    len = lseek(fd, 0, SEEK_END);
    if(len < 0) {
        printf("GPU Error: get file length error\n");
        close(fd);
        exit(0);
    }
    lseek(fd, 0, SEEK_SET);
    size = read(fd, text, len);
    if(len != size) {
        printf("GPU Error: file len %d   %d\n", len, size);
        exit(0);
    }
    text[size]='\0';

    printf("GPU: read cl success\n");

    return 0;
}

int32_t init_kernel_platform() {

	cl_uint plat_num;
	cl_platform_id plat_id = NULL;
	cl_uint dev_num = 0;
	cl_device_id *devices;

	ret = clGetPlatformIDs(0, NULL, &plat_num);
	if (ret < 0) {
		printf("GPU Error: Getting plat_ids!\n");
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
		printf("GPU: No GPU device available.\n");
		printf("GPU: Choose CPU as default device.\n");
		ret = clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_CPU, 0, NULL, &dev_num);
		devices = (cl_device_id*)malloc(dev_num * sizeof(cl_device_id));
		ret = clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_CPU, dev_num, devices, NULL);
	} else {
		printf("GPU: Choose GPU as default device. dev_num %d\n", dev_num);
		devices = (cl_device_id*)malloc(dev_num * sizeof(cl_device_id));
		ret = clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_GPU, dev_num, devices, NULL);
	}
	
	context = clCreateContext(NULL,1, devices,NULL,NULL,NULL);

	commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
    
	char filename[] = "core_kernel.c";
	char file_context[10*1024]={0};
	const char *source = &file_context[0];

    ret = read_cl(filename, &file_context[0]);

	size_t sourceSize[10] = {strlen(source)};
	cl_program program = clCreateProgramWithSource(context, 1, &source, &sourceSize[0], NULL);
	
	ret = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
    if(ret < 0) {
        printf("GPU Error: clBuildProgram error\n");
        exit(0);
    }

	kernel = clCreateKernel(program, "matrix", NULL);

	inputBuffer_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, 
            sizeof(iat), (void *)(&iat[0][0]), NULL);
	inputBuffer_p = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, 
            sizeof(params), (void *)(&params[0]), NULL);
	inputBuffer_o = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, 
            sizeof(os), (void *)(&os[0]), NULL);


	ret = clSetKernelArg(kernel, 0, sizeof(inputBuffer_d), (void *)&inputBuffer_d);
	ret = clSetKernelArg(kernel, 1, sizeof(inputBuffer_p), (void *)&inputBuffer_p);
	ret = clSetKernelArg(kernel, 2, sizeof(inputBuffer_o), (void *)&inputBuffer_o);



    if(devices != NULL) { free(devices);}

    printf("GPU: init cl plat success\n");
    return 0;
}


static int32_t set_input_data_to_kernel() {
    
    char* p = (char*) clEnqueueMapBuffer(commandQueue, inputBuffer_d, CL_TRUE, CL_MAP_WRITE, 0, 
            sizeof(iat), 0, NULL, NULL, &ret);
    if(ret < 0) {
        printf("GPU Error: clEnqueueMapBuffer failed. when set input\n");
        return ret;
    }

    memcpy(p, (char *)(iat), sizeof(iat));

    ret = clEnqueueUnmapMemObject(commandQueue, inputBuffer_d, p, 0, NULL, NULL);
	clFlush(commandQueue);
	return 0;
}

static int32_t update_input_params_to_kernel() {
    
    char* p = (char*) clEnqueueMapBuffer(commandQueue, inputBuffer_p, CL_TRUE, CL_MAP_WRITE, 0, 
            sizeof(params), 0, NULL, NULL, &ret);
    if(ret < 0) {
        printf("GPU Error: clEnqueueMapBuffer failed. when set input\n");
        return ret;
    }

    memcpy(p, (char *)(params), sizeof(params));

    ret = clEnqueueUnmapMemObject(commandQueue, inputBuffer_p, p, 0, NULL, NULL);
	clFlush(commandQueue);
	return 0;
}

static int32_t run_kernel() {

    ret = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    //ret = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if(ret < 0) {
        printf("GPU Error: clEnqueueNDRangeKernel error\n");
    }

    clFinish(commandQueue);

    return ret;

}

static int32_t get_output_from_kernel() {

    char* p = (char*) clEnqueueMapBuffer(commandQueue, inputBuffer_o, CL_TRUE, CL_MAP_READ, 0, sizeof(os), 0, NULL, NULL, &ret);
    if(ret < 0) {
        printf("GPU Error: clEnqueueMapBuffer failed. when get matrix\n");
        return ret;
    }
    memcpy((char *)&os[0], p, sizeof(os));

    ret = clEnqueueUnmapMemObject(commandQueue, inputBuffer_o, p, 0, NULL, NULL);
	clFlush(commandQueue);

    return 0;
}



static int32_t get_parameters_from_kernel() {

    char* p = (char*) clEnqueueMapBuffer(commandQueue, inputBuffer_p, CL_TRUE, CL_MAP_READ, 0, sizeof(params), 0, NULL, NULL, &ret);
    if(ret < 0) {
        printf("GPU Error: clEnqueueMapBuffer failed. when get matrix\n");
        return ret;
    }
    memcpy((char *)&params, p, sizeof(params));

    ret = clEnqueueUnmapMemObject(commandQueue, inputBuffer_p, p, 0, NULL, NULL);
	clFlush(commandQueue);

    return 0;
}

int32_t close_kernel_platform() {

	ret = clReleaseKernel(kernel);				
	ret = clReleaseProgram(program);	
	ret = clReleaseMemObject(inputBuffer_d);
	ret = clReleaseMemObject(inputBuffer_p);
	ret = clReleaseMemObject(inputBuffer_o);
	ret = clReleaseCommandQueue(commandQueue);
	ret = clReleaseContext(context);

    printf("release kernel\n");

    return 0;

}
#endif


int rd;
int init_random() {
    rd = open ("/dev/urandom", O_RDONLY);
    if (rd <= 0) {
        printf("open /dev/urandom error\n");
        return -1;
    }
    return 0;
}
//#define MYRANDOM
int get_random(unsigned char *dst, unsigned int num) {
#ifdef MYRANDOM
    int ret;
    ret = read(rd, dst, num);
    if(ret < 0) {
        printf("get random num error when read /dev/urandom");
        return -1;
    }
    return ret;
#else
    int i, *p;
    int seed[2], ret;
    ret = read(rd, &seed, 8);
    srand(seed[0]);
    p = (int *)dst;
    for(i=seed[1]%num; i<num; i+=4) {
        *p = rand();
        p++;
    }
    for(i=0; i<seed[1]%num; i+=4) {
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



//Train     2010-12-02 ~ 2015-12-31   1236
//Test      2016-01-01 ~ 2016-12-31    244

#define  NOLINE     1480
#define  NOSTOCK    27
#define  NOITEM     4
struct table_t {
    unsigned char date[16];
    float data[NOSTOCK][NOITEM];
};
struct table_t table_org[NOLINE];
struct table_t table[NOLINE-1];
void init_input_data() {

    FILE *fp;
    int ret, i, j, k,l;
    char af[64];
    size_t len = 0;
    ssize_t rlen = 0;
    fp = fopen("../data/data.txt",  "r");
    if(fp == NULL) {
        printf("Error: data file not found\n");
        exit(0);
    }

    memset(table_org, 0, sizeof(table_org));

    for(l=0; l<NOLINE; l++) {
        char *str = NULL;
        rlen = getline(&str, &len, fp);
        for(i=0; i<rlen; i++) {
            if(str[i]!=',') {
                table_org[l].date[i] = str[i];
            } else {
                table_org[l].date[i] = 0;
                break;
            }
        }
        k=0;
        j=i++;
        for(; i<rlen; i++) {
            if(str[i]==',') {
                memset(af, 0, 64);
                memcpy(af, &str[j+1], i-j-1);
                table_org[l].data[k/NOITEM][k%NOITEM] = atof(af);
                j=i;
                k++;
                if(k>=NOITEM*NOSTOCK) {
                    break;
                }
            }
        }
        free(str);
    }
    fclose(fp);

    //modify zero
    for(i=0; i<NOSTOCK; i++) {
        for(j=0; j<NOITEM; j++) {
            if(table_org[0].data[i][j] < 0.000001 && table_org[0].data[i][j] > -0.000001) {
                printf("Data Error: first line have zero\n");
                exit(0);
            }
        }
    }
    for(l=1; l<NOLINE; l++) {
        for(i=0; i<NOSTOCK; i++) {
            for(j=0; j<NOITEM; j++) {
                if(table_org[l].data[i][j] < 0.000001 && table_org[l].data[i][j] > -0.000001) {
                    table_org[l].data[i][j] = table_org[l-1].data[i][j];
                }
            }
        }
    }

    //calc percent point
    for(l=0; l<NOLINE-1; l++) {
        memcpy(table[l].date, table_org[l+1].date, strlen(table_org[l+1].date));
        for(i=0; i<NOSTOCK; i++) {

            table[l].data[i][0] = (table_org[l+1].data[i][0] - table_org[l].data[i][2])/table_org[l].data[i][2];
            table[l].data[i][1] = (table_org[l+1].data[i][1] - table_org[l].data[i][1])/table_org[l].data[i][1];
            table[l].data[i][2] = (table_org[l+1].data[i][2] - table_org[l+1].data[i][0])/table_org[l+1].data[i][0];
            table[l].data[i][3] = (table_org[l+1].data[i][3] - table_org[l].data[i][3])/table_org[l].data[i][3];
        }
    }
    //printf("%s %f %f\n", table_org[0].date, table_org[0].data[0][0], table_org[NOLINE-1].data[NOSTOCK-1][3]);
    //printf("%s %f %f\n", table[NOLINE-12].date, table[NOLINE-12].data[NOSTOCK-1][3], table[NOLINE-12].data[NOSTOCK-1][2]);
    
    for(i=0; i<NOT; i++) {
        calc_input_data_idx(i, &iat[i][0]);
    }

    return;
}


//o,a,c,v
#define start   (NOD) 
#define end     (NOD+NOF)
float update_input_data() {
    static int c=0;

    if(c>=NOF||c>=NOT) {c=0;}
    memcpy(ia, &iat[c][0], sizeof(ia));
    return iat[c++][NOI];
}

float update_input_data_idx(int id) {
    
    if(id<0 || id >= NOT) {
        printf("update input data idx overload\n");
        exit(0);
    }
    memcpy(ia, &iat[id][0], sizeof(ia));
    return iat[id][NOI];
}


int calc_input_data_idx(int id, float *array) {

    int i,d;

    int s = start+id-NOD;
    int idx=0;
    
    for(i=0; i<NOSTOCK-1; i++) {
        for(d=s; d<50+s; d+=5) {
            array[idx++] = (table[d].data[i][1] + table[d+1].data[i][1] + 
                    table[d+2].data[i][1] + table[d+3].data[i][1] + table[d+4].data[i][1])/5;
            array[idx++] = (table[d].data[i][3] + table[d+1].data[i][3] + 
                    table[d+2].data[i][3] + table[d+3].data[i][3] + table[d+4].data[i][3])/5;
        }
        for(; d<NOD+s; d++) {
            array[idx++] = table[d].data[i][1];
            array[idx++] = table[d].data[i][3];
        }
    }
    for(d=s; d<50+s; d+=5) {
        array[idx++] = (table[d].data[i][0] + table[d+1].data[i][0] + 
                   table[d+2].data[i][0] + table[d+3].data[i][0] + table[d+4].data[i][0])/5;
        array[idx++] = (table[d].data[i][1] + table[d+1].data[i][1] + 
                   table[d+2].data[i][1] + table[d+3].data[i][1] + table[d+4].data[i][1])/5;
        array[idx++] = (table[d].data[i][2] + table[d+1].data[i][2] + 
                   table[d+2].data[i][2] + table[d+3].data[i][2] + table[d+4].data[i][2])/5;
        array[idx++] = (table[d].data[i][3] + table[d+1].data[i][3] + 
                   table[d+2].data[i][3] + table[d+3].data[i][3] + table[d+4].data[i][3])/5;
    }
    for(; d<NOD+s; d++) {
        array[idx++] = table[d].data[i][0];
        array[idx++] = table[d].data[i][1];
        array[idx++] = table[d].data[i][2];
        array[idx++] = table[d].data[i][3];
    }
    
    array[idx]=table[start+id].data[i][1];

    return 0;
}

void close_input_data() {
    return;
}

int init_random_parameters() {

    int i,j;
    int wat[NOI];
    int bat[NOH];

    for(i=0; i<NOH; i++) {
        get_random((unsigned char*)&wat[0], sizeof(wat));
        for(j=0; j<NOI; j++) {
            wa1[i*NOI+j]=1.0*(wat[j]%20000-10000)/10000.0;
        }
    }
    get_random((unsigned char*)&bat[0], sizeof(bat));
    for(i=0; i<NOH; i++) {
        ba1[i]=1.0*(bat[i]%20000-10000)/10000.0;
    }
    get_random((unsigned char*)&bat[0], sizeof(bat));
    for(i=0; i<NOH; i++) {
        wa2[i]=1.0*(bat[i]%20000-10000)/10000.0;
    }
    get_random((unsigned char*)&bat[0], sizeof(bat));
    *ba2=1.0*(bat[0]%20000-10000)/10000.0;

    return 0;
}

float non_line_convert(float l) {
    if(l < -1.0)
        return -1.0;
    if(l > 1.0)
        return 1.0;
    if(l >=0) {
        if(l > 0.5)
            return sqrt(l);
        else
            return l*l;
    } else {
        if(l < -0.5)
            return -sqrt(-l);
        else
            return -l*l;
    }
}

//chengyake
//return sqrt(oa[result])/(sum+11.0);         //  86/56
//return sqrt(sqrt(oa[result]))/(sum+11.0);   //  84/51
//return (oa[result])/(sum/4+11.0);           //  86/57
//return (oa[result])/(sum/2+11.0);           //  84/58
//return oa[result]/(sum+11.0);               //  82/53
//return oa[result]/(sum+11.0);               //  82/53
//return oa[result]/(sum-oa[result]+11.0)+oa[result];       //  82/53

float _calc_(float a) {
    int i, j, t;
    #pragma omp parallel for
    for(i=0; i<NOH; i++) {
        ha[i]=0.0;
        for(j=0; j<NOI; j++) {
            ha[i]+=ia[j]*(wa1[i*NOI+j]);
        }
        ha[i]/=NOI;
        ha[i]+=(ba1[i]/10);
    }

    oa=0.0;
    for(i=0; i<NOH; i++) {
        oa += ha[i] * wa2[i];
    }
    oa/=NOH;
    oa+=(*ba2/10);


    return 1/(pow(2.71828, fabs(a-oa)*100));
}

float calc_once() {
#ifdef OPENCL_GPU
    int i;
    float score=0.0;

    //gettimeofday(&t_start, NULL);
    update_input_params_to_kernel();
    run_kernel();
    get_output_from_kernel();
    //gettimeofday(&t_end, NULL);
    for(i=0; i<NOF; i++) {
        score+=os[i];
    }
    //cost_time = (t_end.tv_sec*1000000l+t_end.tv_usec) - (t_start.tv_sec*1000000l+t_start.tv_usec);
    //printf("Cost time: %lds+%ldus\n", cost_time/1000000, cost_time%1000000);
    //exit(0);
    return score/NOF;

#else
    int i;
    float score=0.0, answer;
    //gettimeofday(&t_start, NULL);
    for(i=0; i<NOF; i++) {
        answer = update_input_data();
        score +=  _calc_(answer);
    }
    //gettimeofday(&t_end, NULL);
    //cost_time = (t_end.tv_sec*1000000l+t_end.tv_usec) - (t_start.tv_sec*1000000l+t_start.tv_usec);
    //printf("Cost time: %lds+%ldus\n", cost_time/1000000, cost_time%1000000);
    //exit(0);
    return score/NOF;
#endif
}


int best_parameter(float *p, float level) {
    int i;
    int maxi=-11;
    float maxs=-1.0*NOF, s;
    float org = *p;

    if(level>0.09 && level < 0.11) {
        org = 0.0;
    }
    for(i=-10; i<11; i++) {
        if(org+i*level < -1.0)
            *p = -1.0;
        else if(org+i*level > 1.0)
            *p = 1.0;
        else
            *p=org+i*level;

        s = calc_once();
        if(maxs < s) {
            maxs = s;
            maxi = i;
        }
    }

    //if(maxs > latest_score) {
    if(org+maxi*level < -1.0)
        *p = -1.0;
    else if(org+maxi*level > 1.0)
        *p = 1.0;
    else
        *p=org+maxi*level;
    latest_score = maxs;
    //}
#ifdef OPENCL_GPU
    //update_input_params_to_kernel();
#endif

    printf("w latest_score:%2.10f\n", latest_score);
    return 0;
}

int iter_parameters(float level) {
    int i, j;

    //ba2
    printf("stage %1.6f\tba2\t", stage);
    best_parameter(ba2, level);

    //wa2
    for(i=0; i<NOH; i++) {
        printf("stage %1.6f\twa2[%d]\t", stage, i);
        best_parameter(&wa2[i], level);
    }

    //ba2
    printf("stage %1.6f\tba2\t", stage);
    best_parameter(ba2, level);

    //ba1
    for(i=0; i<NOH; i++) {
        printf("stage %1.6f\tba1[%d]\t", stage, i);
        best_parameter(&ba1[i], level);
    }

    //wa2
    for(i=0; i<NOH; i++) {
        printf("stage %1.6f\twa2[%d]\t", stage, i);
        best_parameter(&wa2[i], level);
    }
    //ba2
    printf("stage %1.6f\tba2\t", stage);
    best_parameter(ba2, level);

    //ba1
    for(i=0; i<NOH; i++) {
        printf("stage %1.6f\tba1[%d]\t", stage, i);
        best_parameter(&ba1[i], level);
    }

    //wa1
    for(i=0; i<NOH; i++) {
        for(j=0; j<NOI; j++) {
            printf("stage %1.6f\twa1[%d %d]\t", stage, i, j);
            best_parameter(&wa1[i*NOI+j], level);
        }
    }
    



    return 0;
}

static float win=0.0;
float test_train(int idx) {

    int i, j, t;
    float answer = update_input_data_idx(idx);

    for(i=0; i<NOH; i++) {
        ha[i]=0.0;
        for(j=0; j<NOI; j++) {
            ha[i]+=ia[j]*(wa1[i*NOI+j]);
        }
        ha[i]/=NOI;
        ha[i]+=(ba1[i]/10);
    }

    oa=0.0;
    for(i=0; i<NOH; i++) {
        oa += ha[i] * wa2[i];
    }
    oa/=NOH;
    oa+=(*ba2/10);

    if(oa > 0.005) {
        win+=answer;
    }

    return fabs(answer-oa);
}

void test_sample() {
    int i;
    float cn=0.0;
    win=0.0;
    printf("\n----------NOF-%d---------------\n", NOF);
    for(i=0; i<NOF; i++) {
        cn+=test_train(i);
    }
    printf("train result: %f, win %f percent\n", cn/NOF, win/NOF);

    cn=0.0;
    win=0.0;
    for(i=NOF; i<NOT; i++) {
        cn+=test_train(i);
    }
    printf("test  result: %f, win %f percent\n", cn/(NOT-NOF), win/(NOT-NOF));

}

int load_random_parameters(char *file) {

    int fd, ret;
    if(file==NULL) {
        printf("CMD Error: load random file point is null\n");    
        exit(0);
    }

    fd = open(file, O_RDONLY, S_IRUSR|S_IWUSR);
    if(fd < 0) {
        printf("create %s error", file);
        exit(0);
    }
    ret = lseek(fd, 0, SEEK_SET);
    if(ret < 0) {
        printf("create %s fixed space error", file);
        close(fd);
        exit(0);
    }
    ret=read(fd, params, sizeof(params));
#ifdef OPENCL_GPU
    update_input_params_to_kernel();
#endif

    close(fd);
}

int store_random_parameters(char *file) {
    
    int fd, ret;
    if(file==NULL) {
        time_t tv;
        struct tm* ptm;
        time(&tv);
        ptm = localtime(&tv);
        strftime(dbname, sizeof(dbname), "single_once_%m-%d_%H-%M-%S.db", ptm);
        file=&dbname[0];
#ifdef OPENCL_GPU
        update_input_params_to_kernel();
#endif
    }

    fd = open(file, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if(fd < 0) {
        printf("create %s error", file);
        exit(0);
    }
    ret = lseek(fd, 0, SEEK_SET);
    if(ret < 0) {
        printf("create %s fixed space error", file);
        close(fd);
        exit(0);
    }

#ifdef  OPENCL_GPU
    get_parameters_from_kernel();
#endif

    ret=write(fd, params, sizeof(params));

    close(fd);
}


void train_sample(char *file) {

    int i,j;
    float score_terminal=0.0;
    //train
    stage=1.0;
    for(i=0; i<3; i++) {
        stage*=0.1;
        score_terminal = 0.0;
        for(j=0; j<500; j++) {
            iter_parameters(stage);
            store_random_parameters(file);
            printf("\n================run %d times and get correct rate: %f=================\n", j+1, latest_score);
            if(score_terminal == latest_score || latest_score > 0.995) {
                break;
            } else {
                score_terminal = latest_score;
            }
        }
    }

}

/*
argv: filename to train or test

mn            :new train once
mn c file     :train continue
mn t file     :test samples

 */
void main(int argc, char *argv[]) {
    
    if(argc != 3 && argc != 1) {
        printf("CMD Error: parameteres Error!\n");
        return ;
    }
    if(argc == 3 && (access(argv[2], F_OK|R_OK) != 0 || (argv[1][0] != 'c' && argv[1][0] != 't'))) {
        printf("CMD Error: no file found or parameteres error!\n");
        return ;
    }

    init_random();
    init_input_data();

#ifdef OPENCL_GPU
    ret = init_kernel_platform();
    if(ret < 0) {
        printf("init opencl mu1 error\n");
        return;
    }
    set_input_data_to_kernel();
#endif   

    if(argc == 3) {
        memcpy(dbname, argv[2], strlen(argv[2]));
        if(argv[1][0] == 'c')  {
            load_random_parameters(dbname);
            train_sample(dbname);
        }
        if(argv[1][0] == 't') {
            load_random_parameters(dbname);
            test_sample();
        }

    } else {
        init_random_parameters();
        store_random_parameters(NULL);
        train_sample(dbname);
        test_sample();
    }
#ifdef OPENCL_GPU
    close_kernel_platform();
#endif
    close_input_data();
    close_random();
}












