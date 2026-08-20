/**
 * @file    test_ism330dhcx.c
 * @brief   Unit tests for ism330dhcx.c, mocking ism330dhcx_reg.h with CMock.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-19
 *
 * SPDX-License-Identifier: MIT
 */

#include "unity.h"
#include "ism330dhcx.h"
#include "Mockism330dhcx_reg.h"

void setUp(void) { Mockism330dhcx_reg_Init(); }
void tearDown(void) { Mockism330dhcx_reg_Verify(); Mockism330dhcx_reg_Destroy(); }

void test_ACC_GetSensitivity_maps_2g_full_scale_to_the_datasheet_constant(void)
{
    ISM330DHCX_Object_t obj = {0};
    float sensitivity = 0.0f;
    ism330dhcx_fs_xl_t full_scale = ISM330DHCX_2g;

    ism330dhcx_xl_full_scale_get_ExpectAndReturn(&obj.Ctx, NULL, ISM330DHCX_OK);
    ism330dhcx_xl_full_scale_get_IgnoreArg_val();
    ism330dhcx_xl_full_scale_get_ReturnThruPtr_val(&full_scale);

    int32_t ret = ISM330DHCX_ACC_GetSensitivity(&obj, &sensitivity);

    TEST_ASSERT_EQUAL_INT32(ISM330DHCX_OK, ret);
    TEST_ASSERT_EQUAL_FLOAT(ISM330DHCX_ACC_SENSITIVITY_FS_2G, sensitivity);
}

void test_ACC_GetSensitivity_maps_16g_full_scale_to_the_datasheet_constant(void)
{
    ISM330DHCX_Object_t obj = {0};
    float sensitivity = 0.0f;
    ism330dhcx_fs_xl_t full_scale = ISM330DHCX_16g;

    ism330dhcx_xl_full_scale_get_ExpectAndReturn(&obj.Ctx, NULL, ISM330DHCX_OK);
    ism330dhcx_xl_full_scale_get_IgnoreArg_val();
    ism330dhcx_xl_full_scale_get_ReturnThruPtr_val(&full_scale);

    int32_t ret = ISM330DHCX_ACC_GetSensitivity(&obj, &sensitivity);

    TEST_ASSERT_EQUAL_INT32(ISM330DHCX_OK, ret);
    TEST_ASSERT_EQUAL_FLOAT(ISM330DHCX_ACC_SENSITIVITY_FS_16G, sensitivity);
}

void test_GYRO_GetSensitivity_maps_2000dps_full_scale_to_the_datasheet_constant(void)
{
    ISM330DHCX_Object_t obj = {0};
    float sensitivity = 0.0f;
    ism330dhcx_fs_g_t full_scale = ISM330DHCX_2000dps;

    ism330dhcx_gy_full_scale_get_ExpectAndReturn(&obj.Ctx, NULL, ISM330DHCX_OK);
    ism330dhcx_gy_full_scale_get_IgnoreArg_val();
    ism330dhcx_gy_full_scale_get_ReturnThruPtr_val(&full_scale);

    int32_t ret = ISM330DHCX_GYRO_GetSensitivity(&obj, &sensitivity);

    TEST_ASSERT_EQUAL_INT32(ISM330DHCX_OK, ret);
    TEST_ASSERT_EQUAL_FLOAT(ISM330DHCX_GYRO_SENSITIVITY_FS_2000DPS, sensitivity);
}

void test_ACC_Enable_applies_the_stored_output_data_rate_and_marks_enabled(void)
{
    ISM330DHCX_Object_t obj = {0};
    obj.acc_is_enabled = 0;
    obj.acc_odr = ISM330DHCX_XL_ODR_104Hz;

    ism330dhcx_xl_data_rate_set_ExpectAndReturn(&obj.Ctx, ISM330DHCX_XL_ODR_104Hz, ISM330DHCX_OK);

    int32_t ret = ISM330DHCX_ACC_Enable(&obj);

    TEST_ASSERT_EQUAL_INT32(ISM330DHCX_OK, ret);
    TEST_ASSERT_EQUAL_UINT8(1, obj.acc_is_enabled);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ACC_GetSensitivity_maps_2g_full_scale_to_the_datasheet_constant);
    RUN_TEST(test_ACC_GetSensitivity_maps_16g_full_scale_to_the_datasheet_constant);
    RUN_TEST(test_GYRO_GetSensitivity_maps_2000dps_full_scale_to_the_datasheet_constant);
    RUN_TEST(test_ACC_Enable_applies_the_stored_output_data_rate_and_marks_enabled);
    return UNITY_END();
}
