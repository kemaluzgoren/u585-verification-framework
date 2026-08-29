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

#include <stdio.h>

#include "light_sensor.h"
#include "humidity_sensor.h"
#include "pressure_sensor.h"
#include "magneto_sensor.h"
#include "imu_sensor.h"

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


static TX_MUTEX sensor_service_mutex;

void sensor_service_init(void) {

    tx_mutex_create(&sensor_service_mutex, "Sensor Service Mutex", TX_INHERIT);
}

/* Reads every sensor once. Called periodically by sensor_service_task's
 * ThreadX thread, which owns the read period. Sensor reads happen outside
 * the mutex (I2C transfers are slow); only the publish step is locked. */
void sensor_service_run(void) {

    float local_ambient_light_lux;
    float local_temperature_hts221_degC;
    float local_humidity_rh;
    float local_pressure_hpa;
    float local_temperature_lps22hh_degC;
    int32_t local_mx_mgauss, local_my_mgauss, local_mz_mgauss;
    int32_t local_ax_mg, local_ay_mg, local_az_mg;
    int32_t local_gx_mdps, local_gy_mdps, local_gz_mdps;

    light_sensor_read_lux(&local_ambient_light_lux);
    humidity_sensor_read(&local_temperature_hts221_degC, &local_humidity_rh);
    pressure_sensor_read(&local_pressure_hpa, &local_temperature_lps22hh_degC);
    magneto_sensor_read(&local_mx_mgauss, &local_my_mgauss, &local_mz_mgauss);
    imu_sensor_read_accel(&local_ax_mg, &local_ay_mg, &local_az_mg);
    imu_sensor_read_gyro(&local_gx_mdps, &local_gy_mdps, &local_gz_mdps);

    tx_mutex_get(&sensor_service_mutex, TX_WAIT_FOREVER);

    ambient_light_lux = local_ambient_light_lux;
    temperature_hts221_degC = local_temperature_hts221_degC;
    humidity_rh = local_humidity_rh;
    pressure_hpa = local_pressure_hpa;
    temperature_lps22hh_degC = local_temperature_lps22hh_degC;
    magnetic_field_mx_mgauss = local_mx_mgauss;
    magnetic_field_my_mgauss = local_my_mgauss;
    magnetic_field_mz_mgauss = local_mz_mgauss;
    acceleration_ax_mg = local_ax_mg;
    acceleration_ay_mg = local_ay_mg;
    acceleration_az_mg = local_az_mg;
    angular_rate_gx_mdps = local_gx_mdps;
    angular_rate_gy_mdps = local_gy_mdps;
    angular_rate_gz_mdps = local_gz_mdps;

    tx_mutex_put(&sensor_service_mutex);
}

void sensor_service_task(ULONG thread_input) {

    (void)thread_input;

    while (1) {
        sensor_service_run();
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}

float sensor_service_get_ambient_light(void) {

    float value;

    tx_mutex_get(&sensor_service_mutex, TX_WAIT_FOREVER);
    value = ambient_light_lux;
    tx_mutex_put(&sensor_service_mutex);

    return value;
}

float sensor_service_get_humidity(void) {

    float value;

    tx_mutex_get(&sensor_service_mutex, TX_WAIT_FOREVER);
    value = humidity_rh;
    tx_mutex_put(&sensor_service_mutex);

    return value;
}

float sensor_service_get_temperature_hts221(void) {

    float value;

    tx_mutex_get(&sensor_service_mutex, TX_WAIT_FOREVER);
    value = temperature_hts221_degC;
    tx_mutex_put(&sensor_service_mutex);

    return value;
}

float sensor_service_get_pressure(void) {

    float value;

    tx_mutex_get(&sensor_service_mutex, TX_WAIT_FOREVER);
    value = pressure_hpa;
    tx_mutex_put(&sensor_service_mutex);

    return value;
}

float sensor_service_get_temperature_lps22hh(void) {

    float value;

    tx_mutex_get(&sensor_service_mutex, TX_WAIT_FOREVER);
    value = temperature_lps22hh_degC;
    tx_mutex_put(&sensor_service_mutex);

    return value;
}

void sensor_service_get_magnetic_field(int32_t *mx_mgauss, int32_t *my_mgauss, int32_t *mz_mgauss) {

    tx_mutex_get(&sensor_service_mutex, TX_WAIT_FOREVER);
    *mx_mgauss = magnetic_field_mx_mgauss;
    *my_mgauss = magnetic_field_my_mgauss;
    *mz_mgauss = magnetic_field_mz_mgauss;
    tx_mutex_put(&sensor_service_mutex);
}

void sensor_service_get_acceleration(int32_t *ax_mg, int32_t *ay_mg, int32_t *az_mg) {

    tx_mutex_get(&sensor_service_mutex, TX_WAIT_FOREVER);
    *ax_mg = acceleration_ax_mg;
    *ay_mg = acceleration_ay_mg;
    *az_mg = acceleration_az_mg;
    tx_mutex_put(&sensor_service_mutex);
}

void sensor_service_get_angular_rate(int32_t *gx_mdps, int32_t *gy_mdps, int32_t *gz_mdps) {

    tx_mutex_get(&sensor_service_mutex, TX_WAIT_FOREVER);
    *gx_mdps = angular_rate_gx_mdps;
    *gy_mdps = angular_rate_gy_mdps;
    *gz_mdps = angular_rate_gz_mdps;
    tx_mutex_put(&sensor_service_mutex);
}

/* This project links against nano.specs without -u _printf_float
 * (cmake/gcc-arm-none-eabi.cmake), so snprintf's "%f" silently does not
 * work - renders a float as fixed-point with 2 decimal digits using only
 * integer formatting, which nano newlib's snprintf does support. */
static int32_t format_fixed_2dp(char *dest, uint32_t dest_capacity, float value) {

    float abs_value = (value < 0.0f) ? -value : value;
    int32_t whole = (int32_t)abs_value;
    int32_t hundredths = (int32_t)((abs_value - (float)whole) * 100.0f + 0.5f);

    if (hundredths >= 100) {
        whole += 1;
        hundredths -= 100;
    }

    return snprintf(dest, dest_capacity, "%s%ld.%02ld", (value < 0.0f) ? "-" : "", (long)whole, (long)hundredths);
}

int32_t Sensor_Service_ProvideJSON(char *dest, uint32_t dest_capacity, uint32_t *len, ULONG timeout) {

    int32_t mx, my, mz, ax, ay, az, gx, gy, gz;
    int32_t written;
    uint32_t pos = 0;

    (void)timeout;

    sensor_service_get_magnetic_field(&mx, &my, &mz);
    sensor_service_get_acceleration(&ax, &ay, &az);
    sensor_service_get_angular_rate(&gx, &gy, &gz);

    written = snprintf(dest + pos, dest_capacity - pos, "{\"ambient_light_lux\":");
    if (written < 0 || (uint32_t)written >= dest_capacity - pos) return 0;
    pos += (uint32_t)written;

    written = format_fixed_2dp(dest + pos, dest_capacity - pos, sensor_service_get_ambient_light());
    if (written < 0 || (uint32_t)written >= dest_capacity - pos) return 0;
    pos += (uint32_t)written;

    written = snprintf(dest + pos, dest_capacity - pos, ",\"humidity_rh\":");
    if (written < 0 || (uint32_t)written >= dest_capacity - pos) return 0;
    pos += (uint32_t)written;

    written = format_fixed_2dp(dest + pos, dest_capacity - pos, sensor_service_get_humidity());
    if (written < 0 || (uint32_t)written >= dest_capacity - pos) return 0;
    pos += (uint32_t)written;

    written = snprintf(dest + pos, dest_capacity - pos, ",\"temperature_hts221_degC\":");
    if (written < 0 || (uint32_t)written >= dest_capacity - pos) return 0;
    pos += (uint32_t)written;

    written = format_fixed_2dp(dest + pos, dest_capacity - pos, sensor_service_get_temperature_hts221());
    if (written < 0 || (uint32_t)written >= dest_capacity - pos) return 0;
    pos += (uint32_t)written;

    written = snprintf(dest + pos, dest_capacity - pos, ",\"pressure_hpa\":");
    if (written < 0 || (uint32_t)written >= dest_capacity - pos) return 0;
    pos += (uint32_t)written;

    written = format_fixed_2dp(dest + pos, dest_capacity - pos, sensor_service_get_pressure());
    if (written < 0 || (uint32_t)written >= dest_capacity - pos) return 0;
    pos += (uint32_t)written;

    written = snprintf(dest + pos, dest_capacity - pos, ",\"temperature_lps22hh_degC\":");
    if (written < 0 || (uint32_t)written >= dest_capacity - pos) return 0;
    pos += (uint32_t)written;

    written = format_fixed_2dp(dest + pos, dest_capacity - pos, sensor_service_get_temperature_lps22hh());
    if (written < 0 || (uint32_t)written >= dest_capacity - pos) return 0;
    pos += (uint32_t)written;

    written = snprintf(dest + pos, dest_capacity - pos,
                        ",\"magnetic_field_mgauss\":{\"x\":%ld,\"y\":%ld,\"z\":%ld}"
                        ",\"acceleration_mg\":{\"x\":%ld,\"y\":%ld,\"z\":%ld}"
                        ",\"angular_rate_mdps\":{\"x\":%ld,\"y\":%ld,\"z\":%ld}}",
                        (long)mx, (long)my, (long)mz, (long)ax, (long)ay, (long)az, (long)gx, (long)gy, (long)gz);
    if (written < 0 || (uint32_t)written >= dest_capacity - pos) return 0;
    pos += (uint32_t)written;

    *len = pos;
    return 1;
}
