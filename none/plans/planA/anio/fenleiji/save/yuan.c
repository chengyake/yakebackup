/*
 *  base on linggan feedback
 *
 */


#include "yuan.h"
#include "list.h"
#include "random.h"
#include "debug.h"

//all of yuan include
static struct yuan yuans[YUAN_NUM];

static unsigned long int fiffes, cfiffes; //jiffes
static enum system_status sys_status;

void __init_yuan(int id, enum yuan_type type)
{

    yuans[id].id=id;
    yuans[id].type=type;

    if(A_START >= id && id <= H_END)
    {
        yuans[id].iwing=(struct feather *)malloc(sizeof(struct feather)*YUAN_NUM);
        memset(yuans[id].iwing, 0, sizeof(struct feather)*YUAN_NUM);
    }

    if(SA_START >= id && id <= N_END)
    {
        yuans[id].owing=(struct feather *)malloc(sizeof(struct feather)*YUAN_NUM);
        memset(yuans[id].owing, 0, sizeof(struct feather)*YUAN_NUM);
    }
}

void init_yuans()
{
    int i; 

    for(i=SA_START; i<=SA_END; i++)
    {
        __init_yuan(i, SA);
    }
    for(i=SN_START; i<=SN_END; i++)
    {
        __init_yuan(i, SN);
    }
    for(i=I_START; i<=I_END; i++)
    {
        __init_yuan(i, I);
    }
    for(i=A_START; i<=A_END; i++)
    {
        __init_yuan(i, A);
    }
    for(i=N_START; i<=N_END; i++)
    {
        __init_yuan(i, N);
    }
    for(i=O_START; i<=O_END; i++)
    {
        __init_yuan(i, O);
    }
    for(i=H_START; i<=H_END; i++)
    {
        __init_yuan(i, H);
    }
}

//rule: 1000->1 input
int input()
{
    int i;

    if(backnum>0)
    {
        backnum--;
    }
    //input yuans
    if(get_random()%1000 == 0xfe)
    {
        backnum=100;
        for(i=I_START; i<=I_END; i++)
        {
            yuans[i].idata=OUTPUT;
        }
    }
    else
    {
        for(i=I_START; i<=I_END; i++)
        {
            yuans[i].idata=0;
        }
    }
    //sun
    for(i=SA_START; i<=SN_END; i++)
    {
        yuans[i].idata=OUTPUT;
    }
}

void y()
{
    int i, j;
    long int sum=0;
    for(i=A_START; i<YUAN_NUM; i++)
    {   
            for(j=0; j<YUAN_NUM; j++)
            {
                sum+= yuans[i].iwing[j].ps[yuans[i].iwing[j].pc];
            }
            if(OUTPUT>=sum && sum>=(-1-OUTPUT))
            {
                yuans[i].idata=sum;
            }
            else
            {
                yuans[i].idata=sum>0 ? OUTPUT : (-1-OUTPUT);
            }
        
    }
}

int a()
{
    int i;
    for(i=YUAN_START; i<=YUAN_END; i++)
    {
        if(yuans[i].idata >= THRESHOLD)
        {
            if(yuans[i].type == N)
            {
                yuans[i].odata=(-1-OUTPUT);
            }
            else
            {
                yuans[i].odata=OUTPUT;
            }
        }
        else
        {
            yuans[i].odata=0;
        }
    }
}

int output_feedback()
{
    long int sum=0;

    for(i=O_START; i<=O_END; i++)
    {
        sum+=yuans[i].odata;
    }

    if(backnum>0 && sum>OUTPUT*4)
    {
        printf("V!!!!!!\n");

        update_pbs();
    }
}

void update_yuan_p(unsigned int id)
{
    int i;
    unsigned int p[O_START];
     
    if(yuans[id].type > N_END)
    {
        return;
    }
     
    for(i=0; i<O_START; i++)
    {
        p[i]=get_random()%OUTPUT;
    }
    quick_sort(p, 0, O_START-1);

    yuans[id].iwing[0].pc=p[0];
    yuans[id].iwing[O_START-1].pc=OUTPUT-p[O_START];
    for(i=1; i<O_START-1; i++)
    {   
        if(yuans[id].owing[i] != 0)
        {
            yuans[id].iwing[i].pc=p[i]-p[i-1];
        }
    }
}

int update_yuans_p()
{
    int i;
    for(i=S_START; i<=N_END; i++)
    {
        update_pc(i);
        push_ps(i);
    }

    return 0;
}

int k()
{
    acc_pdata();
}

int e()
{
    int i, j, k;

    for(i=0; i<O_START; i++)
    {
        for(j=0; j<O_START; j++)
        {
            //cpy data&p
        }
    }
}

void update_hz()
{
    fiffes++;
    ciffes=fiffes/CTRL_FLASH_HZ;
}

void release_yuan()
{
    for(i=YUAN_START, i<YUAN_NUM, i++);
    {
        if(A_START >= id && id <= H_END)
        {
            free(yuans[id].iwing);
        }
        if(SA_START >= id && id <= N_END)
        {
            free(yuans[id].owing)
        }
    }
}

void main()
{

    sys_status=SYS_INIT;

    add_cmd_thread();

    init_yuans();
    init_random ();

    while(1)
    {
        if(sys_status==SYS_RUN)
        {
            update_hz();
            //1 tianchong I&SA&SN yuan
            input();
            //2 collect A&N&O&H yuan
            y();
            //3 if idata > yu; i->o all yuan else i=0
            a();
            //4 detect o and feedback, push pbs
            output_feedback();
            //5 if fhz > n; update pc and ps
            update_yuans_p();
            //6 according to pc&pbs , pdata ->> o o o o&h->null
            k();
            //7 o o o ->> i i i 
            e();
        } 
        else if(sys_status==SYS_SUS)
        {
            sleep(1);
        }
        else if(sys_status==SYS_STOP)
        {
            break;
        }
    }

    close_random();
    release_yuan();
}


/******************************************************/




