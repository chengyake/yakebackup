/*******       DTW.C         ********/

#include "stm32f10x.h"
#include <math.h>
#include "ADC.h"
#include "VAD.H"
#include "MFCC.H"
#include "DTW.H"
#include "USART.H"

/*	
	DTW�㷨 ͨ���ֲ��Ż��ķ���ʵ�ּ�Ȩ�����ܺ���С
	
	ʱ�����������
	C={c(1),c(2),��,c(N)}
	NΪ·�����ȣ�c(n)=(i(n),j(n))��ʾ��n��ƥ������вο�ģ���
��i(n)������ʸ�������ģ��ĵ�j(n)������ʸ�����ɵ�ƥ���ԣ���
��֮��ľ���d(x(i(n)),y(j(n)))��Ϊƥ����롣
	ʱ�������������һ��Լ����
	1.�����ԣ����������������ӡ�
	2.����յ�Լ����������㣬�յ���յ㡣
	3.�����ԣ������������κ�һ�㡣
	4.��������������ĳһ����ֵ��|i(n)-j(n)|<M,MΪ����������
��������������λ��ƽ���ı����ڡ��ֲ�·��Լ�����������Ƶ���n��
Ϊ(i(n),j(n))ʱ��ǰ�������ڼ��ֿ��ܵ�·����

	DTW���裺
	1.��ʼ������i(0)=j(0)=0,i(N)=in_frm_num,j(N)=mdl_frm_num.
ȷ��һ��ƽ���ı��Σ�������λ��(0,0)��(in_frm_num,mdl_frm_num)�Ķ��㣬����б��б
�ʷֱ���2��1/2.�����������ɳ�����ƽ���ı��Ρ�
	2.�������ۼƾ��롣
	
	����������������ģ���֡��������ֱ�ӽ�ƥ�������Ϊ���
	frm_in_num<(frm_mdl_num/2)||frm_in_num>(frm_mdl_num*2)
*/

/*
	��ȡ��������ʸ��֮��ľ���
	����
	frm_ftr1	����ʸ��1
	frm_ftr2	����ʸ��2
	����ֵ
	dis			ʸ������
*/
u32 get_dis(s16 *frm_ftr1, s16 *frm_ftr2)
{
	u8 	i;
	u32	dis;
	s32 dif;	//��ʸ����ͬά���ϵĲ�ֵ
	
	dis=0;
	for(i=0;i<mfcc_num;i++)
	{
		//USART1_printf("dis=%d ",dis);
		dif=frm_ftr1[i]-frm_ftr2[i];
		dis+=(dif*dif);
	}
	//USART1_printf("dis=%d ",dis);
	dis=sqrtf(dis);
	//USART1_printf("%d\r\n",dis);
	return dis;
}

//ƽ���ı������������� X����ֵ
static u16	X1;			//�ϱ߽���
static u16	X2;			//�±߽���
static u16	in_frm_num;	//��������֡��
static u16	mdl_frm_num;//����ģ��֡��

#define ins		0
#define outs	1

/*
	��Χ����
*/
u8 dtw_limit(u16 x, u16 y)
{
	if(x<X1)
	{
		if(y>=((2*x)+2))
		{
			return outs;
		}
	}
	else
	{
		if((2*y+in_frm_num-2*mdl_frm_num)>=(x+4))
		{
			return outs;
		}
	}
	
	if(x<X2)
	{
		if((2*y+2)<=x)
		{
			return outs;
		}
	}
	else
	{
		if((y+4)<=(2*x+mdl_frm_num-2*in_frm_num))
		{
			return outs;
		}
	}
	
	return ins;
}

/*	
	DTW ��̬ʱ�����
	����
	ftr_in	:��������ֵ
	ftr_mdl	:����ģ��
	����ֵ
	dis		:�ۼ�ƥ�����
*/

u32 dtw(v_ftr_tag *ftr_in, v_ftr_tag *frt_mdl)
{
	u32 dis;
	u16 x,y;
	u16 step;
	s16 *in;
	s16 *mdl;
	u32 up,right,right_up;
	u32 min;
	
	in_frm_num=ftr_in->frm_num;
	mdl_frm_num=frt_mdl->frm_num;
	
	if((in_frm_num>(mdl_frm_num*2))||((2*in_frm_num)<mdl_frm_num))
	{
		//USART1_printf("in_frm_num=%d mdl_frm_num=%d\r\n", in_frm_num,mdl_frm_num);
		return dis_err;
	}
	else
	{
		// ����Լ��ƽ���ı��ζ���ֵ
		X1=(2*mdl_frm_num-in_frm_num)/3;
		X2=(4*in_frm_num-2*mdl_frm_num)/3;
		in=ftr_in->mfcc_dat;
		mdl=frt_mdl->mfcc_dat;

		dis=get_dis(in,mdl);
		x=1;
		y=1; 
		step=1;
		do
		{
			up=(dtw_limit(x,y+1)==ins)?get_dis(mdl+mfcc_num,in):dis_err;
			right=(dtw_limit(x+1,y)==ins)?get_dis(mdl,in+mfcc_num):dis_err;
			right_up=(dtw_limit(x+1,y+1)==ins)?get_dis(mdl+mfcc_num,in+mfcc_num):dis_err;
			
			min=right_up;
			if(min>right)
			{
				min=right;
			}
			if(min>up)
			{
				min=up;
			}
			
			dis+=min;
			
			if(min==right_up)
			{
				in+=mfcc_num;
				x++;
				mdl+=mfcc_num;
				y++;
			}
			else if(min==up)
			{
				mdl+=mfcc_num;
				y++;
			}
			else
			{
				in+=mfcc_num;
				x++;
			}
			step++;	
			//USART1_printf("x=%d y=%d\r\n",x,y);
		} 
		while((x<in_frm_num)&&(y<mdl_frm_num));
		//USART1_printf("step=%d\r\n",step);
	}
	return (dis/step); //������һ��
}


void get_mean(s16 *frm_ftr1, s16 *frm_ftr2, s16 *mean)
{
	u8 	i;

	for(i=0;i<mfcc_num;i++)
	{
		mean[i]=(frm_ftr1[i]+frm_ftr2[i])/2;
		USART1_printf("x=%d y=%d ",frm_ftr1[i],frm_ftr2[i]);
		USART1_printf("mean=%d\r\n",mean[i]);
	}
}

/*	
	��������ʸ����ȡ����ģ��
	����
	ftr_in1	:��������ֵ
	ftr_in2	:��������ֵ
	ftr_mdl	:����ģ��
	����ֵ
	dis		:�ۼ�ƥ�����
*/

u32 get_mdl(v_ftr_tag *ftr_in1, v_ftr_tag *ftr_in2, v_ftr_tag *ftr_mdl)
{
	u32 dis;
	u16 x,y;
	u16 step;
	s16 *in1;
	s16 *in2;
	s16 *mdl;
	u32 up,right,right_up;
	u32 min;
	
	in_frm_num=ftr_in1->frm_num;
	mdl_frm_num=ftr_in2->frm_num;
	
	if((in_frm_num>(mdl_frm_num*2))||((2*in_frm_num)<mdl_frm_num))
	{
		return dis_err;
	}
	else
	{
		// ����Լ��ƽ���ı��ζ���ֵ
		X1=(2*mdl_frm_num-in_frm_num)/3;
		X2=(4*in_frm_num-2*mdl_frm_num)/3;
		in1=ftr_in1->mfcc_dat;
		in2=ftr_in2->mfcc_dat;
		mdl=ftr_mdl->mfcc_dat;

		dis=get_dis(in1,in2);
		get_mean(in1, in2, mdl);
		x=1;
		y=1; 
		step=1;
		do
		{
			up=(dtw_limit(x,y+1)==ins)?get_dis(in2+mfcc_num,in1):dis_err;
			right=(dtw_limit(x+1,y)==ins)?get_dis(in2,in1+mfcc_num):dis_err;
			right_up=(dtw_limit(x+1,y+1)==ins)?get_dis(in2+mfcc_num,in1+mfcc_num):dis_err;
			
			min=right_up;
			if(min>right)
			{
				min=right;
			}
			if(min>up)
			{
				min=up;
			}
			
			dis+=min;
			
			if(min==right_up)
			{
				in1+=mfcc_num;
				x++;
				in2+=mfcc_num;
				y++;
			}
			else if(min==up)
			{
				in2+=mfcc_num;
				y++;
			}
			else
			{
				in1+=mfcc_num;
				x++;
			}
			step++;	
			
			mdl+=mfcc_num;
			get_mean(in1, in2, mdl);
			
			USART1_printf("x=%d y=%d\r\n",x,y);
		}
		while((x<in_frm_num)&&(y<mdl_frm_num));
		USART1_printf("step=%d\r\n",step);
		ftr_mdl->frm_num=step;
	}
	return (dis/step); //������һ��
}
