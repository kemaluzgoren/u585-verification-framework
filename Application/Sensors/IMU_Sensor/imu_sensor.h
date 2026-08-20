/**
 * @file    imu_sensor.h
 * @brief
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef IMU_SENSOR_H_
#define IMU_SENSOR_H_

#include <stdint.h>

#include "i2c_bus.h"

int32_t imu_sensor_init(I2C_Bus_t *bus);
int32_t imu_sensor_read_accel(int32_t *ax_mg, int32_t *ay_mg, int32_t *az_mg);
int32_t imu_sensor_read_gyro(int32_t *gx_mdps, int32_t *gy_mdps, int32_t *gz_mdps);

#endif  /* IMU_SENSOR_H_ */
