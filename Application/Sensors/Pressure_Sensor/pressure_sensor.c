/**
 * @file    pressure_sensor.c
 * @brief   Application system interface.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#include "pressure_sensor.h"

#include "lps22hh.h"
#include "lps22hh_reg.h"

#define PRESSURE_SENSOR_I2C_TIMEOUT     (100U)

static I2C_Bus_t *bus = NULL;
static LPS22HH_Object_t lps22hh_obj;

static int32_t Pressure_Sensor_IO_Init(void);
static int32_t Pressure_Sensor_IO_DeInit(void);
static int32_t Pressure_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Pressure_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Pressure_Sensor_IO_GetTick(void);
static void Pressure_Sensor_IO_Delay(uint32_t ms);

int32_t pressure_sensor_init(I2C_Bus_t *i2c_bus) {

    int32_t ret;

    if(i2c_bus == NULL) {
        ret = LPS22HH_ERROR;
    }
    else {
        bus = i2c_bus;

        LPS22HH_IO_t io = {0};
        io.Init     = Pressure_Sensor_IO_Init;
        io.DeInit   = Pressure_Sensor_IO_DeInit;
        io.BusType  = LPS22HH_I2C_BUS;
        io.Address  = LPS22HH_I2C_ADD_H;
        io.WriteReg = Pressure_Sensor_IO_Write;
        io.ReadReg  = Pressure_Sensor_IO_Read;
        io.GetTick  = Pressure_Sensor_IO_GetTick;
        io.Delay    = Pressure_Sensor_IO_Delay;

        ret = LPS22HH_RegisterBusIO(&lps22hh_obj, &io);
        if(ret == LPS22HH_OK) {
            ret = LPS22HH_Init(&lps22hh_obj);
        }
        if(ret == LPS22HH_OK) {
            ret = LPS22HH_PRESS_Enable(&lps22hh_obj);
        }
        if(ret == LPS22HH_OK) {
            ret = LPS22HH_TEMP_Enable(&lps22hh_obj);
        }
    }

    return ret;
}

int32_t pressure_sensor_read(float *pressure_hpa, float *temperature_degC) {

    int32_t ret;

    if((pressure_hpa == NULL) || (temperature_degC == NULL)) {
        ret = LPS22HH_ERROR;
    }
    else {
        ret = LPS22HH_PRESS_GetPressure(&lps22hh_obj, pressure_hpa);
        if(ret == LPS22HH_OK) {
            ret = LPS22HH_TEMP_GetTemperature(&lps22hh_obj, temperature_degC);
        }
    }

    return ret;
}

static int32_t Pressure_Sensor_IO_Init(void) {
    return LPS22HH_OK;
}

static int32_t Pressure_Sensor_IO_DeInit(void) {
    return LPS22HH_OK;
}

static int32_t Pressure_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemWrite(bus, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, PRESSURE_SENSOR_I2C_TIMEOUT);
}

static int32_t Pressure_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemRead(bus, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, PRESSURE_SENSOR_I2C_TIMEOUT);
}

static int32_t Pressure_Sensor_IO_GetTick(void) {
    return (int32_t)HAL_GetTick();
}

static void Pressure_Sensor_IO_Delay(uint32_t ms) {
    HAL_Delay(ms);
}
