/**
 * @file    pressure_sensor.h
 * @brief
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PRESSURE_SENSOR_H_
#define PRESSURE_SENSOR_H_

#include <stdint.h>

#include "i2c_bus.h"

int32_t pressure_sensor_init(I2C_Bus_t *bus);
int32_t pressure_sensor_read(float *pressure_hpa, float *temperature_degC);

#endif  /* PRESSURE_SENSOR_H_ */
