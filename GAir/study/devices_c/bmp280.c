#ifndef __BMP280_H__  
#define __BMP280_H__  

#define BMP280_ADDRESS 0xEC  

#define BMP280_RESET_VALUE 0xB6  

/*calibration parameters */  
#define BMP280_DIG_T1_LSB_REG                0x88  
#define BMP280_DIG_T1_MSB_REG                0x89  
#define BMP280_DIG_T2_LSB_REG                0x8A  
#define BMP280_DIG_T2_MSB_REG                0x8B  
#define BMP280_DIG_T3_LSB_REG                0x8C  
#define BMP280_DIG_T3_MSB_REG                0x8D  
#define BMP280_DIG_P1_LSB_REG                0x8E  
#define BMP280_DIG_P1_MSB_REG                0x8F  
#define BMP280_DIG_P2_LSB_REG                0x90  
#define BMP280_DIG_P2_MSB_REG                0x91  
#define BMP280_DIG_P3_LSB_REG                0x92  
#define BMP280_DIG_P3_MSB_REG                0x93  
#define BMP280_DIG_P4_LSB_REG                0x94  
#define BMP280_DIG_P4_MSB_REG                0x95  
#define BMP280_DIG_P5_LSB_REG                0x96  
#define BMP280_DIG_P5_MSB_REG                0x97  
#define BMP280_DIG_P6_LSB_REG                0x98  
#define BMP280_DIG_P6_MSB_REG                0x99  
#define BMP280_DIG_P7_LSB_REG                0x9A  
#define BMP280_DIG_P7_MSB_REG                0x9B  
#define BMP280_DIG_P8_LSB_REG                0x9C  
#define BMP280_DIG_P8_MSB_REG                0x9D  
#define BMP280_DIG_P9_LSB_REG                0x9E  
#define BMP280_DIG_P9_MSB_REG                0x9F  

#define BMP280_CHIPID_REG                    0xD0  /*Chip ID Register */  
#define BMP280_RESET_REG                     0xE0  /*Softreset Register */  
#define BMP280_STATUS_REG                    0xF3  /*Status Register */  
#define BMP280_CTRLMEAS_REG                  0xF4  /*Ctrl Measure Register */  
#define BMP280_CONFIG_REG                    0xF5  /*Configuration Register */  
#define BMP280_PRESSURE_MSB_REG              0xF7  /*Pressure MSB Register */  
#define BMP280_PRESSURE_LSB_REG              0xF8  /*Pressure LSB Register */  
#define BMP280_PRESSURE_XLSB_REG             0xF9  /*Pressure XLSB Register */  
#define BMP280_TEMPERATURE_MSB_REG           0xFA  /*Temperature MSB Reg */  
#define BMP280_TEMPERATURE_LSB_REG           0xFB  /*Temperature LSB Reg */  
#define BMP280_TEMPERATURE_XLSB_REG          0xFC  /*Temperature XLSB Reg */  

/* 在foreced mode下，1s的采样周期，温度和气压使用最低的精度采集并且使用最小的滤波器系数, 
 * 数据的采集时间大概在6ms，平均功率为3.27uA。 
 * */  

/* 在foreced mode下，1s的采样周期, 温度和气压使用最高的精度采集并且使用最大的滤波器系数， 
 * 数据的采集时间大概在70ms，平均功率为30uA。 
 * */  

typedef enum {  
    BMP280_T_MODE_SKIP = 0x0,   /*skipped*/  
    BMP280_T_MODE_1,            /*x1*/  
    BMP280_T_MODE_2,            /*x2*/  
    BMP280_T_MODE_3,            /*x4*/  
    BMP280_T_MODE_4,            /*x8*/  
    BMP280_T_MODE_5             /*x16*/  
} BMP280_T_OVERSAMPLING;  

typedef enum {  
    BMP280_SLEEP_MODE = 0x0,  
    BMP280_FORCED_MODE,  
    BMP280_NORMAL_MODE  
} BMP280_WORK_MODE;  

typedef enum {  
    BMP280_P_MODE_SKIP = 0x0,   /*skipped*/  
    BMP280_P_MODE_1,            /*x1*/  
    BMP280_P_MODE_2,            /*x2*/  
    BMP280_P_MODE_3,            /*x4*/  
    BMP280_P_MODE_4,            /*x8*/  
    BMP280_P_MODE_5             /*x16*/  
} BMP280_P_OVERSAMPLING;  

typedef enum {  
    BMP280_FILTER_OFF = 0x0,    /*filter off*/  
    BMP280_FILTER_MODE_1,       /*0.223*ODR*/  
    BMP280_FILTER_MODE_2,       /*0.092*ODR*/  
    BMP280_FILTER_MODE_3,       /*0.042*ODR*/  
    BMP280_FILTER_MODE_4        /*0.021*ODR*/  
} BMP280_FILTER_COEFFICIENT;  

typedef enum {  
    BMP280_T_SB1 = 0x0,     /*0.5ms*/  
    BMP280_T_SB2,           /*62.5ms*/  
    BMP280_T_SB3,           /*125ms*/  
    BMP280_T_SB4,           /*250ms*/  
    BMP280_T_SB5,           /*500ms*/  
    BMP280_T_SB6,           /*1000ms*/  
    BMP280_T_SB7,           /*2000ms*/  
    BMP280_T_SB8,           /*4000ms*/  
} BMP280_T_SB;  

struct bmp280 {  
    I2C_HandleTypeDef I2cHandle;  
    /* T1~P9 为补偿系数 */  
    uint16_t T1;  
    int16_t T2;  
    int16_t T3;  
    uint16_t P1;
    int16_t P2;  
    int16_t P3;  
    int16_t P4;  
    int16_t P5;  
    int16_t P6;  
    int16_t P7;  
    int16_t P8;  
    int16_t P9;  
    int32_t t_fine;  
    uint8_t t_sb;  
    uint8_t mode;  
    uint8_t t_oversampling;  
    uint8_t p_oversampling;  
    uint8_t filter_coefficient;  
};  

extern struct bmp280 *bmp280_init(I2C_HandleTypeDef I2cHandle);  

extern void bmp280_reset(struct bmp280 *bmp280);  

extern void bmp280_set_standby_time(struct bmp280 *bmp280, BMP280_T_SB t_standby);  

extern void bmp280_set_work_mode(struct bmp280 *bmp280, BMP280_WORK_MODE mode);  

extern void bmp280_set_temperature_oversampling_mode(struct bmp280 *bmp280, BMP280_T_OVERSAMPLING t_osl);  

extern void bmp280_set_pressure_oversampling_mode(struct bmp280 *bmp280, BMP280_P_OVERSAMPLING p_osl);  

extern void bmp280_set_filter_mode(struct bmp280 *bmp280, BMP280_FILTER_COEFFICIENT f_coefficient);  

extern double bmp280_get_temperature(struct bmp280 *bmp280);  

extern double bmp280_get_pressure(struct bmp280 *bmp280);  

extern void bmp280_get_temperature_and_pressure(struct bmp280 *bmp280, doubledouble *temperature, doubledouble *pressure);  

extern void bmp280_forced_mode_get_temperature_and_pressure(struct bmp280 *bmp280, doubledouble *temperature, doubledouble *pressure);  

extern void bmp280_demo(I2C_HandleTypeDef I2cHandle, doubledouble *temperature, doubledouble *pressure);  

#endif  







#include <stdlib.h>  
#include "stm32f4xx_hal.h"  
#include "stm32f4xx_hal_i2c.h"  
#include <sensors/bmp280.h>  

#define dig_T1 bmp280->T1  
#define dig_T2 bmp280->T2  
#define dig_T3 bmp280->T3  
#define dig_P1 bmp280->P1  
#define dig_P2 bmp280->P2  
#define dig_P3 bmp280->P3  
#define dig_P4 bmp280->P4  
#define dig_P5 bmp280->P5  
#define dig_P6 bmp280->P6  
#define dig_P7 bmp280->P7  
#define dig_P8 bmp280->P8  
#define dig_P9 bmp280->P9  

static uint8_t bmp280_read_register(I2C_HandleTypeDef Bmp280_I2cHandle, uint8_t reg_addr)  
{  
    uint8_t reg_data;  

    while(HAL_I2C_Master_Transmit(&Bmp280_I2cHandle, BMP280_ADDRESS, &reg_addr, 1, 10000) != HAL_OK) {  
        if(HAL_I2C_GetError(&Bmp280_I2cHandle) != HAL_I2C_ERROR_AF) {  
            printf("Transmit slave address error!!!\r\n");  
            return -1;  
        }  
    }  

    while(HAL_I2C_Master_Receive(&Bmp280_I2cHandle, BMP280_ADDRESS, &reg_data, 1, 10000) != HAL_OK) {  
        if(HAL_I2C_GetError(&Bmp280_I2cHandle) != HAL_I2C_ERROR_AF) {  
            printf("Receive slave data error!!!\r\n");  
            return -1;  
        }  
    }  

    return reg_data;  
}  

static void bmp280_write_register(I2C_HandleTypeDef Bmp280_I2cHandle, uint8_t reg_addr, uint8_t reg_data)  
{  
    uint8_t tx_data[2] = {reg_addr, reg_data};  

    while(HAL_I2C_Master_Transmit(&Bmp280_I2cHandle, BMP280_ADDRESS, tx_data, 2, 10000) != HAL_OK) {  
        if(HAL_I2C_GetError(&Bmp280_I2cHandle) != HAL_I2C_ERROR_AF) {  
            printf("Transmit slave address error!!!\r\n");  
        }  
    }  
}  

/** 
 * 在bmp280_init()函数里默认初始化t_standby为0.5ms， 
 * 温度和气压的采样精度设为最低， 
 * 滤波器系数设为最低， 
 * 并且进入sleep mode。 
 */  
struct bmp280 *bmp280_init(I2C_HandleTypeDef I2cHandle)  
{
    uint8_t bmp280_id;  
    uint8_t lsb, msb;  
    uint8_t ctrlmeas_reg, config_reg;  
    struct bmp280 *bmp280;  

    bmp280_id = bmp280_read_register(I2cHandle, BMP280_CHIPID_REG);  
    if(bmp280_id == 0x58) {  
        bmp280 = malloc(sizeof(struct bmp280));  

        bmp280->I2cHandle = I2cHandle;  
        bmp280->mode = BMP280_SLEEP_MODE;  
        bmp280->t_sb = BMP280_T_SB1;  
        bmp280->p_oversampling = BMP280_P_MODE_1;  
        bmp280->t_oversampling = BMP280_T_MODE_1;  
        bmp280->filter_coefficient = BMP280_FILTER_MODE_1;  
    } else {  
        printf("Read BMP280 id error!\r\n");  
        return NULL;  
    }  

    /* read the temperature calibration parameters */  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_T1_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_T1_MSB_REG);  
    dig_T1 = msb << 8 | lsb;  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_T2_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_T2_MSB_REG);  
    dig_T2 = msb << 8 | lsb;  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_T3_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_T3_MSB_REG);  
    dig_T3 = msb << 8 | lsb;  

    /* read the pressure calibration parameters */  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_P1_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_P1_MSB_REG);  
    dig_P1 = msb << 8 | lsb;  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_P2_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_P2_MSB_REG);  
    dig_P2 = msb << 8 | lsb;  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_P3_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_P3_MSB_REG);  
    dig_P3 = msb << 8 | lsb;  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_P4_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_P4_MSB_REG);  
    dig_P4 = msb << 8 | lsb;  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_P5_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_P5_MSB_REG);  
    dig_P5 = msb << 8 | lsb;  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_P6_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_P6_MSB_REG);  
    dig_P6 = msb << 8 | lsb;  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_P7_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_P7_MSB_REG);  
    dig_P7 = msb << 8 | lsb;  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_P8_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_P8_MSB_REG);  
    dig_P8 = msb << 8 | lsb;  
    lsb = bmp280_read_register(I2cHandle, BMP280_DIG_P9_LSB_REG);  
    msb = bmp280_read_register(I2cHandle, BMP280_DIG_P9_MSB_REG);  
    dig_P9 = msb << 8 | lsb;  

    bmp280_reset(bmp280);  

    ctrlmeas_reg = bmp280->t_oversampling << 5 | bmp280->p_oversampling << 2 | bmp280->mode;  
    config_reg = bmp280->t_sb << 5 | bmp280->filter_coefficient << 2;  

    bmp280_write_register(I2cHandle, BMP280_CTRLMEAS_REG, ctrlmeas_reg);  
    bmp280_write_register(I2cHandle, BMP280_CONFIG_REG, config_reg);  

    HAL_Delay(100);  

    return bmp280;  
}  

void bmp280_reset(struct bmp280 *bmp280)  
{  
    bmp280_write_register(bmp280->I2cHandle, BMP280_RESET_REG, BMP280_RESET_VALUE);  
}  

void bmp280_set_standby_time(struct bmp280 *bmp280, BMP280_T_SB t_standby)  
{  
    uint8_t config_reg;  

    bmp280->t_sb = t_standby;  
    config_reg = bmp280->t_sb << 5 | bmp280->filter_coefficient << 2;  

    bmp280_write_register(bmp280->I2cHandle, BMP280_CONFIG_REG, config_reg);  
}  

void bmp280_set_work_mode(struct bmp280 *bmp280, BMP280_WORK_MODE mode)  
{  
    uint8_t ctrlmeas_reg;  

    bmp280->mode = mode;  
    ctrlmeas_reg = bmp280->t_oversampling << 5 | bmp280->p_oversampling << 2 | bmp280->mode;  

    bmp280_write_register(bmp280->I2cHandle, BMP280_CTRLMEAS_REG, ctrlmeas_reg);  
}  

void bmp280_set_temperature_oversampling_mode(struct bmp280 *bmp280, BMP280_T_OVERSAMPLING t_osl)  
{  
    uint8_t ctrlmeas_reg;  

    bmp280->t_oversampling = t_osl;  
    ctrlmeas_reg = bmp280->t_oversampling << 5 | bmp280->p_oversampling << 2 | bmp280->mode;  

    bmp280_write_register(bmp280->I2cHandle, BMP280_CTRLMEAS_REG, ctrlmeas_reg);  
}  

void bmp280_set_pressure_oversampling_mode(struct bmp280 *bmp280, BMP280_P_OVERSAMPLING p_osl)  
{  
    uint8_t ctrlmeas_reg;  

    bmp280->t_oversampling = p_osl;  
    ctrlmeas_reg = bmp280->t_oversampling << 5 | bmp280->p_oversampling << 2 | bmp280->mode;  

    bmp280_write_register(bmp280->I2cHandle, BMP280_CTRLMEAS_REG, ctrlmeas_reg);  
}  

void bmp280_set_filter_mode(struct bmp280 *bmp280, BMP280_FILTER_COEFFICIENT f_coefficient)  
{  
    uint8_t config_reg;  

    bmp280->filter_coefficient = f_coefficient;  
    config_reg = bmp280->t_sb << 5 | bmp280->filter_coefficient << 2;  

    bmp280_write_register(bmp280->I2cHandle, BMP280_CONFIG_REG, config_reg);  
}  

/* Returns temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC. */  
static double bmp280_compensate_temperature_double(struct bmp280 *bmp280, int32_t adc_T)  
{
    double var1, var2, temperature;  

    var1 = (((double) adc_T) / 16384.0 - ((double) dig_T1) / 1024.0)  
        * ((double) dig_T2);  
    var2 = ((((double) adc_T) / 131072.0 - ((double) dig_T1) / 8192.0)  
            * (((double) adc_T) / 131072.0 - ((double) dig_T1) / 8192.0))  
        * ((double) dig_T3);
    bmp280->t_fine = (int32_t) (var1 + var2);  
    temperature = (var1 + var2) / 5120.0;  

    return temperature;  
}  


/* Returns pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa */  
static double bmp280_compensate_pressure_double(struct bmp280 *bmp280, int32_t adc_P)  
{  
    double var1, var2, pressure;  

    var1 = ((double) bmp280->t_fine / 2.0) - 64000.0;  
    var2 = var1 * var1 * ((double) dig_P6) / 32768.0;  
    var2 = var2 + var1 * ((double) dig_P5) * 2.0;  
    var2 = (var2 / 4.0) + (((double) dig_P4) * 65536.0);
    var1 = (((double) dig_P3) * var1 * var1 / 524288.0  
            + ((double) dig_P2) * var1) / 524288.0;  
    var1 = (1.0 + var1 / 32768.0) * ((double) dig_P1);  

    if (var1 == 0.0) {  
        return 0; // avoid exception caused by division by zero  
    }  

    pressure = 1048576.0 - (double) adc_P;  
    pressure = (pressure - (var2 / 4096.0)) * 6250.0 / var1;  
    var1 = ((double) dig_P9) * pressure * pressure / 2147483648.0;  
    var2 = pressure * ((double) dig_P8) / 32768.0;  
    pressure = pressure + (var1 + var2 + ((double) dig_P7)) / 16.0;  

    return pressure;  
}  

#if 0  
static int32_t bmp280_compensate_temperature_int32(struct bmp280 *bmp280, int32_t adc_T)  
{  
    int32_t var1, var2, temperature;  

    var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;  
    var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;  
    bmp280->t_fine = var1 + var2;  
    temperature = (bmp280->t_fine * 5 + 128) >> 8;  

    return temperature;  
}  

static uint32_t bmp280_compensate_pressure_int64(struct bmp280 *bmp280, int32_t adc_P)  
{  
    int64_t var1, var2, pressure;  

    var1 = ((int64_t)bmp280->t_fine) - 128000;  
    var2 = var1 * var1 * (int64_t)dig_P6;  
    var2 = var2 + ((var1*(int64_t)dig_P5)<<17);  
    var2 = var2 + (((int64_t)dig_P4)<<35);  
    var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);  
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33;  
    if (var1 == 0)  
    {  
        return 0; // avoid exception caused by division by zero  
    }  

    pressure = 1048576-adc_P;  
    pressure = (((pressure<<31)-var2)*3125)/var1;  
    var1 = (((int64_t)dig_P9) * (pressure>>13) * (pressure>>13)) >> 25;  
    var2 = (((int64_t)dig_P8) * pressure) >> 19;  
    pressure = ((pressure + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4);  

    return (uint32_t)pressure;  
}  
#endif  

/* Returns temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC. */  
double bmp280_get_temperature(struct bmp280 *bmp280)  
{  
    uint8_t lsb, msb, xlsb;  
    int32_t adc_T;  
    double temperature;  

    xlsb = bmp280_read_register(bmp280->I2cHandle, BMP280_TEMPERATURE_XLSB_REG);  
    lsb = bmp280_read_register(bmp280->I2cHandle, BMP280_TEMPERATURE_LSB_REG);  
    msb = bmp280_read_register(bmp280->I2cHandle, BMP280_TEMPERATURE_MSB_REG);  

    adc_T = (msb << 12) | (lsb << 4) | (xlsb >> 4);  
    temperature = bmp280_compensate_temperature_double(bmp280, adc_T);  

    return temperature;  
}  

/* Returns pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa */  
double bmp280_get_pressure(struct bmp280 *bmp280)  
{  
    uint8_t lsb, msb, xlsb;  
    int32_t adc_P;  
    double pressure;  


    xlsb = bmp280_read_register(bmp280->I2cHandle, BMP280_PRESSURE_XLSB_REG);  
    lsb = bmp280_read_register(bmp280->I2cHandle, BMP280_PRESSURE_LSB_REG);  
    msb = bmp280_read_register(bmp280->I2cHandle, BMP280_PRESSURE_MSB_REG);  

    adc_P = (msb << 12) | (lsb << 4) | (xlsb >> 4);  
    pressure = bmp280_compensate_pressure_double(bmp280, adc_P);  

    return pressure;  
}  

/** 
 * 仅在BMP280被设置为normal mode后， 
 * 可使用该接口直接读取温度和气压。 
 */  
void bmp280_get_temperature_and_pressure(struct bmp280 *bmp280, doubledouble *temperature, doubledouble *pressure)  
{  
    *temperature = bmp280_get_temperature(bmp280);  
    *pressure = bmp280_get_pressure(bmp280);  
}  

/** 
 * 当BMP280被设置为forced mode后， 
 * 可使用该接口直接读取温度和气压。 
 */  
void bmp280_forced_mode_get_temperature_and_pressure(struct bmp280 *bmp280, doubledouble *temperature, doubledouble *pressure)  
{  
    bmp280_set_work_mode(bmp280, BMP280_FORCED_MODE);  

    HAL_Delay(100);  

    bmp280_get_temperature_and_pressure(bmp280, temperature, pressure);  
}  

/** 
 * 此demo使用forced mode以1s为周期， 
 * 对温度和气压进行数据采集并打印。 
 */  
void bmp280_demo(I2C_HandleTypeDef I2cHandle, doubledouble *temperature, doubledouble *pressure)  
{  
    struct bmp280 *bmp280;  
    bmp280 = bmp280_init(I2cHandle);  

    if(bmp280 != NULL) {  
        while(1) {  
            bmp280_forced_mode_get_temperature_and_pressure(bmp280, temperature, pressure);  
            printf("temperature=%ld   pressure=%ld\r\n", (int32_t)*temperature, (uint32_t)*pressure);  

            HAL_Delay(1000);  
        }  
    } else  
        printf("create bmp280 error!\r\n");  
}


int main() {
    uint8_t bmp280_id = 0;  
    uint8_t ctr_reg = 0;  
    uint8_t status_reg = 0;  

    int32_t tem = 0;  
    uint32_t pressure = 0;  

    bmp280_id = bmp280_init(&I2cHandle);  
    if(bmp280_id == 0x58) {  
        bmp280_reset(&I2cHandle);  

        ctr_reg = bmp280_read_register(&I2cHandle, 0xF4);  

        printf("ctr_reg1111===0x%x\r\n", ctr_reg);  

        bmp280_write_register(&I2cHandle, 0xF4, 0xFF);  

        bmp280_write_register(&I2cHandle, 0xF5, 0x14);  

        HAL_Delay(100);  

        while(1) {  
            ctr_reg = bmp280_read_register(&I2cHandle, 0xF4);  

            printf("ctr_reg2222===0x%x\r\n", ctr_reg);  

            status_reg = bmp280_read_register(&I2cHandle, 0xF3);  
            printf("status == 0x%x\r\n", status_reg);  


            tem = bmp280_get_temperature(&I2cHandle);  
            pressure = bmp280_get_pressure(&I2cHandle);  

            printf("bmp280_id = 0x%x  tem=%ld    pressure=%ld   \r\n", bmp280_id, tem, pressure/256);  

            HAL_Delay(2000);  
        }  
    }  


}


