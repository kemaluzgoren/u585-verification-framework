/**
 * @file    test_ov5640.c
 * @brief   Unit tests for ov5640.c, mocking ov5640_reg.h with CMock.
 *
 *          Focuses on the input validation and bit-packing/decoding logic
 *          (SetPolarities/GetPolarities, GetResolution, GetCapabilities,
 *          the Init/SetResolution guard clauses) rather than the giant
 *          per-resolution/per-pixel-format register tables, which are
 *          data, not logic.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#include "unity.h"
#include "ov5640.h"
#include "Mockov5640_reg.h"

void setUp(void) { Mockov5640_reg_Init(); }
void tearDown(void) { Mockov5640_reg_Verify(); Mockov5640_reg_Destroy(); }

/* ---- OV5640_SetPolarities ------------------------------------------- */

void test_SetPolarities_writes_the_control_register_on_valid_input(void)
{
    OV5640_Object_t obj = {0};

    ov5640_write_reg_ExpectAndReturn(&obj.Ctx, OV5640_POLARITY_CTRL, NULL, 1, OV5640_OK);
    ov5640_write_reg_IgnoreArg_pdata();

    int32_t ret = OV5640_SetPolarities(&obj, OV5640_POLARITY_PCLK_HIGH, OV5640_POLARITY_HREF_HIGH,
                                        OV5640_POLARITY_VSYNC_LOW);

    TEST_ASSERT_EQUAL_INT32(OV5640_OK, ret);
}

void test_SetPolarities_rejects_a_null_object_without_touching_the_register(void)
{
    int32_t ret = OV5640_SetPolarities(NULL, OV5640_POLARITY_PCLK_HIGH, OV5640_POLARITY_HREF_HIGH,
                                        OV5640_POLARITY_VSYNC_LOW);

    TEST_ASSERT_EQUAL_INT32(OV5640_ERROR, ret);
}

void test_SetPolarities_rejects_an_invalid_pclk_polarity_without_touching_the_register(void)
{
    OV5640_Object_t obj = {0};

    int32_t ret = OV5640_SetPolarities(&obj, 2U /* neither LOW nor HIGH */, OV5640_POLARITY_HREF_HIGH,
                                        OV5640_POLARITY_VSYNC_LOW);

    TEST_ASSERT_EQUAL_INT32(OV5640_ERROR, ret);
}

void test_SetPolarities_propagates_a_register_write_failure(void)
{
    OV5640_Object_t obj = {0};

    ov5640_write_reg_ExpectAndReturn(&obj.Ctx, OV5640_POLARITY_CTRL, NULL, 1, OV5640_ERROR);
    ov5640_write_reg_IgnoreArg_pdata();

    int32_t ret = OV5640_SetPolarities(&obj, OV5640_POLARITY_PCLK_LOW, OV5640_POLARITY_HREF_LOW,
                                        OV5640_POLARITY_VSYNC_HIGH);

    TEST_ASSERT_EQUAL_INT32(OV5640_ERROR, ret);
}

/* ---- OV5640_GetPolarities --------------------------------------------
 * OV5640_SetPolarities packs (Pclk << 5) | (Href << 1) | Vsync; these
 * cases pin down that GetPolarities decodes the same convention back. */

void test_GetPolarities_unpacks_all_bits_high(void)
{
    OV5640_Object_t obj = {0};
    uint8_t raw = (1U << 5) | (1U << 1) | 1U;
    uint32_t pclk = 0;
    uint32_t href = 0;
    uint32_t vsync = 0;

    ov5640_read_reg_ExpectAndReturn(&obj.Ctx, OV5640_POLARITY_CTRL, NULL, 1, OV5640_OK);
    ov5640_read_reg_IgnoreArg_pdata();
    ov5640_read_reg_ReturnThruPtr_pdata(&raw);

    int32_t ret = OV5640_GetPolarities(&obj, &pclk, &href, &vsync);

    TEST_ASSERT_EQUAL_INT32(OV5640_OK, ret);
    TEST_ASSERT_EQUAL_UINT32(1U, pclk);
    TEST_ASSERT_EQUAL_UINT32(1U, href);
    TEST_ASSERT_EQUAL_UINT32(1U, vsync);
}

void test_GetPolarities_unpacks_all_bits_low(void)
{
    OV5640_Object_t obj = {0};
    uint8_t raw = 0x00U;
    uint32_t pclk = 1;
    uint32_t href = 1;
    uint32_t vsync = 1;

    ov5640_read_reg_ExpectAndReturn(&obj.Ctx, OV5640_POLARITY_CTRL, NULL, 1, OV5640_OK);
    ov5640_read_reg_IgnoreArg_pdata();
    ov5640_read_reg_ReturnThruPtr_pdata(&raw);

    int32_t ret = OV5640_GetPolarities(&obj, &pclk, &href, &vsync);

    TEST_ASSERT_EQUAL_INT32(OV5640_OK, ret);
    TEST_ASSERT_EQUAL_UINT32(0U, pclk);
    TEST_ASSERT_EQUAL_UINT32(0U, href);
    TEST_ASSERT_EQUAL_UINT32(0U, vsync);
}

void test_GetPolarities_rejects_null_output_pointers_without_touching_the_register(void)
{
    OV5640_Object_t obj = {0};
    uint32_t pclk = 0;

    int32_t ret = OV5640_GetPolarities(&obj, &pclk, NULL, NULL);

    TEST_ASSERT_EQUAL_INT32(OV5640_ERROR, ret);
}

/* ---- OV5640_GetResolution --------------------------------------------
 * Reads DVPHO/DVPVO (x/y size) and maps the pair back to an OV5640_Rxxx
 * enum value. */

static void expect_resolution_read(OV5640_Object_t *obj, uint16_t x_size, uint16_t y_size, uint8_t *hi_bytes) {

    /* hi_bytes must stay alive for the whole test: ReturnThruPtr just
     * stores the pointer, the copy happens when the mock is invoked. */
    hi_bytes[0] = (uint8_t)(x_size >> 8);
    hi_bytes[1] = (uint8_t)(x_size & 0xFFU);
    hi_bytes[2] = (uint8_t)(y_size >> 8);
    hi_bytes[3] = (uint8_t)(y_size & 0xFFU);

    ov5640_read_reg_ExpectAndReturn(&obj->Ctx, OV5640_TIMING_DVPHO_HIGH, NULL, 1, OV5640_OK);
    ov5640_read_reg_IgnoreArg_pdata();
    ov5640_read_reg_ReturnThruPtr_pdata(&hi_bytes[0]);

    ov5640_read_reg_ExpectAndReturn(&obj->Ctx, OV5640_TIMING_DVPHO_LOW, NULL, 1, OV5640_OK);
    ov5640_read_reg_IgnoreArg_pdata();
    ov5640_read_reg_ReturnThruPtr_pdata(&hi_bytes[1]);

    ov5640_read_reg_ExpectAndReturn(&obj->Ctx, OV5640_TIMING_DVPVO_HIGH, NULL, 1, OV5640_OK);
    ov5640_read_reg_IgnoreArg_pdata();
    ov5640_read_reg_ReturnThruPtr_pdata(&hi_bytes[2]);

    ov5640_read_reg_ExpectAndReturn(&obj->Ctx, OV5640_TIMING_DVPVO_LOW, NULL, 1, OV5640_OK);
    ov5640_read_reg_IgnoreArg_pdata();
    ov5640_read_reg_ReturnThruPtr_pdata(&hi_bytes[3]);
}

void test_GetResolution_maps_640x480_to_R640x480(void)
{
    OV5640_Object_t obj = {0};
    uint8_t bytes[4];
    uint32_t resolution = 0xFFFFFFFFU;

    expect_resolution_read(&obj, 640U, 480U, bytes);

    int32_t ret = OV5640_GetResolution(&obj, &resolution);

    TEST_ASSERT_EQUAL_INT32(OV5640_OK, ret);
    TEST_ASSERT_EQUAL_UINT32(OV5640_R640x480, resolution);
}

void test_GetResolution_maps_800x480_to_R800x480(void)
{
    OV5640_Object_t obj = {0};
    uint8_t bytes[4];
    uint32_t resolution = 0xFFFFFFFFU;

    expect_resolution_read(&obj, 800U, 480U, bytes);

    int32_t ret = OV5640_GetResolution(&obj, &resolution);

    TEST_ASSERT_EQUAL_INT32(OV5640_OK, ret);
    TEST_ASSERT_EQUAL_UINT32(OV5640_R800x480, resolution);
}

void test_GetResolution_maps_160x120_to_R160x120(void)
{
    OV5640_Object_t obj = {0};
    uint8_t bytes[4];
    uint32_t resolution = 0xFFFFFFFFU;

    expect_resolution_read(&obj, 160U, 120U, bytes);

    int32_t ret = OV5640_GetResolution(&obj, &resolution);

    TEST_ASSERT_EQUAL_INT32(OV5640_OK, ret);
    TEST_ASSERT_EQUAL_UINT32(OV5640_R160x120, resolution);
}

void test_GetResolution_returns_error_for_an_unrecognized_size(void)
{
    OV5640_Object_t obj = {0};
    uint8_t bytes[4];
    uint32_t resolution = 0;

    expect_resolution_read(&obj, 1920U, 1080U, bytes);

    int32_t ret = OV5640_GetResolution(&obj, &resolution);

    TEST_ASSERT_EQUAL_INT32(OV5640_ERROR, ret);
}

/* ---- OV5640_GetCapabilities -------------------------------------------- */

void test_GetCapabilities_rejects_a_null_object(void)
{
    int32_t ret = OV5640_GetCapabilities(NULL, NULL);

    TEST_ASSERT_EQUAL_INT32(OV5640_ERROR, ret);
}

void test_GetCapabilities_reports_every_feature_supported(void)
{
    OV5640_Object_t obj = {0};
    OV5640_Capabilities_t caps = {0};

    int32_t ret = OV5640_GetCapabilities(&obj, &caps);

    TEST_ASSERT_EQUAL_INT32(OV5640_OK, ret);
    TEST_ASSERT_EQUAL_UINT32(1U, caps.Config_Resolution);
    TEST_ASSERT_EQUAL_UINT32(1U, caps.Config_LightMode);
    TEST_ASSERT_EQUAL_UINT32(1U, caps.Config_SpecialEffect);
    TEST_ASSERT_EQUAL_UINT32(1U, caps.Config_Brightness);
    TEST_ASSERT_EQUAL_UINT32(1U, caps.Config_Saturation);
    TEST_ASSERT_EQUAL_UINT32(1U, caps.Config_Contrast);
    TEST_ASSERT_EQUAL_UINT32(1U, caps.Config_HueDegree);
    TEST_ASSERT_EQUAL_UINT32(1U, caps.Config_MirrorFlip);
    TEST_ASSERT_EQUAL_UINT32(1U, caps.Config_Zoom);
    TEST_ASSERT_EQUAL_UINT32(1U, caps.Config_NightMode);
}

/* ---- Guard clauses: OV5640_Init / OV5640_SetResolution -----------------
 * Both must reject an out-of-range value before writing a single
 * register - no ov5640_write_reg_Expect* call is set up in these tests,
 * so CMock fails them if the code under test writes anything anyway. */

void test_Init_rejects_a_resolution_past_the_maximum(void)
{
    OV5640_Object_t obj = {0};

    int32_t ret = OV5640_Init(&obj, OV5640_R800x480 + 1U, OV5640_RGB565);

    TEST_ASSERT_EQUAL_INT32(OV5640_ERROR, ret);
    TEST_ASSERT_EQUAL_UINT8(0U, obj.IsInitialized);
}

void test_Init_rejects_an_unsupported_pixel_format(void)
{
    OV5640_Object_t obj = {0};

    int32_t ret = OV5640_Init(&obj, OV5640_R640x480, 0xEEU /* not one of the OV5640_* formats */);

    TEST_ASSERT_EQUAL_INT32(OV5640_ERROR, ret);
    TEST_ASSERT_EQUAL_UINT8(0U, obj.IsInitialized);
}

void test_SetResolution_rejects_a_resolution_past_the_maximum(void)
{
    OV5640_Object_t obj = {0};

    int32_t ret = OV5640_SetResolution(&obj, OV5640_R800x480 + 1U);

    TEST_ASSERT_EQUAL_INT32(OV5640_ERROR, ret);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_SetPolarities_writes_the_control_register_on_valid_input);
    RUN_TEST(test_SetPolarities_rejects_a_null_object_without_touching_the_register);
    RUN_TEST(test_SetPolarities_rejects_an_invalid_pclk_polarity_without_touching_the_register);
    RUN_TEST(test_SetPolarities_propagates_a_register_write_failure);

    RUN_TEST(test_GetPolarities_unpacks_all_bits_high);
    RUN_TEST(test_GetPolarities_unpacks_all_bits_low);
    RUN_TEST(test_GetPolarities_rejects_null_output_pointers_without_touching_the_register);

    RUN_TEST(test_GetResolution_maps_640x480_to_R640x480);
    RUN_TEST(test_GetResolution_maps_800x480_to_R800x480);
    RUN_TEST(test_GetResolution_maps_160x120_to_R160x120);
    RUN_TEST(test_GetResolution_returns_error_for_an_unrecognized_size);

    RUN_TEST(test_GetCapabilities_rejects_a_null_object);
    RUN_TEST(test_GetCapabilities_reports_every_feature_supported);

    RUN_TEST(test_Init_rejects_a_resolution_past_the_maximum);
    RUN_TEST(test_Init_rejects_an_unsupported_pixel_format);
    RUN_TEST(test_SetResolution_rejects_a_resolution_past_the_maximum);

    return UNITY_END();
}
