/**
 * @file    test_http_pages.c
 * @brief   Regression tests for Application/Network/http_pages.c's static
 *          page content - specifically CameraPage's embedded stream URL.
 *
 *          Direct regression test for a real bug found this session:
 *          network_service.h's STREAM_SERVER_PORT used to be defined as
 *          "(81)" (parenthesized, the usual - and normally correct -
 *          defensive style for a numeric macro), but CameraPage
 *          stringifies that token as-is via XSTR() to build a URL, so the
 *          parens leaked straight into the page as "http://host:(81)/stream.jpg"
 *          - a URL no browser can load, silently breaking the embedded
 *          camera view with no build error anywhere. This file's own
 *          fake network_service.h (this directory) mirrors that exact
 *          hazard, so a regression here would be caught the same way.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-29
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

#include "unity.h"
#include "http_pages.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_CameraPage_embeds_a_valid_stream_url(void) {
    TEST_ASSERT_NOT_NULL(strstr(CameraPage, "http://'+location.hostname+':81/stream.jpg'"));
}

void test_CameraPage_has_no_stray_parentheses_around_the_port(void) {
    /* The exact bug: STREAM_SERVER_PORT stringified with its defensive
     * parens still attached. */
    TEST_ASSERT_NULL(strstr(CameraPage, "(81)"));
}

void test_CameraPage_links_back_to_home(void) {
    TEST_ASSERT_NOT_NULL(strstr(CameraPage, "href=\"/\""));
}

void test_IndexPage_links_to_camera_and_sensors_pages(void) {
    TEST_ASSERT_NOT_NULL(strstr(IndexPage, "href=\"/camera.html\""));
    TEST_ASSERT_NOT_NULL(strstr(IndexPage, "href=\"/sensors.html\""));
}

void test_SensorsPage_polls_the_json_endpoint(void) {
    TEST_ASSERT_NOT_NULL(strstr(SensorsPage, "fetch('/sensors.json')"));
}

void test_HelloBody_is_not_empty(void) {
    TEST_ASSERT_TRUE(strlen(HelloBody) > 0U);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_CameraPage_embeds_a_valid_stream_url);
    RUN_TEST(test_CameraPage_has_no_stray_parentheses_around_the_port);
    RUN_TEST(test_CameraPage_links_back_to_home);
    RUN_TEST(test_IndexPage_links_to_camera_and_sensors_pages);
    RUN_TEST(test_SensorsPage_polls_the_json_endpoint);
    RUN_TEST(test_HelloBody_is_not_empty);

    return UNITY_END();
}
