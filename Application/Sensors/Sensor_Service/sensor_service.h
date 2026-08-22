/**
 * @file    sensor_service.h
 * @brief
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-18
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SENSOR_SERVICE_H_
#define SENSOR_SERVICE_H_

#include <stdint.h>
#include "tx_api.h"

void sensor_service_init(void);
void sensor_service_run(void);

/* ThreadX thread entry: periodically calls sensor_service_run(). */
void sensor_service_task(ULONG thread_input);

float sensor_service_get_ambient_light(void);
float sensor_service_get_humidity(void);
float sensor_service_get_temperature_hts221(void);
float sensor_service_get_pressure(void);
float sensor_service_get_temperature_lps22hh(void);
void sensor_service_get_magnetic_field(int32_t *mx_mgauss, int32_t *my_mgauss, int32_t *mz_mgauss);
void sensor_service_get_acceleration(int32_t *ax_mg, int32_t *ay_mg, int32_t *az_mg);
void sensor_service_get_angular_rate(int32_t *gx_mdps, int32_t *gy_mdps, int32_t *gz_mdps);

#endif /* SENSOR_SERVICE_H_ */
