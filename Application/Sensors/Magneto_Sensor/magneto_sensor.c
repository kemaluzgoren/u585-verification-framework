/**
 * @file    magneto_sensor.c
 * @brief   Application system interface.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#include "magneto_sensor.h"

#include "iis2mdc.h"
#include "iis2mdc_reg.h"

#define MAGNETO_SENSOR_I2C_TIMEOUT     (100U)

static I2C_Bus_t *bus = NULL;
static IIS2MDC_Object_t iis2mdc_obj;

static int32_t Magneto_Sensor_IO_Init(void);
static int32_t Magneto_Sensor_IO_DeInit(void);
static int32_t Magneto_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Magneto_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Magneto_Sensor_IO_GetTick(void);
static void Magneto_Sensor_IO_Delay(uint32_t ms);

int32_t magneto_sensor_init(I2C_Bus_t *i2c_bus) {

    int32_t ret;

    if(i2c_bus == NULL) {
        ret = IIS2MDC_ERROR;
    }
    else {
        bus = i2c_bus;

        IIS2MDC_IO_t io = {0};
        io.Init     = Magneto_Sensor_IO_Init;
        io.DeInit   = Magneto_Sensor_IO_DeInit;
        io.BusType  = IIS2MDC_I2C_BUS;
        io.Address  = IIS2MDC_I2C_ADD;
        io.WriteReg = Magneto_Sensor_IO_Write;
        io.ReadReg  = Magneto_Sensor_IO_Read;
        io.GetTick  = Magneto_Sensor_IO_GetTick;
        io.Delay    = Magneto_Sensor_IO_Delay;

        ret = IIS2MDC_RegisterBusIO(&iis2mdc_obj, &io);
        if(ret == IIS2MDC_OK) {
            ret = IIS2MDC_Init(&iis2mdc_obj);
        }
        if(ret == IIS2MDC_OK) {
            ret = IIS2MDC_MAG_Enable(&iis2mdc_obj);
        }
    }

    return ret;
}

int32_t magneto_sensor_read(int32_t *mx_mgauss, int32_t *my_mgauss, int32_t *mz_mgauss) {

    int32_t ret;
    IIS2MDC_Axes_t axes;

    if((mx_mgauss == NULL) || (my_mgauss == NULL) || (mz_mgauss == NULL)) {
        ret = IIS2MDC_ERROR;
    }
    else {
        ret = IIS2MDC_MAG_GetAxes(&iis2mdc_obj, &axes);
        if(ret == IIS2MDC_OK) {
            *mx_mgauss = axes.x;
            *my_mgauss = axes.y;
            *mz_mgauss = axes.z;
        }
    }

    return ret;
}

static int32_t Magneto_Sensor_IO_Init(void) {
    return IIS2MDC_OK;
}

static int32_t Magneto_Sensor_IO_DeInit(void) {
    return IIS2MDC_OK;
}

static int32_t Magneto_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemWrite(bus, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, MAGNETO_SENSOR_I2C_TIMEOUT);
}

static int32_t Magneto_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemRead(bus, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, MAGNETO_SENSOR_I2C_TIMEOUT);
}

static int32_t Magneto_Sensor_IO_GetTick(void) {
    return (int32_t)HAL_GetTick();
}

static void Magneto_Sensor_IO_Delay(uint32_t ms) {
    HAL_Delay(ms);
}
