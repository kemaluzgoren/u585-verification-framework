/**
 * @file    magneto_sensor.h
 * @brief
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef MAGNETO_SENSOR_H_
#define MAGNETO_SENSOR_H_

#include <stdint.h>

#include "i2c_bus.h"

int32_t magneto_sensor_init(I2C_Bus_t *bus);
int32_t magneto_sensor_read(int32_t *mx_mgauss, int32_t *my_mgauss, int32_t *mz_mgauss);

#endif  /* MAGNETO_SENSOR_H_ */
