/**
 * @file    test_veml3235.c
 * @brief   Unit tests for veml3235.c, mocking veml3235_reg.h with CMock.
 *
 * Unlike test_veml3235_reg.c, this layer calls the free functions
 * veml3235_read_reg()/veml3235_write_reg() directly rather than through an
 * injectable struct, so CMock generates the fake in their place at link
 * time (see Mockveml3235_reg.c under the build directory).
 *
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#include "unity.h"
#include "veml3235.h"
#include "Mockveml3235_reg.h"

void setUp(void) { Mockveml3235_reg_Init(); }
void tearDown(void) { Mockveml3235_reg_Verify(); Mockveml3235_reg_Destroy(); }

void test_GetValues_reads_ALS_then_WHITE_and_fills_the_output_array(void)
{
    VEML3235_Object_t obj = {0};
    uint32_t values[VEML3235_MAX_CHANNELS] = {0};
    uint16_t als_raw = 1000;
    uint16_t white_raw = 500;

    veml3235_read_reg_ExpectAndReturn(&obj.Ctx, VEML3235_REG_ALS, NULL, 2, VEML3235_OK);
    veml3235_read_reg_IgnoreArg_pdata();
    veml3235_read_reg_ReturnThruPtr_pdata(&als_raw);

    veml3235_read_reg_ExpectAndReturn(&obj.Ctx, VEML3235_REG_WHITE, NULL, 2, VEML3235_OK);
    veml3235_read_reg_IgnoreArg_pdata();
    veml3235_read_reg_ReturnThruPtr_pdata(&white_raw);

    int32_t ret = VEML3235_GetValues(&obj, values);

    TEST_ASSERT_EQUAL_INT32(VEML3235_OK, ret);
    TEST_ASSERT_EQUAL_UINT32(1000, values[VEML3235_ALS_CHANNEL]);
    TEST_ASSERT_EQUAL_UINT32(500, values[VEML3235_WHITE_CHANNEL]);
}

void test_GetValues_returns_error_when_the_ALS_register_read_fails(void)
{
    VEML3235_Object_t obj = {0};
    uint32_t values[VEML3235_MAX_CHANNELS] = {0};

    veml3235_read_reg_ExpectAndReturn(&obj.Ctx, VEML3235_REG_ALS, NULL, 2, VEML3235_ERROR);
    veml3235_read_reg_IgnoreArg_pdata();

    int32_t ret = VEML3235_GetValues(&obj, values);

    TEST_ASSERT_EQUAL_INT32(VEML3235_ERROR, ret);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_GetValues_reads_ALS_then_WHITE_and_fills_the_output_array);
    RUN_TEST(test_GetValues_returns_error_when_the_ALS_register_read_fails);
    return UNITY_END();
}
