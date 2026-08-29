/**
 * @file    pressure_sensor.h
 * @brief   Minimal stand-in for Application/Sensors/Pressure_Sensor/pressure_sensor.h -
 *          see light_sensor.h (this directory) for why.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-29
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PRESSURE_SENSOR_H_
#define PRESSURE_SENSOR_H_

#include <stdint.h>

int32_t pressure_sensor_read(float *pressure_hpa, float *temperature_degC);

#endif /* PRESSURE_SENSOR_H_ */
