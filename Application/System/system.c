/**
 * @file    system.c
 * @brief   Application system interface.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-17
 *
 * SPDX-License-Identifier: MIT
 */

#include "system.h"
#include "i2c_bus.h"
#include "light_sensor.h"
#include "humidity_sensor.h"
#include "pressure_sensor.h"
#include "magneto_sensor.h"
#include "imu_sensor.h"
#include "sensor_service.h"

#include "main.h"

extern I2C_HandleTypeDef hi2c2;

/* All MEMS sensors on this board (light, humidity/temp, pressure/temp,
 * magneto, IMU) share this single physical I2C2 peripheral, so they are
 * handed the same I2C_Bus_t instance rather than each wrapping hi2c2
 * separately. That is what lets i2c_bus.c serialize their transfers with
 * one lock once RTOS support is added. */
static I2C_Bus_t hi2c2_bus;


void System_Init(void) {

    I2C_Bus_Init(&hi2c2_bus, &hi2c2);

    if (light_sensor_init(&hi2c2_bus) != 0) {
        Error_Handler();
    }

    if (humidity_sensor_init(&hi2c2_bus) != 0) {
        Error_Handler();
    }

    if (pressure_sensor_init(&hi2c2_bus) != 0) {
        Error_Handler();
    }

    if (magneto_sensor_init(&hi2c2_bus) != 0) {
        Error_Handler();
    }

    if (imu_sensor_init(&hi2c2_bus) != 0) {
        Error_Handler();
    }

    sensor_service_init();

}


void System_Run(void) {

    sensor_service_run();

}
