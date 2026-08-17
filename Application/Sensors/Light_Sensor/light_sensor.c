/**
 * @file    light_sensor.c
 * @brief   Application system interface.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-17
 *
 * SPDX-License-Identifier: MIT
 */

#include "light_sensor.h"

#include "veml6030.h"
#include "veml6030_reg.h"
#include <string.h>

/* lux per ALS count at the VEML6030 power-up default (Gain x1, IT 100 ms) */
#define LIGHT_SENSOR_LUX_RESOLUTION  (0.0576f)
#define LIGHT_SENSOR_I2C_TIMEOUT     (100U)

static I2C_HandleTypeDef *i2c = NULL;
static VEML6030_Object_t veml6030_obj;

static int32_t Light_Sensor_IO_Init(void);
static int32_t Light_Sensor_IO_DeInit(void);
static int32_t Light_Sensor_IO_IsReady(uint16_t DevAddress, uint32_t Trials);
static int32_t Light_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Light_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Light_Sensor_IO_GetTick(void);

int32_t light_sensor_init(I2C_HandleTypeDef *hi2c) {

    int32_t ret;

    if(hi2c == NULL) {
        ret = VEML6030_ERROR;
    }
    else {
        i2c = hi2c;

        VEML6030_IO_t io = {0};
        io.Init         = Light_Sensor_IO_Init;
        io.DeInit       = Light_Sensor_IO_DeInit;
        io.ReadAddress  = VEML6030_I2C_READ_ADD;
        io.WriteAddress = VEML6030_I2C_WRITE_ADD;
        io.IsReady      = Light_Sensor_IO_IsReady;
        io.WriteReg     = Light_Sensor_IO_Write;
        io.ReadReg      = Light_Sensor_IO_Read;
        io.GetTick      = Light_Sensor_IO_GetTick;

        ret = VEML6030_RegisterBusIO(&veml6030_obj, &io);
        if(ret == VEML6030_OK) {
            ret = VEML6030_Init(&veml6030_obj);
        }
    }

    return ret;
}

int32_t light_sensor_read_raw(uint16_t *als_raw, uint16_t *white_raw) {

    uint32_t values[VEML6030_MAX_CHANNELS] = {0};
    int32_t ret;

    if((als_raw == NULL) || (white_raw == NULL)) {
        ret = VEML6030_ERROR;
    }
    else {
        ret = VEML6030_GetValues(&veml6030_obj, values);
        if(ret == VEML6030_OK) {
            *als_raw   = (uint16_t)values[VEML6030_ALS_CHANNEL];
            *white_raw = (uint16_t)values[VEML6030_WHITE_CHANNEL];
        }
    }

    return ret;
}

int32_t light_sensor_read_lux(float *lux) {

    uint16_t als_raw;
    uint16_t white_raw;
    int32_t ret;

    if(lux == NULL) {
        ret = VEML6030_ERROR;
    }
    else {
        ret = light_sensor_read_raw(&als_raw, &white_raw);
        if(ret == VEML6030_OK) {
            *lux = (float)als_raw * LIGHT_SENSOR_LUX_RESOLUTION;
        }
    }

    return ret;
}

static int32_t Light_Sensor_IO_Init(void) {
    return VEML6030_OK;
}

static int32_t Light_Sensor_IO_DeInit(void) {
    return VEML6030_OK;
}

static int32_t Light_Sensor_IO_IsReady(uint16_t DevAddress, uint32_t Trials) {
    return (int32_t)HAL_I2C_IsDeviceReady(i2c, DevAddress, Trials, LIGHT_SENSOR_I2C_TIMEOUT);
}

static int32_t Light_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return (int32_t)HAL_I2C_Mem_Write(i2c, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, LIGHT_SENSOR_I2C_TIMEOUT);
}

static int32_t Light_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return (int32_t)HAL_I2C_Mem_Read(i2c, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, LIGHT_SENSOR_I2C_TIMEOUT);
}

static int32_t Light_Sensor_IO_GetTick(void) {
    return (int32_t)HAL_GetTick();
}
