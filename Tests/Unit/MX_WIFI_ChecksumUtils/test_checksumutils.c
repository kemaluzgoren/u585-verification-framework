/**
 * @file    test_checksumutils.c
 * @brief   Unit tests for Platform/MX_WIFI/core/checksumutils.c (CRC8/CRC16
 *          used to frame mx_wifi SPI/HCI traffic). Pure algorithm, no
 *          hardware dependency, so nothing needs mocking here.
 *
 *          Expected values were computed independently with a standalone
 *          host program running the same crc/update loop, not derived
 *          from reading this file's own logic.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

#include "unity.h"
#include "checksumutils.h"

void setUp(void) {}
void tearDown(void) {}

/* ---- CRC8 -------------------------------------------------------------- */

void test_CRC8_of_empty_input_is_zero(void)
{
    CRC8_Context ctx;
    uint8_t result = 0xFFU;

    CRC8_Init(&ctx);
    CRC8_Update(&ctx, NULL, 0);
    CRC8_Final(&ctx, &result);

    TEST_ASSERT_EQUAL_UINT8(0x00U, result);
}

void test_CRC8_of_a_single_byte(void)
{
    CRC8_Context ctx;
    const uint8_t data[] = {0xAB};
    uint8_t result = 0;

    CRC8_Init(&ctx);
    CRC8_Update(&ctx, data, sizeof(data));
    CRC8_Final(&ctx, &result);

    TEST_ASSERT_EQUAL_UINT8(0x8FU, result);
}

void test_CRC8_of_the_ascii_check_string(void)
{
    CRC8_Context ctx;
    const uint8_t data[] = "123456789";
    uint8_t result = 0;

    CRC8_Init(&ctx);
    CRC8_Update(&ctx, data, 9);
    CRC8_Final(&ctx, &result);

    TEST_ASSERT_EQUAL_UINT8(0xA1U, result);
}

void test_CRC8_accepts_input_fed_across_multiple_updates(void)
{
    CRC8_Context ctx;
    const uint8_t part1[] = "123";
    const uint8_t part2[] = "456789";
    uint8_t result = 0;

    CRC8_Init(&ctx);
    CRC8_Update(&ctx, part1, 3);
    CRC8_Update(&ctx, part2, 6);
    CRC8_Final(&ctx, &result);

    /* Must match the single-shot "123456789" result: a good CRC update
     * loop carries state across calls instead of restarting per call. */
    TEST_ASSERT_EQUAL_UINT8(0xA1U, result);
}

void test_CRC8_of_abc(void)
{
    CRC8_Context ctx;
    const uint8_t data[] = "abc";
    uint8_t result = 0;

    CRC8_Init(&ctx);
    CRC8_Update(&ctx, data, 3);
    CRC8_Final(&ctx, &result);

    TEST_ASSERT_EQUAL_UINT8(0x42U, result);
}

/* ---- CRC16 -------------------------------------------------------------- */

void test_CRC16_of_empty_input_is_zero(void)
{
    CRC16_Context ctx;
    uint16_t result = 0xFFFFU;

    CRC16_Init(&ctx);
    CRC16_Update(&ctx, NULL, 0);
    CRC16_Final(&ctx, &result);

    TEST_ASSERT_EQUAL_UINT16(0x0000U, result);
}

void test_CRC16_of_a_single_byte(void)
{
    CRC16_Context ctx;
    const uint8_t data[] = {0xAB};
    uint16_t result = 0;

    CRC16_Init(&ctx);
    CRC16_Update(&ctx, data, sizeof(data));
    CRC16_Final(&ctx, &result);

    TEST_ASSERT_EQUAL_UINT16(0x0481U, result);
}

void test_CRC16_of_the_ascii_check_string(void)
{
    CRC16_Context ctx;
    const uint8_t data[] = "123456789";
    uint16_t result = 0;

    CRC16_Init(&ctx);
    CRC16_Update(&ctx, data, 9);
    CRC16_Final(&ctx, &result);

    TEST_ASSERT_EQUAL_UINT16(0x31C3U, result);
}

void test_CRC16_of_abc(void)
{
    CRC16_Context ctx;
    const uint8_t data[] = "abc";
    uint16_t result = 0;

    CRC16_Init(&ctx);
    CRC16_Update(&ctx, data, 3);
    CRC16_Final(&ctx, &result);

    TEST_ASSERT_EQUAL_UINT16(0x9DD6U, result);
}

void test_CRC16_Final_is_idempotent_to_call_state_not_repeatable(void)
{
    /* CRC16_Final folds in two extra zero-bytes (a CCITT/XMODEM trailer
     * flush) via UpdateCRC16, which mutates ctx->crc - so this pins down
     * that behaviour: finalizing twice does NOT reproduce the same value,
     * because the second call flushes an already-flushed state. */
    CRC16_Context ctx;
    const uint8_t data[] = "abc";
    uint16_t first = 0;
    uint16_t second = 0;

    CRC16_Init(&ctx);
    CRC16_Update(&ctx, data, 3);
    CRC16_Final(&ctx, &first);
    CRC16_Final(&ctx, &second);

    TEST_ASSERT_EQUAL_UINT16(0x9DD6U, first);
    TEST_ASSERT_NOT_EQUAL(first, second);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_CRC8_of_empty_input_is_zero);
    RUN_TEST(test_CRC8_of_a_single_byte);
    RUN_TEST(test_CRC8_of_the_ascii_check_string);
    RUN_TEST(test_CRC8_accepts_input_fed_across_multiple_updates);
    RUN_TEST(test_CRC8_of_abc);

    RUN_TEST(test_CRC16_of_empty_input_is_zero);
    RUN_TEST(test_CRC16_of_a_single_byte);
    RUN_TEST(test_CRC16_of_the_ascii_check_string);
    RUN_TEST(test_CRC16_of_abc);
    RUN_TEST(test_CRC16_Final_is_idempotent_to_call_state_not_repeatable);

    return UNITY_END();
}
