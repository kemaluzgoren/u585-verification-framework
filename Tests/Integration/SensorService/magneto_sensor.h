/**
 * @file    magneto_sensor.h
 * @brief   Minimal stand-in for Application/Sensors/Magneto_Sensor/magneto_sensor.h -
 *          see light_sensor.h (this directory) for why.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-29
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef MAGNETO_SENSOR_H_
#define MAGNETO_SENSOR_H_

#include <stdint.h>

int32_t magneto_sensor_read(int32_t *mx_mgauss, int32_t *my_mgauss, int32_t *mz_mgauss);

#endif /* MAGNETO_SENSOR_H_ */
