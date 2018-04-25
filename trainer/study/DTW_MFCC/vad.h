#ifndef _VAD_H
#define _VAD_H

#define	max_vc_con	3	//VAD��������������
#define frame_time	20						// ÿ֡ʱ�䳤�� ��λms
#define frame_mov_t	10						// ֡��
#define frame_len	(frame_time*fs/1000)	// ֡��	
#define frame_mov	(frame_mov_t*fs/1000)	// ֡�ƣ�����֡��������	

typedef struct
{
	u32 mid_val;	//��������ֵ �൱���з��ŵ�0ֵ ���ڶ�ʱ�����ʼ���
	u16	n_thl;		//������ֵ�����ڶ�ʱ�����ʼ���
	u16 z_thl;		//��ʱ��������ֵ����������ֵ����Ϊ������ɶΡ�
	u32 s_thl;		//��ʱ�ۼӺ���ֵ����������ֵ����Ϊ������ɶΡ�
}atap_tag;			//����Ӧ����

typedef struct
{
	u16 *start;	//��ʼ��
	u16 *end;	//������
}valid_tag;	//��Ч������

void noise_atap(const u16* noise,u16 n_len,atap_tag* atap);
void VAD(const u16 *vc, u16 buf_len, valid_tag *valid_voice, atap_tag *atap_arg);

#endif