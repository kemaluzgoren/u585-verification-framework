/**
 * @file    light_sensor.h
 * @brief
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-17
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIGHT_SENSOR_H_
#define LIGHT_SENSOR_H_

#include <stdint.h>

#include "stm32u5xx_hal.h"

int32_t light_sensor_init(I2C_HandleTypeDef *hi2c);
int32_t light_sensor_read_raw(uint16_t *als_raw, uint16_t *white_raw);
int32_t light_sensor_read_lux(float *lux);

#endif  /* LIGHT_SENSOR_H_ */
