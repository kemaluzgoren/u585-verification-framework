/**
 * @file    i2c_bus.h
 * @brief   Shared-I2C-peripheral access. Serialized across ThreadX threads
 *          when built with USE_THREADX defined; a plain no-op lock (and so
 *          safe to drop into a bare-metal/superloop build) otherwise.
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

#ifdef USE_THREADX
#include "tx_api.h"
#endif

typedef struct {
    I2C_HandleTypeDef *hi2c;
#ifdef USE_THREADX
    TX_MUTEX lock;
#endif
} I2C_Bus_t;

void I2C_Bus_Init(I2C_Bus_t *bus, I2C_HandleTypeDef *hi2c);

int32_t I2C_Bus_IsDeviceReady(I2C_Bus_t *bus, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);
int32_t I2C_Bus_MemRead(I2C_Bus_t *bus, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Length, uint32_t Timeout);
int32_t I2C_Bus_MemWrite(I2C_Bus_t *bus, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Length, uint32_t Timeout);

#endif /* I2C_BUS_H_ */
