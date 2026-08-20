/**
 * @file    i2c_bus.h
 * @brief   Shared-I2C-peripheral access, guarded for future multi-task use.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-20
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef I2C_BUS_H_
#define I2C_BUS_H_

#include <stdint.h>

#include "stm32u5xx_hal.h"

typedef struct {
    I2C_HandleTypeDef *hi2c;
    /* TODO(ThreadX): add a TX_MUTEX lock member here once ThreadX is
     * integrated, create it in I2C_Bus_Init(), and acquire/release it in
     * i2c_bus.c's I2C_Bus_Lock()/I2C_Bus_Unlock(). Those two are no-ops
     * today because the app is still a single-threaded superloop; every
     * sensor module already routes its transfers through this struct, so
     * that change stays local to i2c_bus.c. */
} I2C_Bus_t;

void I2C_Bus_Init(I2C_Bus_t *bus, I2C_HandleTypeDef *hi2c);

int32_t I2C_Bus_IsDeviceReady(I2C_Bus_t *bus, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);
int32_t I2C_Bus_MemRead(I2C_Bus_t *bus, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Length, uint32_t Timeout);
int32_t I2C_Bus_MemWrite(I2C_Bus_t *bus, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Length, uint32_t Timeout);

#endif /* I2C_BUS_H_ */
