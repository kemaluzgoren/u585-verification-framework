/**
 * @file    imu_sensor.c
 * @brief   Application system interface.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#include "imu_sensor.h"

#include "ism330dhcx.h"
#include "ism330dhcx_reg.h"

#define IMU_SENSOR_I2C_TIMEOUT     (100U)

static I2C_Bus_t *bus = NULL;
static ISM330DHCX_Object_t ism330dhcx_obj;

static int32_t IMU_Sensor_IO_Init(void);
static int32_t IMU_Sensor_IO_DeInit(void);
static int32_t IMU_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t IMU_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t IMU_Sensor_IO_GetTick(void);
static void IMU_Sensor_IO_Delay(uint32_t ms);

int32_t imu_sensor_init(I2C_Bus_t *i2c_bus) {

    int32_t ret;

    if(i2c_bus == NULL) {
        ret = ISM330DHCX_ERROR;
    }
    else {
        bus = i2c_bus;

        ISM330DHCX_IO_t io = {0};
        io.Init     = IMU_Sensor_IO_Init;
        io.DeInit   = IMU_Sensor_IO_DeInit;
        io.BusType  = ISM330DHCX_I2C_BUS;
        io.Address  = ISM330DHCX_I2C_ADD_H;
        io.WriteReg = IMU_Sensor_IO_Write;
        io.ReadReg  = IMU_Sensor_IO_Read;
        io.GetTick  = IMU_Sensor_IO_GetTick;
        io.Delay    = IMU_Sensor_IO_Delay;

        ret = ISM330DHCX_RegisterBusIO(&ism330dhcx_obj, &io);
        if(ret == ISM330DHCX_OK) {
            ret = ISM330DHCX_Init(&ism330dhcx_obj);
        }
        if(ret == ISM330DHCX_OK) {
            ret = ISM330DHCX_ACC_Enable(&ism330dhcx_obj);
        }
        if(ret == ISM330DHCX_OK) {
            ret = ISM330DHCX_GYRO_Enable(&ism330dhcx_obj);
        }
    }

    return ret;
}

int32_t imu_sensor_read_accel(int32_t *ax_mg, int32_t *ay_mg, int32_t *az_mg) {

    int32_t ret;
    ISM330DHCX_Axes_t axes;

    if((ax_mg == NULL) || (ay_mg == NULL) || (az_mg == NULL)) {
        ret = ISM330DHCX_ERROR;
    }
    else {
        ret = ISM330DHCX_ACC_GetAxes(&ism330dhcx_obj, &axes);
        if(ret == ISM330DHCX_OK) {
            *ax_mg = axes.x;
            *ay_mg = axes.y;
            *az_mg = axes.z;
        }
    }

    return ret;
}

int32_t imu_sensor_read_gyro(int32_t *gx_mdps, int32_t *gy_mdps, int32_t *gz_mdps) {

    int32_t ret;
    ISM330DHCX_Axes_t axes;

    if((gx_mdps == NULL) || (gy_mdps == NULL) || (gz_mdps == NULL)) {
        ret = ISM330DHCX_ERROR;
    }
    else {
        ret = ISM330DHCX_GYRO_GetAxes(&ism330dhcx_obj, &axes);
        if(ret == ISM330DHCX_OK) {
            *gx_mdps = axes.x;
            *gy_mdps = axes.y;
            *gz_mdps = axes.z;
        }
    }

    return ret;
}

static int32_t IMU_Sensor_IO_Init(void) {
    return ISM330DHCX_OK;
}

static int32_t IMU_Sensor_IO_DeInit(void) {
    return ISM330DHCX_OK;
}

static int32_t IMU_Sensor_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemWrite(bus, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, IMU_SENSOR_I2C_TIMEOUT);
}

static int32_t IMU_Sensor_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemRead(bus, DevAddress, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, IMU_SENSOR_I2C_TIMEOUT);
}

static int32_t IMU_Sensor_IO_GetTick(void) {
    return (int32_t)HAL_GetTick();
}

static void IMU_Sensor_IO_Delay(uint32_t ms) {
    HAL_Delay(ms);
}
