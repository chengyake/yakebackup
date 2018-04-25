/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_rtos.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/3/7
 *
 * Description:
 *          luaƽ̨��rtos��ӿ�
 **************************************************************************/

#ifndef _PLATFORM_RTOS_H_
#define _PLATFORM_RTOS_H_

/*+\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */
typedef enum PlatformPoweronReasonTag
{
    PLATFORM_POWERON_KEY,
    PLATFORM_POWERON_CHARGER,
    PLATFORM_POWERON_ALARM,
    PLATFORM_POWERON_RESTART,
    PLATFORM_POWERON_OTHER,
    PLATFORM_POWERON_UNKNOWN,
    /*+\NewReq NEW\zhuth\2014.6.18\���ӿ���ԭ��ֵ�ӿ�*/
    PLATFORM_POWERON_EXCEPTION,
    PLATFORM_POWERON_HOST,
    PLATFORM_POWERON_WATCHDOG,
    /*-\NewReq NEW\zhuth\2014.6.18\���ӿ���ԭ��ֵ�ӿ�*/

}PlatformPoweronReason;    
/*-\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */

// ��ʼ����ģ��ID
typedef enum PlatformRtosModuleTag
{
    RTOS_MODULE_ID_KEYPAD,

    // touch screen...

    NumOfRTOSModules
}PlatformRtosModule;

typedef struct KeypadMatrixDataTag
{
    unsigned char       row;
    unsigned char       col;
}KeypadMatrixData;

typedef struct KeypadMsgDataTag
{
    u8   type;  // keypad type
    BOOL bPressed; /* �Ƿ��ǰ�����Ϣ */
    union {
        struct {
            u8 row;
            u8 col;
        }matrix, gpio;
        u16 adc;
    }data;
}KeypadMsgData;

/*+\NEW\liweiqiang\2013.4.5\����lua gpio �ж�����*/
typedef struct PlatformIntDataTag
{
    elua_int_id             id;
    elua_int_resnum         resnum;
}PlatformIntData;
/*-\NEW\liweiqiang\2013.4.5\����lua gpio �ж�����*/

/*+\NEW\liweiqiang\2013.7.8\����rtos.pmd��Ϣ*/
#define PLATFORM_BATT_NOT_CHARGING      0
#define PLATFORM_BATT_CHARING           1
#define PLATFORM_BATT_CHARGE_STOP       2

typedef struct PlatformPmdDataTag
{
    BOOL    battStatus;
    BOOL    chargerStatus;
    u8      chargeState;
    u8      battLevel;
    u16     battVolt;
}PlatformPmdData;
/*-\NEW\liweiqiang\2013.7.8\����rtos.pmd��Ϣ*/

/*+\NEW\liweiqiang\2013.11.4\����audio.core�ӿڿ� */
typedef struct PlatformAudioDataTag
{
    BOOL    playEndInd;
    BOOL    playErrorInd;
}PlatformAudioData;
/*-\NEW\liweiqiang\2013.11.4\����audio.core�ӿڿ� */

#define PLATFORM_RTOS_WAIT_MSG_INFINITE         (0)

typedef enum PlatformMsgIdTag
{
    RTOS_MSG_WAIT_MSG_TIMEOUT, // receive message timeout
    RTOS_MSG_TIMER,
    RTOS_MSG_UART_RX_DATA,
    RTOS_MSG_KEYPAD,
/*+\NEW\liweiqiang\2013.4.5\����lua gpio �ж�����*/
    RTOS_MSG_INT,             
/*-\NEW\liweiqiang\2013.4.5\����lua gpio �ж�����*/
/*+\NEW\liweiqiang\2013.7.8\����rtos.pmd��Ϣ*/
    RTOS_MSG_PMD,
/*-\NEW\liweiqiang\2013.7.8\����rtos.pmd��Ϣ*/
/*+\NEW\liweiqiang\2013.11.4\����audio.core�ӿڿ� */
    RTOS_MSG_AUDIO,
/*-\NEW\liweiqiang\2013.11.4\����audio.core�ӿڿ� */

    NumOfMsgIds
}PlatformMsgId;

typedef union PlatformMsgDataTag
{
    int                 timer_id;
    int                 uart_id;
    KeypadMsgData       keypadMsgData;
/*+\NEW\liweiqiang\2013.4.5\����lua gpio �ж�����*/
    PlatformIntData     interruptData;
/*-\NEW\liweiqiang\2013.4.5\����lua gpio �ж�����*/
/*+\NEW\liweiqiang\2013.7.8\����rtos.pmd��Ϣ*/
    PlatformPmdData     pmdData;
/*-\NEW\liweiqiang\2013.7.8\����rtos.pmd��Ϣ*/
/*+\NEW\liweiqiang\2013.11.4\����audio.core�ӿڿ� */
    PlatformAudioData   audioData;
/*-\NEW\liweiqiang\2013.11.4\����audio.core�ӿڿ� */
}PlatformMsgData;

typedef struct PlatformMessageTag
{
    PlatformMsgId       id;
    PlatformMsgData     data;
}PlatformMessage;

typedef struct PlatformKeypadInitParamTag
{
    int type;
    struct{
        int inMask;         /* active key in mask */
        int outMask;        /* active key out mask */        
    }matrix;
}PlatformKeypadInitParam;

int platform_rtos_init(void);

int platform_rtos_poweroff(void);

/*+\NEW\liweiqiang\2013.9.7\����rtos.restart�ӿ�*/
int platform_rtos_restart(void);
/*-\NEW\liweiqiang\2013.9.7\����rtos.restart�ӿ�*/

int platform_rtos_init_module(int module, void *pParam);

int platform_rtos_receive(void **ppMessage, u32 timeout);

int platform_rtos_send(PlatformMessage *pMsg);

int platform_rtos_start_timer(int timer_id, int milliSecond);

int platform_rtos_stop_timer(int timer_id);

/*+\NEW\liweiqiang\2013.4.5\����rtos.tick�ӿ�*/
int platform_rtos_tick(void);
/*-\NEW\liweiqiang\2013.4.5\����rtos.tick�ӿ�*/

/*-\NEW\zhuwangbin\2017.2.12\��Ӱ汾��ѯ�ӿ� */
char *platform_rtos_get_version(void);
/*-\NEW\zhuwangbin\2017.2.12\��Ӱ汾��ѯ�ӿ� */

/*+\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */
int platform_get_poweron_reason(void);

int platform_rtos_poweron(int flag);

void platform_poweron_try(void);
/*-\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */
#endif/*_PLATFORM_RTOS_H_*/
