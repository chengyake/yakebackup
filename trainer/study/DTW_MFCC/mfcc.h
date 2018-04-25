#ifndef _MFCC_H
#define _MFCC_H
#include "stm32f10x.h"
#include "VAD.H"


#define hp_ratio	95/100			//Ԥ����ϵ�� 0.95
#define fft_point	1024			//FFT����
#define frq_max		(fft_point/2)	//���Ƶ��
#define hamm_top	10000			//���������ֵ
#define	tri_top		1000			//�����˲�������ֵ
#define tri_num		24				//�����˲�������
#define mfcc_num	12				//MFCC����

#define vv_tim_max	1200	//������Ч�����ʱ�� ms
#define vv_frm_max	((vv_tim_max-frame_time)/(frame_time-frame_mov_t)+1)	//������Ч�����֡��

#pragma pack(1)
typedef struct
{
	u16 save_sign;						//�洢��� �����ж�flash������ģ���Ƿ���Ч
	u16 frm_num;						//֡��
	s16 mfcc_dat[vv_frm_max*mfcc_num];	//MFCCת�����
}v_ftr_tag;								//���������ṹ��
#pragma pack()

void get_mfcc(valid_tag *valid, v_ftr_tag *v_ftr, atap_tag *atap_arg);

#endif