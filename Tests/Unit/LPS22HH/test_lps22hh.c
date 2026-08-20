/**
 * @file    test_lps22hh.c
 * @brief   Unit tests for lps22hh.c, mocking lps22hh_reg.h with CMock.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#include "unity.h"
#include "lps22hh.h"
#include "Mocklps22hh_reg.h"

void setUp(void) { Mocklps22hh_reg_Init(); }
void tearDown(void) { Mocklps22hh_reg_Verify(); Mocklps22hh_reg_Destroy(); }

void test_PRESS_GetPressure_converts_raw_lsb_to_hpa(void)
{
    LPS22HH_Object_t obj = {0};
    float_t pressure = 0.0f;
    uint32_t raw = 1048576U; /* 1048576 / 1048576.0f == 1.0 hPa */

    lps22hh_pressure_raw_get_ExpectAndReturn(&obj.Ctx, NULL, LPS22HH_OK);
    lps22hh_pressure_raw_get_IgnoreArg_buff();
    lps22hh_pressure_raw_get_ReturnThruPtr_buff(&raw);
    lps22hh_from_lsb_to_hpa_ExpectAndReturn(raw, 1.0f);

    int32_t ret = LPS22HH_PRESS_GetPressure(&obj, &pressure);

    TEST_ASSERT_EQUAL_INT32(LPS22HH_OK, ret);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, pressure);
}

void test_PRESS_GetPressure_returns_error_when_the_raw_read_fails(void)
{
    LPS22HH_Object_t obj = {0};
    float_t pressure = 0.0f;

    lps22hh_pressure_raw_get_ExpectAndReturn(&obj.Ctx, NULL, LPS22HH_ERROR);
    lps22hh_pressure_raw_get_IgnoreArg_buff();

    int32_t ret = LPS22HH_PRESS_GetPressure(&obj, &pressure);

    TEST_ASSERT_EQUAL_INT32(LPS22HH_ERROR, ret);
}

void test_TEMP_GetTemperature_converts_raw_lsb_to_celsius(void)
{
    LPS22HH_Object_t obj = {0};
    float_t temperature = 0.0f;
    int16_t raw = 2550; /* 2550 / 100.0f == 25.5 degC */

    lps22hh_temperature_raw_get_ExpectAndReturn(&obj.Ctx, NULL, LPS22HH_OK);
    lps22hh_temperature_raw_get_IgnoreArg_buff();
    lps22hh_temperature_raw_get_ReturnThruPtr_buff(&raw);
    lps22hh_from_lsb_to_celsius_ExpectAndReturn(raw, 25.5f);

    int32_t ret = LPS22HH_TEMP_GetTemperature(&obj, &temperature);

    TEST_ASSERT_EQUAL_INT32(LPS22HH_OK, ret);
    TEST_ASSERT_EQUAL_FLOAT(25.5f, temperature);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_PRESS_GetPressure_converts_raw_lsb_to_hpa);
    RUN_TEST(test_PRESS_GetPressure_returns_error_when_the_raw_read_fails);
    RUN_TEST(test_TEMP_GetTemperature_converts_raw_lsb_to_celsius);
    return UNITY_END();
}
