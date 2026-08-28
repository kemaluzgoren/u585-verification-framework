/**
 * @file    test_mx_address.c
 * @brief   Unit tests for Platform/MX_WIFI/core/mx_address.c (IPv4
 *          string <-> struct conversion used throughout mx_wifi). Pure
 *          parsing/formatting, no hardware dependency - the local
 *          mx_wifi.h in this directory stands in for the real one (see
 *          its header comment) so nothing RTOS-related needs mocking.
 *
 *          Expected values were computed independently by compiling
 *          mx_address.c into a throwaway host program and running it,
 *          not derived from reading this file's own logic.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

#include "unity.h"
#include "mx_address.h"

void setUp(void) {}
void tearDown(void) {}

/* ---- mx_aton_r ----------------------------------------------------------
 * addr->addr is stored so that reading it byte-by-byte from the lowest
 * memory address (as mx_ntoa does) recovers the dotted-decimal octets in
 * order on a little-endian host - the assertions below spell that out as
 * the 0xDDCCBBAA layout for "AA.BB.CC.DD" rather than the naively
 * expected 0xAABBCCDD, since that inversion is exactly what a byte-order
 * regression here would silently get backwards. */

void test_aton_r_converts_a_typical_address(void)
{
    int32_t result = mx_aton_r("192.168.1.10");

    TEST_ASSERT_EQUAL_HEX32(0x0A01A8C0U, (uint32_t)result);
}

void test_aton_r_converts_all_zeros(void)
{
    int32_t result = mx_aton_r("0.0.0.0");

    TEST_ASSERT_EQUAL_HEX32(0x00000000U, (uint32_t)result);
}

void test_aton_r_converts_all_255s(void)
{
    int32_t result = mx_aton_r("255.255.255.255");

    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFFU, (uint32_t)result);
}

void test_aton_r_converts_a_second_typical_address(void)
{
    int32_t result = mx_aton_r("10.0.0.1");

    TEST_ASSERT_EQUAL_HEX32(0x0100000AU, (uint32_t)result);
}

void test_aton_r_rejects_non_numeric_input(void)
{
    int32_t result = mx_aton_r("not.an.ip");

    TEST_ASSERT_EQUAL_INT32(0, result);
}

/* Documents an actual gap in mx_aton(): the case-4 branch (a.b.c.d) only
 * range-checks the trailing octet ("d" <= 255) against
 * OV5640-style expectations; it never checks parts[0..2] ("a","b","c")
 * individually, so an out-of-range leading octet like 999 is accepted
 * instead of rejected. This pins down the CURRENT behaviour so a future
 * change to it is a deliberate, visible decision - not a claim that 999
 * *should* be accepted. */
void test_aton_r_does_not_validate_leading_octets_above_255(void)
{
    int32_t result = mx_aton_r("999.1.1.1");

    TEST_ASSERT_NOT_EQUAL(0, result);
}

/* ---- mx_ntoa -------------------------------------------------------- */

void test_ntoa_formats_a_typical_address(void)
{
    mx_ip_addr_t addr = {0};
    addr.addr = 0x0A01A8C0U; /* "192.168.1.10" per test_aton_r_converts_a_typical_address */

    TEST_ASSERT_EQUAL_STRING("192.168.1.10", mx_ntoa(&addr));
}

void test_ntoa_formats_all_255s(void)
{
    mx_ip_addr_t addr = {0};
    addr.addr = 0xFFFFFFFFU;

    TEST_ASSERT_EQUAL_STRING("255.255.255.255", mx_ntoa(&addr));
}

void test_ntoa_formats_all_zeros(void)
{
    mx_ip_addr_t addr = {0};
    addr.addr = 0x00000000U;

    TEST_ASSERT_EQUAL_STRING("0.0.0.0", mx_ntoa(&addr));
}

/* ---- round trip -------------------------------------------------------- */

void test_aton_r_and_ntoa_round_trip(void)
{
    const char *original = "172.16.254.1";
    int32_t parsed = mx_aton_r(original);

    mx_ip_addr_t addr = {0};
    addr.addr = (uint32_t)parsed;

    TEST_ASSERT_EQUAL_STRING(original, mx_ntoa(&addr));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_aton_r_converts_a_typical_address);
    RUN_TEST(test_aton_r_converts_all_zeros);
    RUN_TEST(test_aton_r_converts_all_255s);
    RUN_TEST(test_aton_r_converts_a_second_typical_address);
    RUN_TEST(test_aton_r_rejects_non_numeric_input);
    RUN_TEST(test_aton_r_does_not_validate_leading_octets_above_255);

    RUN_TEST(test_ntoa_formats_a_typical_address);
    RUN_TEST(test_ntoa_formats_all_255s);
    RUN_TEST(test_ntoa_formats_all_zeros);

    RUN_TEST(test_aton_r_and_ntoa_round_trip);

    return UNITY_END();
}
