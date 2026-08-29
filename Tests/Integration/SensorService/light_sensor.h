/**
 * @file    light_sensor.h
 * @brief   Minimal stand-in for Application/Sensors/Light_Sensor/light_sensor.h -
 *          only the one function sensor_service.c actually calls, so this
 *          test never needs the real header's i2c_bus.h -> stm32u5xx_hal.h
 *          chain (ARM-target only, not host-compilable).
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-29
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIGHT_SENSOR_H_
#define LIGHT_SENSOR_H_

#include <stdint.h>

int32_t light_sensor_read_lux(float *lux);

#endif /* LIGHT_SENSOR_H_ */
