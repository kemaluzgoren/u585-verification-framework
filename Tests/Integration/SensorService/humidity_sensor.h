/**
 * @file    humidity_sensor.h
 * @brief   Minimal stand-in for Application/Sensors/Humidity_Sensor/humidity_sensor.h -
 *          see light_sensor.h (this directory) for why.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-29
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef HUMIDITY_SENSOR_H_
#define HUMIDITY_SENSOR_H_

#include <stdint.h>

int32_t humidity_sensor_read(float *temperature_degC, float *humidity_rh);

#endif /* HUMIDITY_SENSOR_H_ */
