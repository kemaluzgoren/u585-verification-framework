/**
 * @file    test_sensor_service.c
 * @brief   Integration tests for Application/Sensors/Sensor_Service/sensor_service.c's
 *          Sensor_Service_ProvideJSON() - the seam between the service's
 *          own logic (JSON rendering, the format_fixed_2dp() fixed-point
 *          float formatter) and a real ThreadX mutex
 *          (sensor_service_mutex, created with TX_INHERIT), linked here
 *          against ThreadX's own real Middlewares/ST/threadx/common/src/tx_mutex_*.c
 *          source rather than a mock - see threadx_host_glue.c's header
 *          comment for how/why. The five sensor-read functions
 *          sensor_service_run() calls are mocked (via this directory's
 *          fake light_sensor.h/humidity_sensor.h/pressure_sensor.h/
 *          magneto_sensor.h/imu_sensor.h) so the JSON output can be
 *          checked against known input values.
 *
 *          format_fixed_2dp() exists because this project links against
 *          nano.specs without -u _printf_float (cmake/gcc-arm-none-eabi.cmake),
 *          so snprintf's "%f" silently does not work on the real target -
 *          the negative-value and rounding-carry cases below are exactly
 *          the kind of off-by-one/sign bug that formatter could hide.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-29
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

#include "unity.h"
#include "sensor_service.h"
#include "Mocklight_sensor.h"
#include "Mockhumidity_sensor.h"
#include "Mockpressure_sensor.h"
#include "Mockmagneto_sensor.h"
#include "Mockimu_sensor.h"

void setUp(void) {
    Mocklight_sensor_Init();
    Mockhumidity_sensor_Init();
    Mockpressure_sensor_Init();
    Mockmagneto_sensor_Init();
    Mockimu_sensor_Init();
}

void tearDown(void) {
    Mocklight_sensor_Verify();
    Mocklight_sensor_Destroy();
    Mockhumidity_sensor_Verify();
    Mockhumidity_sensor_Destroy();
    Mockpressure_sensor_Verify();
    Mockpressure_sensor_Destroy();
    Mockmagneto_sensor_Verify();
    Mockmagneto_sensor_Destroy();
    Mockimu_sensor_Verify();
    Mockimu_sensor_Destroy();
}

/* Sets up mock expectations for one sensor_service_run() call with the
 * given readings, then runs it - every test below starts from this so
 * Sensor_Service_ProvideJSON() has real, known, mutex-protected state to
 * render. */
static void run_with_readings(float lux, float temp_hts221, float humidity, float pressure, float temp_lps22hh,
                               int32_t mx, int32_t my, int32_t mz, int32_t ax, int32_t ay, int32_t az, int32_t gx,
                               int32_t gy, int32_t gz) {

    light_sensor_read_lux_ExpectAndReturn(NULL, 0);
    light_sensor_read_lux_IgnoreArg_lux();
    light_sensor_read_lux_ReturnThruPtr_lux(&lux);

    humidity_sensor_read_ExpectAndReturn(NULL, NULL, 0);
    humidity_sensor_read_IgnoreArg_temperature_degC();
    humidity_sensor_read_IgnoreArg_humidity_rh();
    humidity_sensor_read_ReturnThruPtr_temperature_degC(&temp_hts221);
    humidity_sensor_read_ReturnThruPtr_humidity_rh(&humidity);

    pressure_sensor_read_ExpectAndReturn(NULL, NULL, 0);
    pressure_sensor_read_IgnoreArg_pressure_hpa();
    pressure_sensor_read_IgnoreArg_temperature_degC();
    pressure_sensor_read_ReturnThruPtr_pressure_hpa(&pressure);
    pressure_sensor_read_ReturnThruPtr_temperature_degC(&temp_lps22hh);

    magneto_sensor_read_ExpectAndReturn(NULL, NULL, NULL, 0);
    magneto_sensor_read_IgnoreArg_mx_mgauss();
    magneto_sensor_read_IgnoreArg_my_mgauss();
    magneto_sensor_read_IgnoreArg_mz_mgauss();
    magneto_sensor_read_ReturnThruPtr_mx_mgauss(&mx);
    magneto_sensor_read_ReturnThruPtr_my_mgauss(&my);
    magneto_sensor_read_ReturnThruPtr_mz_mgauss(&mz);

    imu_sensor_read_accel_ExpectAndReturn(NULL, NULL, NULL, 0);
    imu_sensor_read_accel_IgnoreArg_ax_mg();
    imu_sensor_read_accel_IgnoreArg_ay_mg();
    imu_sensor_read_accel_IgnoreArg_az_mg();
    imu_sensor_read_accel_ReturnThruPtr_ax_mg(&ax);
    imu_sensor_read_accel_ReturnThruPtr_ay_mg(&ay);
    imu_sensor_read_accel_ReturnThruPtr_az_mg(&az);

    imu_sensor_read_gyro_ExpectAndReturn(NULL, NULL, NULL, 0);
    imu_sensor_read_gyro_IgnoreArg_gx_mdps();
    imu_sensor_read_gyro_IgnoreArg_gy_mdps();
    imu_sensor_read_gyro_IgnoreArg_gz_mdps();
    imu_sensor_read_gyro_ReturnThruPtr_gx_mdps(&gx);
    imu_sensor_read_gyro_ReturnThruPtr_gy_mdps(&gy);
    imu_sensor_read_gyro_ReturnThruPtr_gz_mdps(&gz);

    sensor_service_run();
}

void test_ProvideJSON_renders_every_field(void) {
    char buf[512];
    uint32_t len = 0;

    run_with_readings(300.0f, 23.4f, 45.6f, 1013.25f, 23.1f, 100, -200, 300, 10, -20, 980, 1, -2, 3);

    int32_t ret = Sensor_Service_ProvideJSON(buf, sizeof(buf), &len, TX_NO_WAIT);

    TEST_ASSERT_EQUAL_INT32(1, ret);
    buf[len] = '\0';
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"ambient_light_lux\":300.00"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"humidity_rh\":45.60"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"temperature_hts221_degC\":23.40"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"pressure_hpa\":1013.25"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"temperature_lps22hh_degC\":23.10"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"magnetic_field_mgauss\":{\"x\":100,\"y\":-200,\"z\":300}"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"acceleration_mg\":{\"x\":10,\"y\":-20,\"z\":980}"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"angular_rate_mdps\":{\"x\":1,\"y\":-2,\"z\":3}"));
}

/* format_fixed_2dp() (sensor_service.c, private) exists specifically
 * because this project's nano.specs build cannot use snprintf's "%f" -
 * negative floats are exactly the case a naive integer-truncation
 * fixed-point formatter gets wrong (e.g. truncating -5.5 towards zero
 * loses the sign on the fractional part). */
void test_ProvideJSON_formats_a_negative_temperature_correctly(void) {
    char buf[512];
    uint32_t len = 0;

    run_with_readings(0.0f, -5.5f, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    int32_t ret = Sensor_Service_ProvideJSON(buf, sizeof(buf), &len, TX_NO_WAIT);

    TEST_ASSERT_EQUAL_INT32(1, ret);
    buf[len] = '\0';
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"temperature_hts221_degC\":-5.50"));
}

/* 9.996 rounds to the *next whole number* at two decimal places (9.996 ->
 * "10.00", not "9.100" or "9.996 truncated to 9.99") - the "hundredths >=
 * 100" carry branch in format_fixed_2dp(). */
void test_ProvideJSON_rounds_a_hundredths_carry_into_the_whole_part(void) {
    char buf[512];
    uint32_t len = 0;

    run_with_readings(9.996f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    int32_t ret = Sensor_Service_ProvideJSON(buf, sizeof(buf), &len, TX_NO_WAIT);

    TEST_ASSERT_EQUAL_INT32(1, ret);
    buf[len] = '\0';
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"ambient_light_lux\":10.00"));
}

void test_ProvideJSON_fails_when_the_buffer_is_too_small(void) {
    char buf[4];
    uint32_t len = 999; /* sentinel - must stay untouched on failure */

    run_with_readings(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1, 1, 1, 1, 1, 1, 1, 1, 1);

    int32_t ret = Sensor_Service_ProvideJSON(buf, sizeof(buf), &len, TX_NO_WAIT);

    TEST_ASSERT_EQUAL_INT32(0, ret);
    TEST_ASSERT_EQUAL_UINT32(999, len);
}

int main(void) {
    /* sensor_service_mutex (Application/Sensors/Sensor_Service/sensor_service.c)
     * is one static TX_MUTEX for the whole process, and ThreadX's real
     * tx_mutex_create() (linked here for real) does not support being
     * called twice on the same control block without an intervening
     * tx_mutex_delete() in between - it corrupts the created-object list
     * bookkeeping (confirmed as a real segfault while writing this test:
     * the second create() zeroes the mutex's own linked-list fields via
     * memset() while the global "created" list still points at it as
     * already-linked). sensor_service_init() must run exactly once here,
     * not per-test in setUp() - matching how it is genuinely only ever
     * called once in the real firmware too (Application/System/system.c's
     * System_Start()). */
    sensor_service_init();

    UNITY_BEGIN();

    RUN_TEST(test_ProvideJSON_renders_every_field);
    RUN_TEST(test_ProvideJSON_formats_a_negative_temperature_correctly);
    RUN_TEST(test_ProvideJSON_rounds_a_hundredths_carry_into_the_whole_part);
    RUN_TEST(test_ProvideJSON_fails_when_the_buffer_is_too_small);

    return UNITY_END();
}
