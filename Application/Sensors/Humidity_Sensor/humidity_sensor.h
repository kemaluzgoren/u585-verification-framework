/**
 * @file    humidity_sensor.h
 * @brief
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef HUMIDITY_SENSOR_H_
#define HUMIDITY_SENSOR_H_

#include <stdint.h>

#include "i2c_bus.h"

int32_t humidity_sensor_init(I2C_Bus_t *bus);
int32_t humidity_sensor_read(float *temperature_degC, float *humidity_rh);

#endif  /* HUMIDITY_SENSOR_H_ */
