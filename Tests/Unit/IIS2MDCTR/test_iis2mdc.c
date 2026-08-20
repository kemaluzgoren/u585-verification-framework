/**
 * @file    test_iis2mdc.c
 * @brief   Unit tests for iis2mdc.c, mocking iis2mdc_reg.h with CMock.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#include "unity.h"
#include "iis2mdc.h"
#include "Mockiis2mdc_reg.h"

void setUp(void) { Mockiis2mdc_reg_Init(); }
void tearDown(void) { Mockiis2mdc_reg_Verify(); Mockiis2mdc_reg_Destroy(); }

void test_MAG_Enable_sets_continuous_mode_and_marks_enabled(void)
{
    IIS2MDC_Object_t obj = {0};
    obj.mag_is_enabled = 0;

    iis2mdc_operating_mode_set_ExpectAndReturn(&obj.Ctx, IIS2MDC_CONTINUOUS_MODE, IIS2MDC_OK);

    int32_t ret = IIS2MDC_MAG_Enable(&obj);

    TEST_ASSERT_EQUAL_INT32(IIS2MDC_OK, ret);
    TEST_ASSERT_EQUAL_UINT8(1, obj.mag_is_enabled);
}

void test_MAG_Enable_is_a_no_op_when_already_enabled(void)
{
    IIS2MDC_Object_t obj = {0};
    obj.mag_is_enabled = 1;

    int32_t ret = IIS2MDC_MAG_Enable(&obj);

    TEST_ASSERT_EQUAL_INT32(IIS2MDC_OK, ret);
}

void test_MAG_Disable_sets_power_down_and_marks_disabled(void)
{
    IIS2MDC_Object_t obj = {0};
    obj.mag_is_enabled = 1;

    iis2mdc_operating_mode_set_ExpectAndReturn(&obj.Ctx, IIS2MDC_POWER_DOWN, IIS2MDC_OK);

    int32_t ret = IIS2MDC_MAG_Disable(&obj);

    TEST_ASSERT_EQUAL_INT32(IIS2MDC_OK, ret);
    TEST_ASSERT_EQUAL_UINT8(0, obj.mag_is_enabled);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_MAG_Enable_sets_continuous_mode_and_marks_enabled);
    RUN_TEST(test_MAG_Enable_is_a_no_op_when_already_enabled);
    RUN_TEST(test_MAG_Disable_sets_power_down_and_marks_disabled);
    return UNITY_END();
}
