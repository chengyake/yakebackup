/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_disp.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/3/26
 *
 * Description:
 *          platform display �ӿ�
 **************************************************************************/

#ifndef _PLATFORM_DISP_H_
#define _PLATFORM_DISP_H_

#if defined LUA_DISP_LIB

typedef struct PlatformRectTag
{
    u16 ltx;        //left top x y
    u16 lty;
    u16 rbx;        //right bottom x y
    u16 rby;
}PlatformRect;

// ��ɫ���� RGB(5,6,5)
#define COLOR_WHITE 0xffff
#define COLOR_BLACK 0x0000

typedef enum PlatformLcdBusTag
{
    PLATFORM_LCD_BUS_SPI4LINE,
    PLATFORM_LCD_BUS_PARALLEL,

/*+\new\liweiqiang\2014.10.11\���lcd i2c spi�ӿ� */
    PLATFORM_LCD_BUS_I2C,
    PLATFORM_LCD_BUS_SPI,
/*-\new\liweiqiang\2014.10.11\���lcd i2c spi�ӿ� */
    
    PLATFORM_LCD_BUS_QTY,
}PlatformLcdBus;

/*+\new\liweiqiang\2014.10.11\���lcd i2c spi�ӿ� */
typedef union {
    struct {
        int bus_id;
        int pin_rs;
        int pin_cs;
        int freq;
    } bus_spi;
    
    struct {
        int bus_id;
        int freq;
        int slave_addr;
        int cmd_addr;
        int data_addr;
    } bus_i2c;
} lcd_itf_t;
/*-\new\liweiqiang\2014.10.11\���lcd i2c spi�ӿ� */

typedef struct PlatformDispInitParamTag
{
    u16 width;  // lcd�豸���
    u16 height; // lcd�豸�߶�
    u8  bpp; // bits per pixel lcd�豸ɫ�� 1:�ڰ� 16:16λɫ����
    u16 x_offset;
    u16 y_offset;
    u32 *pLcdCmdTable;    //lcd��ʼ��ָ���
    u16 tableSize;         //lcd��ʼ��ָ�����С
/*+\NEW\liweiqiang\2013.12.18\����lcd˯������֧�� */
    u32 *pLcdSleepCmd;  // lcd sleepָ���
    u16 sleepCmdSize;
    u32 *pLcdWakeCmd;   // lcd wakeָ���
    u16 wakeCmdSize;
/*-\NEW\liweiqiang\2013.12.18\����lcd˯������֧�� */
    PlatformLcdBus bus;
/*+\new\liweiqiang\2014.10.11\���lcd i2c�ӿ� */
    lcd_itf_t lcd_itf;
/*-\new\liweiqiang\2014.10.11\���lcd i2c�ӿ� */
    int pin_rst; //reset pin
    /*+\new\liweiqiang\2014.10.21\���Ӳ�ͬ�ڰ������ɫ���� */
    int hwfillcolor; // lcd�������ɫ
    /*-\new\liweiqiang\2014.10.21\���Ӳ�ͬ�ڰ������ɫ���� */
/*+\NEW\2013.4.10\���Ӻڰ�����ʾ֧�� */
    int pin_cs; // cs pin
    u8 *framebuffer;
/*-\NEW\2013.4.10\���Ӻڰ�����ʾ֧�� */
}PlatformDispInitParam;

void platform_disp_init(PlatformDispInitParam *pParam);

void platform_disp_close(void);

void platform_disp_clear(void);

void platform_disp_update(void);

void platform_disp_puttext(const char *string, u16 x, u16 y);

/*+\NEW\liweiqiang\2013.12.6\����ͼƬ͸��ɫ���� */
/*+\NEW\liweiqiang\2013.11.4\����BMPͼƬ��ʾ֧�� */
/*+\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
int platform_disp_putimage(const char *filename, u16 x, u16 y, int transcolor, u16 left, u16 top, u16 right, u16 bottom);
/*-\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
/*-\NEW\liweiqiang\2013.11.4\����BMPͼƬ��ʾ֧�� */
/*-\NEW\liweiqiang\2013.12.6\����ͼƬ͸��ɫ���� */

/*+\NEW\liweiqiang\2013.12.7\���Ӿ�����ʾ֧�� */
int platform_disp_drawrect(int x1, int y1, int x2, int y2, int color);
/*-\NEW\liweiqiang\2013.12.7\���Ӿ�����ʾ֧�� */

/*+\NEW\liweiqiang\2013.12.9\����ǰ��ɫ\����ɫ���� */
int platform_disp_setcolor(int color);
int platform_disp_setbkcolor(int color);
/*-\NEW\liweiqiang\2013.12.9\����ǰ��ɫ\����ɫ���� */

/*+\NEW\liweiqiang\2013.12.9\���ӷ������������� */
int platform_disp_loadfont(const char *name);
int platform_disp_setfont(int id);
/*-\NEW\liweiqiang\2013.12.9\���ӷ������������� */

#endif

#endif//_PLATFORM_DISP_H_
