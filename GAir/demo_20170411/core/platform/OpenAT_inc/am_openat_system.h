/*********************************************************
  Copyright (C), AirM2M Tech. Co., Ltd.
  Author: lifei
  Description: AMOPENAT ����ƽ̨
  Others:
  History: 
    Version�� Date:       Author:   Modification:
    V0.1      2012.12.14  lifei     �����ļ�
*********************************************************/
#ifndef AM_OPENAT_SYSTEM_H
#define AM_OPENAT_SYSTEM_H

#include "am_openat_common.h"

/****************************** SYSTEM ******************************/
#define OPENAT_CUST_TASKS_PRIORITY_BASE 235
#define OPENAT_SEMAPHORE_TIMEOUT_MIN_PERIOD 5 //5ms

/* �߳�����ں��������� pParameter Ϊ create_task �ӿڴ���Ĳ��� */
typedef VOID (*PTASK_MAIN)(PVOID pParameter);

typedef enum E_AMOPENAT_OS_CREATION_FLAG_TAG
{
    OPENAT_OS_CREATE_DEFAULT = 0,   /* �̴߳������������� */
    OPENAT_OS_CREATE_SUSPENDED = 1, /* �̴߳������ȹ��� */
}E_AMOPENAT_OS_CREATION_FLAG;

typedef struct T_AMOPENAT_TASK_INFO_TAG
{
    UINT16 nStackSize;
    UINT16 nPriority;
    CONST UINT8 *pName;
}T_AMOPENAT_TASK_INFO;

/*+\NEW\liweiqiang\2013.7.1\[OpenAt]����ϵͳ��Ƶ���ýӿ�*/
typedef enum E_AMOPENAT_SYS_FREQ_TAG
{
    OPENAT_SYS_FREQ_32K    = 32768,
    OPENAT_SYS_FREQ_13M    = 13000000,
    OPENAT_SYS_FREQ_26M    = 26000000,
    OPENAT_SYS_FREQ_39M    = 39000000,
    OPENAT_SYS_FREQ_52M    = 52000000,
    OPENAT_SYS_FREQ_78M    = 78000000,
    OPENAT_SYS_FREQ_104M   = 104000000,
    OPENAT_SYS_FREQ_156M   = 156000000,
    OPENAT_SYS_FREQ_208M   = 208000000,
    OPENAT_SYS_FREQ_250M   = 249600000,
    OPENAT_SYS_FREQ_312M   = 312000000,
}E_AMOPENAT_SYS_FREQ;
/*-\NEW\liweiqiang\2013.7.1\[OpenAt]����ϵͳ��Ƶ���ýӿ�*/

/****************************** TIME ******************************/
typedef struct T_AMOPENAT_SYSTEM_DATETIME_TAG
{
    UINT16 nYear;
    UINT8  nMonth;
    UINT8  nDay;
    UINT8  nHour;
    UINT8  nMin;
    UINT8  nSec;
    UINT8  DayIndex; /* 0=Sunday */
}T_AMOPENAT_SYSTEM_DATETIME;

typedef struct
{
  uint8               alarmIndex;
  bool                alarmOn; /* 1 set,0 clear*/
  uint8               alarmRecurrent; /* 1 once,bit1:Monday...bit7:Sunday */
  T_AMOPENAT_SYSTEM_DATETIME alarmTime;
}T_AMOPENAT_ALARM_PARAM;

/****************************** TIMER ******************************/
#define OPENAT_TIMER_MIN_PERIOD 5 //5ms

typedef struct T_AMOPENAT_TIMER_PARAMETER_TAG
{
    HANDLE hTimer;      /* create_timer �ӿڷ��ص� HANDLE */
    UINT32 period;      /* start_timer �ӿڴ���� nMillisecondes */
    PVOID  pParameter;  /* create_timer �ӿڴ���� pParameter */
}T_AMOPENAT_TIMER_PARAMETER;

/* ��ʱ����ʱ�ص����������� pParameter Ϊջ����ָ�룬�ͻ������в���Ҫ�ͷŸ�ָ�� */
typedef VOID (*PTIMER_EXPFUNC)(T_AMOPENAT_TIMER_PARAMETER *pParameter);

typedef VOID (*PMINUTE_TICKFUNC)(VOID);


int OPENAT_GetEnvUsage(void);
void OPENAT_Decode(UINT32* v, INT32 n);


#endif /* AM_OPENAT_SYSTEM_H */

