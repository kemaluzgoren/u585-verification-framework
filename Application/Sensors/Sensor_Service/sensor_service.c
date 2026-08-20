/**
 * @file    sensor_service.c
 * @brief
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-18
 *
 * SPDX-License-Identifier: MIT
 */

#include "sensor_service.h"

#include "light_sensor.h"
#include "humidity_sensor.h"
#include "pressure_sensor.h"
#include "magneto_sensor.h"
#include "imu_sensor.h"
#include "main.h"

#define SENSOR_SERVICE_READ_PERIOD_MS  (1000U)

static float ambient_light_lux = 0.0f;
static float temperature_hts221_degC = 0.0f;
static float humidity_rh = 0.0f;
static float pressure_hpa = 0.0f;
static float temperature_lps22hh_degC = 0.0f;
static int32_t magnetic_field_mx_mgauss = 0;
static int32_t magnetic_field_my_mgauss = 0;
static int32_t magnetic_field_mz_mgauss = 0;
static int32_t acceleration_ax_mg = 0;
static int32_t acceleration_ay_mg = 0;
static int32_t acceleration_az_mg = 0;
static int32_t angular_rate_gx_mdps = 0;
static int32_t angular_rate_gy_mdps = 0;
static int32_t angular_rate_gz_mdps = 0;
static uint32_t last_read_tick = 0;

void sensor_service_init(void) {

    last_read_tick = HAL_GetTick();
}


void sensor_service_run(void) {

    uint32_t now = HAL_GetTick();

    if ((now - last_read_tick) >= SENSOR_SERVICE_READ_PERIOD_MS) {
        light_sensor_read_lux(&ambient_light_lux);
        humidity_sensor_read(&temperature_hts221_degC, &humidity_rh);
        pressure_sensor_read(&pressure_hpa, &temperature_lps22hh_degC);
        magneto_sensor_read(&magnetic_field_mx_mgauss, &magnetic_field_my_mgauss, &magnetic_field_mz_mgauss);
        imu_sensor_read_accel(&acceleration_ax_mg, &acceleration_ay_mg, &acceleration_az_mg);
        imu_sensor_read_gyro(&angular_rate_gx_mdps, &angular_rate_gy_mdps, &angular_rate_gz_mdps);
        last_read_tick = now;
    }
}

float sensor_service_get_ambient_light(void) {

    return ambient_light_lux;
}

float sensor_service_get_humidity(void) {

    return humidity_rh;
}

float sensor_service_get_temperature_hts221(void) {

    return temperature_hts221_degC;
}

float sensor_service_get_pressure(void) {

    return pressure_hpa;
}

float sensor_service_get_temperature_lps22hh(void) {

    return temperature_lps22hh_degC;
}

void sensor_service_get_magnetic_field(int32_t *mx_mgauss, int32_t *my_mgauss, int32_t *mz_mgauss) {

    *mx_mgauss = magnetic_field_mx_mgauss;
    *my_mgauss = magnetic_field_my_mgauss;
    *mz_mgauss = magnetic_field_mz_mgauss;
}

void sensor_service_get_acceleration(int32_t *ax_mg, int32_t *ay_mg, int32_t *az_mg) {

    *ax_mg = acceleration_ax_mg;
    *ay_mg = acceleration_ay_mg;
    *az_mg = acceleration_az_mg;
}

void sensor_service_get_angular_rate(int32_t *gx_mdps, int32_t *gy_mdps, int32_t *gz_mdps) {

    *gx_mdps = angular_rate_gx_mdps;
    *gy_mdps = angular_rate_gy_mdps;
    *gz_mdps = angular_rate_gz_mdps;
}
