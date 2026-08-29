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

/* Renders the current sensor readings as a JSON object into dest and
 * reports its length via *len. timeout is unused - readings are always
 * "whatever sensor_service_run() last published", there is no "wait for
 * a new one" concept here (unlike Camera_Service_ProvideFrame()) - kept
 * only so this matches Application/Network/http_responses.h's
 * Http_Text_Provider_t shape for registration. Returns 0 (nothing
 * written) if dest_capacity is too small for the rendered JSON,
 * otherwise 1. */
int32_t Sensor_Service_ProvideJSON(char *dest, uint32_t dest_capacity, uint32_t *len, ULONG timeout);

#endif /* SENSOR_SERVICE_H_ */
