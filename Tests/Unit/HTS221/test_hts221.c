/**
 * @file    test_hts221.c
 * @brief   Unit tests for hts221.c, mocking hts221_reg.h with CMock.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#include "unity.h"
#include "hts221.h"
#include "Mockhts221_reg.h"

void setUp(void) { Mockhts221_reg_Init(); }
void tearDown(void) { Mockhts221_reg_Verify(); Mockhts221_reg_Destroy(); }

void test_HUM_GetHumidity_interpolates_between_calibration_points(void)
{
    HTS221_Object_t obj = {0};
    float humidity = 0.0f;
    float x0 = 0.0f;
    float y0 = 20.0f;
    float x1 = 1000.0f;
    float y1 = 80.0f;
    int16_t raw = 500;

    hts221_hum_adc_point_0_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_hum_adc_point_0_get_IgnoreArg_val();
    hts221_hum_adc_point_0_get_ReturnThruPtr_val(&x0);

    hts221_hum_rh_point_0_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_hum_rh_point_0_get_IgnoreArg_val();
    hts221_hum_rh_point_0_get_ReturnThruPtr_val(&y0);

    hts221_hum_adc_point_1_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_hum_adc_point_1_get_IgnoreArg_val();
    hts221_hum_adc_point_1_get_ReturnThruPtr_val(&x1);

    hts221_hum_rh_point_1_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_hum_rh_point_1_get_IgnoreArg_val();
    hts221_hum_rh_point_1_get_ReturnThruPtr_val(&y1);

    hts221_humidity_raw_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_humidity_raw_get_IgnoreArg_val();
    hts221_humidity_raw_get_ReturnThruPtr_val(&raw);

    int32_t ret = HTS221_HUM_GetHumidity(&obj, &humidity);

    TEST_ASSERT_EQUAL_INT32(HTS221_OK, ret);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, humidity);
}

void test_HUM_GetHumidity_clamps_to_100_percent(void)
{
    HTS221_Object_t obj = {0};
    float humidity = 0.0f;
    float x0 = 0.0f;
    float y0 = 0.0f;
    float x1 = 100.0f;
    float y1 = 100.0f;
    int16_t raw = 500; /* beyond the calibrated range -> interpolates past 100% */

    hts221_hum_adc_point_0_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_hum_adc_point_0_get_IgnoreArg_val();
    hts221_hum_adc_point_0_get_ReturnThruPtr_val(&x0);

    hts221_hum_rh_point_0_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_hum_rh_point_0_get_IgnoreArg_val();
    hts221_hum_rh_point_0_get_ReturnThruPtr_val(&y0);

    hts221_hum_adc_point_1_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_hum_adc_point_1_get_IgnoreArg_val();
    hts221_hum_adc_point_1_get_ReturnThruPtr_val(&x1);

    hts221_hum_rh_point_1_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_hum_rh_point_1_get_IgnoreArg_val();
    hts221_hum_rh_point_1_get_ReturnThruPtr_val(&y1);

    hts221_humidity_raw_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_humidity_raw_get_IgnoreArg_val();
    hts221_humidity_raw_get_ReturnThruPtr_val(&raw);

    int32_t ret = HTS221_HUM_GetHumidity(&obj, &humidity);

    TEST_ASSERT_EQUAL_INT32(HTS221_OK, ret);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, humidity);
}

void test_HUM_GetHumidity_returns_error_when_the_first_calibration_read_fails(void)
{
    HTS221_Object_t obj = {0};
    float humidity = 0.0f;

    hts221_hum_adc_point_0_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_ERROR);
    hts221_hum_adc_point_0_get_IgnoreArg_val();

    int32_t ret = HTS221_HUM_GetHumidity(&obj, &humidity);

    TEST_ASSERT_EQUAL_INT32(HTS221_ERROR, ret);
}

void test_TEMP_GetTemperature_interpolates_between_calibration_points(void)
{
    HTS221_Object_t obj = {0};
    float temperature = 0.0f;
    float x0 = 0.0f;
    float y0 = 15.0f;
    float x1 = 1000.0f;
    float y1 = 35.0f;
    int16_t raw = 500;

    hts221_temp_adc_point_0_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_temp_adc_point_0_get_IgnoreArg_val();
    hts221_temp_adc_point_0_get_ReturnThruPtr_val(&x0);

    hts221_temp_deg_point_0_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_temp_deg_point_0_get_IgnoreArg_val();
    hts221_temp_deg_point_0_get_ReturnThruPtr_val(&y0);

    hts221_temp_adc_point_1_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_temp_adc_point_1_get_IgnoreArg_val();
    hts221_temp_adc_point_1_get_ReturnThruPtr_val(&x1);

    hts221_temp_deg_point_1_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_temp_deg_point_1_get_IgnoreArg_val();
    hts221_temp_deg_point_1_get_ReturnThruPtr_val(&y1);

    hts221_temperature_raw_get_ExpectAndReturn(&obj.Ctx, NULL, HTS221_OK);
    hts221_temperature_raw_get_IgnoreArg_val();
    hts221_temperature_raw_get_ReturnThruPtr_val(&raw);

    int32_t ret = HTS221_TEMP_GetTemperature(&obj, &temperature);

    TEST_ASSERT_EQUAL_INT32(HTS221_OK, ret);
    TEST_ASSERT_EQUAL_FLOAT(25.0f, temperature);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_HUM_GetHumidity_interpolates_between_calibration_points);
    RUN_TEST(test_HUM_GetHumidity_clamps_to_100_percent);
    RUN_TEST(test_HUM_GetHumidity_returns_error_when_the_first_calibration_read_fails);
    RUN_TEST(test_TEMP_GetTemperature_interpolates_between_calibration_points);
    return UNITY_END();
}
