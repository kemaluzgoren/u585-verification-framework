/**
 * @file    test_veml6030.c
 * @brief   Unit tests for veml6030.c, mocking veml6030_reg.h with CMock.
 *
 * Unlike test_veml6030_reg.c, this layer calls the free functions
 * veml6030_read_reg()/veml6030_write_reg() directly rather than through an
 * injectable struct, so CMock generates the fake in their place at link
 * time (see Mockveml6030_reg.c under the build directory).
 *
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-17
 *
 * SPDX-License-Identifier: MIT
 */

#include "unity.h"
#include "veml6030.h"
#include "Mockveml6030_reg.h"

void setUp(void) { Mockveml6030_reg_Init(); }
void tearDown(void) { Mockveml6030_reg_Verify(); Mockveml6030_reg_Destroy(); }

void test_GetValues_reads_ALS_then_WHITE_and_fills_the_output_array(void)
{
    VEML6030_Object_t obj = {0};
    uint32_t values[VEML6030_MAX_CHANNELS] = {0};
    uint16_t als_raw = 1000;
    uint16_t white_raw = 500;

    veml6030_read_reg_ExpectAndReturn(&obj.Ctx, VEML6030_REG_ALS, NULL, 2, VEML6030_OK);
    veml6030_read_reg_IgnoreArg_pdata();
    veml6030_read_reg_ReturnThruPtr_pdata(&als_raw);

    veml6030_read_reg_ExpectAndReturn(&obj.Ctx, VEML6030_REG_WHITE, NULL, 2, VEML6030_OK);
    veml6030_read_reg_IgnoreArg_pdata();
    veml6030_read_reg_ReturnThruPtr_pdata(&white_raw);

    int32_t ret = VEML6030_GetValues(&obj, values);

    TEST_ASSERT_EQUAL_INT32(VEML6030_OK, ret);
    TEST_ASSERT_EQUAL_UINT32(1000, values[VEML6030_ALS_CHANNEL]);
    TEST_ASSERT_EQUAL_UINT32(500, values[VEML6030_WHITE_CHANNEL]);
}

void test_GetValues_returns_error_when_the_ALS_register_read_fails(void)
{
    VEML6030_Object_t obj = {0};
    uint32_t values[VEML6030_MAX_CHANNELS] = {0};

    veml6030_read_reg_ExpectAndReturn(&obj.Ctx, VEML6030_REG_ALS, NULL, 2, VEML6030_ERROR);
    veml6030_read_reg_IgnoreArg_pdata();

    int32_t ret = VEML6030_GetValues(&obj, values);

    TEST_ASSERT_EQUAL_INT32(VEML6030_ERROR, ret);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_GetValues_reads_ALS_then_WHITE_and_fills_the_output_array);
    RUN_TEST(test_GetValues_returns_error_when_the_ALS_register_read_fails);
    return UNITY_END();
}
