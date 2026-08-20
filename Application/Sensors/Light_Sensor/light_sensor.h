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

#include "i2c_bus.h"

/* Selects which physical light sensor chip light_sensor.c talks to.
 * B-U585I-IOT02A boards carry VEML6030 only on Rev D; other revisions
 * (including this one) carry VEML3235 instead. */
#define LIGHT_SENSOR_VEML6030   0
#define LIGHT_SENSOR_VEML3235   1

#ifndef LIGHT_SENSOR_CHIP
#define LIGHT_SENSOR_CHIP  LIGHT_SENSOR_VEML3235
#endif

int32_t light_sensor_init(I2C_Bus_t *bus);
int32_t light_sensor_read_raw(uint16_t *als_raw, uint16_t *white_raw);
int32_t light_sensor_read_lux(float *lux);
int32_t light_sensor_debug_read_als_conf(uint16_t *als_conf);

#endif  /* LIGHT_SENSOR_H_ */
