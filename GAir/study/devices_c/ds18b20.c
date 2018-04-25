#include <reg52.h>
#include <intrins.h>

unsigned char tempflag,fraction,tempr;

unsigned char code segmcode[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};

//共阴极数码管段码0-9

unsigned char code bitcode[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};

//8位共阴极数码管位码

unsigned char code fractioncode[]={0,0,1,2,2,3,4,4,5,6,6,7,8,8,9,9};

//将DS18B20的小数部分的0-f刻度转换为0-9刻度的查找表,将精度化为0.1度

sbit ser=P2^0;//74HC595串行数据输入

sbit oe=P2^1;//74HC595使能

sbit rclk=P2^2; //74HC595数据锁存

sbit srclk=P2^3;// 74HC595串行时钟

sbit DQ=P1^1; //温度总线



//延时函数(12MHZ晶振):

void Delayus(unsigned char t)

{  //此函数精确计算：18+6*(t-1)=延时时间(us)

    while(t--);

}



//延时ms延时函数:

void Delayms(unsigned int t)

{

    unsigned int i,j;

    for(i=t;i>0;i--)

        for(j=0;j<120;j++);

}


//DS18B20复位函数:

void Reset18B20(void)

{

    DQ=0;//拉低,开始复位操作

    Delayus(100);//延时至少480us

    DQ=1;//拉高，释放总线控制权

    while(DQ);//等待器件应答（器件拉低），约15-60us后

    while(!DQ);//应答脉冲出现后，等待器件拉高，约60-240us后

}

//DS18B20写命令函数:

void Write18B20(unsigned char com)

{

    unsigned char i;

    for(i=0;i<8;i++)

    {   

        DQ=0;//开始写操作

        _nop_(); _nop_();//至少延时1us

        DQ=com&0x01;//写数据

        Delayus(2);//延时，器件在45us内采样

        DQ=1;//释放总线控制权

        com>>=1; //右移1位，写下一位

    }

}



//DS18B20读数据函数:

unsigned char Read18B20()

{

    unsigned char i,rdata=0;

    for(i=0;i<8;i++)

    {

        DQ=0;//开始读操作

        _nop_();_nop_();//至少延时1us

        DQ=1;//释放总线控制权，15us内要读取数据

        if(DQ==1) rdata|=0x01<<i;

        Delayus(10);//延时要大于45us.读0时，45us后器件才拉高总线

    }

    return rdata;

}



//读出温度函数:

void Read18B20Temperature()

{

    unsigned char templ,temph,temp;

    unsigned int tempv;

    Reset18B20();//复位

    Write18B20(0xcc);//写命令，跳过ROM编码命令

    Write18B20(0x44);//转换命令

    while(!DQ);//等待转换完成

    Reset18B20();//复位

    Write18B20(0xcc);//写命令，跳过ROM编码命令

    Write18B20(0xbe);//读取暂存器字节命令

    templ=Read18B20();//读低字节

    temph=Read18B20();//读高字节

    Reset18B20();//复位

    tempv=temph;

    tempv=tempv<<8|templ;//两个字节合并为一个int型数据

    temp=(unsigned char)(tempv>>4);//去掉小数部分,化成char型数据

    if((temph&0x80)==0x80)//如果是负温度

    {

        tempflag=1; //负号显示

        tempr=~temp+1; //实际温度值为读取值的补码

        fraction=fractioncode[(~templ+1)&0x0f];

        //取小数部分补码，将16刻度转换为10刻度，精度为0.1度

    }

    else//如果是正温度

    {

        tempflag=0;//正温度,负号不显示

        tempr=temp;//

        fraction=fractioncode[templ&0x0f];

        //取小数部分，将16刻度转换为10刻度，精度为0.1度

    }     

}



//主函数:

int main(void)

{

    tempflag=0;

    while(1)

    {

        Read18B20Temperature();//读取温度值

        DTDisplayChar(segmcode[fraction],0x7f);//显示小数部分

        Delayms(1);

        DTDisplayChar(segmcode[tempr%10]|0x80,0xbf);//显示个位和小数点

        Delayms(1);

        DTDisplayChar(segmcode[tempr%100/10],0xdf);//显示十位

        Delayms(1);

        if(tempflag==1) DTDisplayChar(0x40,0xef);//如果是负温度就显示“-”

        else DTDisplayChar(segmcode[tempr/100],0xef);//显示百位

        Delayms(1);

        DTDisplayChar(0xff,0xff);//均衡数码管亮度

    }

    return 0;

}


