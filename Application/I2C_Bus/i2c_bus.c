/**
 * @file    i2c_bus.c
 * @brief   Shared-I2C-peripheral access, guarded for future multi-task use.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-20
 *
 * SPDX-License-Identifier: MIT
 */

#include "i2c_bus.h"

static void I2C_Bus_Lock(I2C_Bus_t *bus) {

    (void)bus;
    /* TODO(ThreadX): tx_mutex_get(&bus->lock, TX_WAIT_FOREVER); */
}

static void I2C_Bus_Unlock(I2C_Bus_t *bus) {

    (void)bus;
    /* TODO(ThreadX): tx_mutex_put(&bus->lock); */
}

void I2C_Bus_Init(I2C_Bus_t *bus, I2C_HandleTypeDef *hi2c) {

    bus->hi2c = hi2c;
    /* TODO(ThreadX): tx_mutex_create(&bus->lock, "i2c_bus", TX_NO_INHERIT); */
}

int32_t I2C_Bus_IsDeviceReady(I2C_Bus_t *bus, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout) {

    int32_t ret;

    I2C_Bus_Lock(bus);
    ret = (int32_t)HAL_I2C_IsDeviceReady(bus->hi2c, DevAddress, Trials, Timeout);
    I2C_Bus_Unlock(bus);

    return ret;
}

int32_t I2C_Bus_MemRead(I2C_Bus_t *bus, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Length, uint32_t Timeout) {

    int32_t ret;

    I2C_Bus_Lock(bus);
    ret = (int32_t)HAL_I2C_Mem_Read(bus->hi2c, DevAddress, MemAddress, MemAddSize, pData, Length, Timeout);
    I2C_Bus_Unlock(bus);

    return ret;
}

int32_t I2C_Bus_MemWrite(I2C_Bus_t *bus, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Length, uint32_t Timeout) {

    int32_t ret;

    I2C_Bus_Lock(bus);
    ret = (int32_t)HAL_I2C_Mem_Write(bus->hi2c, DevAddress, MemAddress, MemAddSize, pData, Length, Timeout);
    I2C_Bus_Unlock(bus);

    return ret;
}
