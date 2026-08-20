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

#include <string.h>

#if LIGHT_SENSOR_CHIP == LIGHT_SENSOR_VEML3235

#include "veml3235.h"
#include "veml3235_reg.h"

#define LS_Object_t          VEML3235_Object_t
#define LS_IO_t              VEML3235_IO_t
#define LS_OK                VEML3235_OK
#define LS_ERROR             VEML3235_ERROR
#define LS_I2C_READ_ADD      VEML3235_I2C_READ_ADD
#define LS_I2C_WRITE_ADD     VEML3235_I2C_WRITE_ADD
#define LS_MAX_CHANNELS      VEML3235_MAX_CHANNELS
#define LS_ALS_CHANNEL       VEML3235_ALS_CHANNEL
#define LS_WHITE_CHANNEL     VEML3235_WHITE_CHANNEL
#define LS_RegisterBusIO     VEML3235_RegisterBusIO
#define LS_Init              VEML3235_Init
#define LS_GetValues         VEML3235_GetValues
#define LS_REG_ALS_CONF      VEML3235_REG_ALS_CONF
#define ls_read_reg          veml3235_read_reg

#else

#include "veml6030.h"
#include "veml6030_reg.h"

#define LS_Object_t          VEML6030_Object_t
#define LS_IO_t              VEML6030_IO_t
#define LS_OK                VEML6030_OK
#define LS_ERROR             VEML6030_ERROR
#define LS_I2C_READ_ADD      VEML6030_I2C_READ_ADD
#define LS_I2C_WRITE_ADD     VEML6030_I2C_WRITE_ADD
#define LS_MAX_CHANNELS      VEML6030_MAX_CHANNELS
#define LS_ALS_CHANNEL       VEML6030_ALS_CHANNEL
#define LS_WHITE_CHANNEL     VEML6030_WHITE_CHANNEL
#define LS_RegisterBusIO     VEML6030_RegisterBusIO
#define LS_Init              VEML6030_Init
#define LS_GetValues         VEML6030_GetValues
#define LS_REG_ALS_CONF      VEML6030_REG_ALS_CONF
#define ls_read_reg          veml6030_read_reg

#endif

/* lux per ALS count at the sensor's power-up default (Gain x1, IT 100 ms) */
#define LIGHT_SENSOR_LUX_RESOLUTION  (0.0576f)
#define LIGHT_SENSOR_I2C_TIMEOUT     (100U)

static I2C_Bus_t *bus = NULL;
static LS_Object_t light_sensor_obj;

static int32_t Light_Sensor_IO_Init(void);
static int32_t Light_Sensor_IO_DeInit(void);
static int32_t Light_Sensor_IO_IsReady(uint16_t DevAddress, uint32_t Trials);
static int32_t Light_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Light_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Light_Sensor_IO_GetTick(void);

int32_t light_sensor_init(I2C_Bus_t *i2c_bus) {

    int32_t ret;

    if(i2c_bus == NULL) {
        ret = LS_ERROR;
    }
    else {
        bus = i2c_bus;

        LS_IO_t io = {0};
        io.Init         = Light_Sensor_IO_Init;
        io.DeInit       = Light_Sensor_IO_DeInit;
        io.ReadAddress  = LS_I2C_READ_ADD;
        io.WriteAddress = LS_I2C_WRITE_ADD;
        io.IsReady      = Light_Sensor_IO_IsReady;
        io.WriteReg     = Light_Sensor_IO_Write;
        io.ReadReg      = Light_Sensor_IO_Read;
        io.GetTick      = Light_Sensor_IO_GetTick;

        ret = LS_RegisterBusIO(&light_sensor_obj, &io);
        if(ret == LS_OK) {
            ret = LS_Init(&light_sensor_obj);
        }
    }

    return ret;
}

int32_t light_sensor_read_raw(uint16_t *als_raw, uint16_t *white_raw) {

    uint32_t values[LS_MAX_CHANNELS] = {0};
    int32_t ret;

    if((als_raw == NULL) || (white_raw == NULL)) {
        ret = LS_ERROR;
    }
    else {
        ret = LS_GetValues(&light_sensor_obj, values);
        if(ret == LS_OK) {
            *als_raw   = (uint16_t)values[LS_ALS_CHANNEL];
            *white_raw = (uint16_t)values[LS_WHITE_CHANNEL];
        }
    }

    return ret;
}

int32_t light_sensor_read_lux(float *lux) {

    uint16_t als_raw;
    uint16_t white_raw;
    int32_t ret;

    if(lux == NULL) {
        ret = LS_ERROR;
    }
    else {
        ret = light_sensor_read_raw(&als_raw, &white_raw);
        if(ret == LS_OK) {
            *lux = (float)als_raw * LIGHT_SENSOR_LUX_RESOLUTION;
        }
    }

    return ret;
}

int32_t light_sensor_debug_read_als_conf(uint16_t *als_conf) {

    if(als_conf == NULL) {
        return LS_ERROR;
    }

    return ls_read_reg(&light_sensor_obj.Ctx, LS_REG_ALS_CONF, als_conf, 2);
}

static int32_t Light_Sensor_IO_Init(void) {
    return LS_OK;
}

static int32_t Light_Sensor_IO_DeInit(void) {
    return LS_OK;
}

static int32_t Light_Sensor_IO_IsReady(uint16_t DevAddress, uint32_t Trials) {
    return I2C_Bus_IsDeviceReady(bus, DevAddress, Trials, LIGHT_SENSOR_I2C_TIMEOUT);
}

static int32_t Light_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemWrite(bus, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, LIGHT_SENSOR_I2C_TIMEOUT);
}

static int32_t Light_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemRead(bus, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, LIGHT_SENSOR_I2C_TIMEOUT);
}

static int32_t Light_Sensor_IO_GetTick(void) {
    return (int32_t)HAL_GetTick();
}
