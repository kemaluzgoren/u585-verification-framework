/**
 * @file    test_veml6030_reg.c
 * @brief   Unit tests for the wire byte-order logic in veml6030_reg.c.
 *
 * No mocking framework is needed here: veml6030_ctx_t already exposes the
 * bus as a pair of function pointers, so a test can plug in its own fake
 * ReadReg/WriteReg directly.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-17
 *
 * SPDX-License-Identifier: MIT
 */

#include "unity.h"
#include "veml6030_reg.h"

#include <string.h>

static uint8_t captured_bytes[2];
static uint16_t captured_reg;
static uint8_t read_reply_bytes[2];
static int32_t read_reply_status;

void setUp(void) {}
void tearDown(void) {}

static int32_t fake_write_reg(void *handle, uint16_t reg, uint8_t *pData, uint16_t length)
{
    (void)handle;
    captured_reg = reg;
    memcpy(captured_bytes, pData, length);
    return 0;
}

static int32_t fake_read_reg(void *handle, uint16_t reg, uint8_t *pData, uint16_t length)
{
    (void)handle;
    (void)reg;
    memcpy(pData, read_reply_bytes, length);
    return read_reply_status;
}

void test_veml6030_write_reg_sends_the_value_MSB_first_on_the_wire(void)
{
    veml6030_ctx_t ctx = { .WriteReg = fake_write_reg, .ReadReg = NULL, .handle = NULL };
    uint16_t value = 0x1234;

    int32_t ret = veml6030_write_reg(&ctx, 0x00, &value, 2);

    TEST_ASSERT_EQUAL_INT32(0, ret);
    TEST_ASSERT_EQUAL_UINT16(0x00, captured_reg);
    TEST_ASSERT_EQUAL_UINT8(0x12, captured_bytes[0]);
    TEST_ASSERT_EQUAL_UINT8(0x34, captured_bytes[1]);
}

void test_veml6030_read_reg_reconstructs_the_value_from_wire_bytes(void)
{
    veml6030_ctx_t ctx = { .WriteReg = NULL, .ReadReg = fake_read_reg, .handle = NULL };
    uint16_t value = 0;

    read_reply_bytes[0] = 0x12;
    read_reply_bytes[1] = 0x34;
    read_reply_status = 0;

    int32_t ret = veml6030_read_reg(&ctx, 0x04, &value, 2);

    TEST_ASSERT_EQUAL_INT32(0, ret);
    TEST_ASSERT_EQUAL_UINT16(0x1234, value);
}

void test_veml6030_read_reg_propagates_bus_error(void)
{
    veml6030_ctx_t ctx = { .WriteReg = NULL, .ReadReg = fake_read_reg, .handle = NULL };
    uint16_t value = 0xAAAA;

    read_reply_status = -1;

    int32_t ret = veml6030_read_reg(&ctx, 0x04, &value, 2);

    TEST_ASSERT_EQUAL_INT32(-1, ret);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_veml6030_write_reg_sends_the_value_MSB_first_on_the_wire);
    RUN_TEST(test_veml6030_read_reg_reconstructs_the_value_from_wire_bytes);
    RUN_TEST(test_veml6030_read_reg_propagates_bus_error);
    return UNITY_END();
}
