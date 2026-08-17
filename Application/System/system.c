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
#include "light_sensor.h"

#include "main.h"

extern I2C_HandleTypeDef hi2c2;


void System_Init(void) {

    if (light_sensor_init(&hi2c2) != 0) {
        Error_Handler();
    }

}


void System_Run(void) {



}
