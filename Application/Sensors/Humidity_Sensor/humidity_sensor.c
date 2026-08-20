/**
 * @file    humidity_sensor.c
 * @brief   Application system interface.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#include "humidity_sensor.h"

#include "hts221.h"
#include "hts221_reg.h"

#define HUMIDITY_SENSOR_I2C_TIMEOUT     (100U)

static I2C_Bus_t *bus = NULL;
static HTS221_Object_t hts221_obj;

static int32_t Humidity_Sensor_IO_Init(void);
static int32_t Humidity_Sensor_IO_DeInit(void);
static int32_t Humidity_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Humidity_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Humidity_Sensor_IO_GetTick(void);
static void Humidity_Sensor_IO_Delay(uint32_t ms);

int32_t humidity_sensor_init(I2C_Bus_t *i2c_bus) {

    int32_t ret;

    if(i2c_bus == NULL) {
        ret = HTS221_ERROR;
    }
    else {
        bus = i2c_bus;

        HTS221_IO_t io = {0};
        io.Init     = Humidity_Sensor_IO_Init;
        io.DeInit   = Humidity_Sensor_IO_DeInit;
        io.BusType  = HTS221_I2C_BUS;
        io.Address  = HTS221_I2C_ADDRESS;
        io.WriteReg = Humidity_Sensor_IO_Write;
        io.ReadReg  = Humidity_Sensor_IO_Read;
        io.GetTick  = Humidity_Sensor_IO_GetTick;
        io.Delay    = Humidity_Sensor_IO_Delay;

        ret = HTS221_RegisterBusIO(&hts221_obj, &io);
        if(ret == HTS221_OK) {
            ret = HTS221_Init(&hts221_obj);
        }
        if(ret == HTS221_OK) {
            ret = HTS221_HUM_Enable(&hts221_obj);
        }
        if(ret == HTS221_OK) {
            ret = HTS221_TEMP_Enable(&hts221_obj);
        }
    }

    return ret;
}

int32_t humidity_sensor_read(float *temperature_degC, float *humidity_rh) {

    int32_t ret;

    if((temperature_degC == NULL) || (humidity_rh == NULL)) {
        ret = HTS221_ERROR;
    }
    else {
        ret = HTS221_TEMP_GetTemperature(&hts221_obj, temperature_degC);
        if(ret == HTS221_OK) {
            ret = HTS221_HUM_GetHumidity(&hts221_obj, humidity_rh);
        }
    }

    return ret;
}

static int32_t Humidity_Sensor_IO_Init(void) {
    return HTS221_OK;
}

static int32_t Humidity_Sensor_IO_DeInit(void) {
    return HTS221_OK;
}

static int32_t Humidity_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemWrite(bus, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, HUMIDITY_SENSOR_I2C_TIMEOUT);
}

static int32_t Humidity_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemRead(bus, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, HUMIDITY_SENSOR_I2C_TIMEOUT);
}

static int32_t Humidity_Sensor_IO_GetTick(void) {
    return (int32_t)HAL_GetTick();
}

static void Humidity_Sensor_IO_Delay(uint32_t ms) {
    HAL_Delay(ms);
}
