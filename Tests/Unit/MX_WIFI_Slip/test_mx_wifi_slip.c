/**
 * @file    test_mx_wifi_slip.c
 * @brief   Unit tests for Platform/MX_WIFI/core/mx_wifi_slip.c (SLIP
 *          byte-stuffing framing used over the UART transport - not the
 *          SPI one this project actually uses, but the pure encode/
 *          decode algorithm is transport-independent).
 *
 *          No RTOS mocking: mx_wifi_conf.h in this directory selects the
 *          real, vendor-shipped bare-metal implementation
 *          (Platform/MX_WIFI/mx_wifi_bare_os.h + the non-CMSIS branch of
 *          Platform/MX_WIFI/core/mx_rtos_abs.c), which is pure C using
 *          malloc() - not a hand-written fake, and not ThreadX/NetXDuo.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

#include "unity.h"
#include "mx_wifi_conf.h" /* defines mx_buf_t before mx_wifi_slip.h uses it */
#include "mx_wifi_slip.h"

/* Link-time stand-ins for the two STM32 HAL functions
 * mx_wifi_bare_os.h/main.h expect (see main.h's header comment). Neither
 * is expected to actually run in these tests: slip_input_byte() only
 * calls DELAY_MS() when MX_NET_BUFFER_ALLOC() (malloc()-backed here)
 * fails to allocate, which does not happen at these buffer sizes. */
uint32_t HAL_GetTick(void) { return 0; }
void HAL_Delay(uint32_t ms) { (void)ms; }

void setUp(void) {}
void tearDown(void) {}

/* ---- slip_transfer (encode) --------------------------------------------- */

void test_transfer_wraps_plain_data_with_start_and_end(void)
{
    uint8_t data[] = {0x01, 0x02, 0x03};
    uint16_t outlen = 0;

    uint8_t *frame = slip_transfer(data, sizeof(data), &outlen);

    TEST_ASSERT_NOT_NULL(frame);
    TEST_ASSERT_EQUAL_UINT16(5, outlen);
    TEST_ASSERT_EQUAL_UINT8(SLIP_START, frame[0]);
    TEST_ASSERT_EQUAL_UINT8(0x01, frame[1]);
    TEST_ASSERT_EQUAL_UINT8(0x02, frame[2]);
    TEST_ASSERT_EQUAL_UINT8(0x03, frame[3]);
    TEST_ASSERT_EQUAL_UINT8(SLIP_END, frame[4]);

    MX_WIFI_FREE(frame);
}

void test_transfer_escapes_an_embedded_start_byte(void)
{
    uint8_t data[] = {(uint8_t)SLIP_START};
    uint16_t outlen = 0;

    uint8_t *frame = slip_transfer(data, sizeof(data), &outlen);

    TEST_ASSERT_NOT_NULL(frame);
    TEST_ASSERT_EQUAL_UINT16(4, outlen); /* START, ESCAPE, ESCAPE_START, END */
    TEST_ASSERT_EQUAL_UINT8(SLIP_START, frame[0]);
    TEST_ASSERT_EQUAL_UINT8(SLIP_ESCAPE, frame[1]);
    TEST_ASSERT_EQUAL_UINT8(SLIP_ESCAPE_START, frame[2]);
    TEST_ASSERT_EQUAL_UINT8(SLIP_END, frame[3]);

    MX_WIFI_FREE(frame);
}

void test_transfer_escapes_an_embedded_end_byte(void)
{
    uint8_t data[] = {(uint8_t)SLIP_END};
    uint16_t outlen = 0;

    uint8_t *frame = slip_transfer(data, sizeof(data), &outlen);

    TEST_ASSERT_NOT_NULL(frame);
    TEST_ASSERT_EQUAL_UINT16(4, outlen);
    TEST_ASSERT_EQUAL_UINT8(SLIP_ESCAPE, frame[1]);
    TEST_ASSERT_EQUAL_UINT8(SLIP_ESCAPE_END, frame[2]);

    MX_WIFI_FREE(frame);
}

void test_transfer_escapes_an_embedded_escape_byte(void)
{
    uint8_t data[] = {(uint8_t)SLIP_ESCAPE};
    uint16_t outlen = 0;

    uint8_t *frame = slip_transfer(data, sizeof(data), &outlen);

    TEST_ASSERT_NOT_NULL(frame);
    TEST_ASSERT_EQUAL_UINT16(4, outlen);
    TEST_ASSERT_EQUAL_UINT8(SLIP_ESCAPE, frame[1]);
    TEST_ASSERT_EQUAL_UINT8(SLIP_ESCAPE_ES, frame[2]);

    MX_WIFI_FREE(frame);
}

void test_transfer_of_empty_input_is_just_start_and_end(void)
{
    uint16_t outlen = 0;

    uint8_t *frame = slip_transfer(NULL, 0, &outlen);

    TEST_ASSERT_NOT_NULL(frame);
    TEST_ASSERT_EQUAL_UINT16(2, outlen);
    TEST_ASSERT_EQUAL_UINT8(SLIP_START, frame[0]);
    TEST_ASSERT_EQUAL_UINT8(SLIP_END, frame[1]);

    MX_WIFI_FREE(frame);
}

/* ---- slip_input_byte (decode) -------------------------------------------
 * Stateful across calls (mirrors bytes arriving one at a time off a real
 * UART), so each test feeds a full frame through in a loop and only the
 * final SLIP_END byte is expected to yield a buffer. */

static mx_buf_t *feed(const uint8_t *bytes, size_t count)
{
    mx_buf_t *result = NULL;

    for (size_t i = 0; i < count; i++)
    {
        mx_buf_t *got = slip_input_byte(bytes[i]);
        if (got != NULL)
        {
            result = got;
        }
    }

    return result;
}

void test_input_byte_decodes_a_plain_frame(void)
{
    const uint8_t frame[] = {(uint8_t)SLIP_START, 0x01, 0x02, 0x03, (uint8_t)SLIP_END};

    mx_buf_t *nbuf = feed(frame, sizeof(frame));

    TEST_ASSERT_NOT_NULL(nbuf);
    TEST_ASSERT_EQUAL_UINT32(3, MX_NET_BUFFER_GET_PAYLOAD_SIZE(nbuf));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(((uint8_t[]) {0x01, 0x02, 0x03}), (uint8_t *)MX_NET_BUFFER_PAYLOAD(nbuf), 3);

    MX_NET_BUFFER_FREE(nbuf);
}

void test_input_byte_unescapes_a_start_byte(void)
{
    const uint8_t frame[] = {
        (uint8_t)SLIP_START, (uint8_t)SLIP_ESCAPE, (uint8_t)SLIP_ESCAPE_START, (uint8_t)SLIP_END
    };

    mx_buf_t *nbuf = feed(frame, sizeof(frame));

    TEST_ASSERT_NOT_NULL(nbuf);
    TEST_ASSERT_EQUAL_UINT32(1, MX_NET_BUFFER_GET_PAYLOAD_SIZE(nbuf));
    TEST_ASSERT_EQUAL_UINT8(SLIP_START, ((uint8_t *)MX_NET_BUFFER_PAYLOAD(nbuf))[0]);

    MX_NET_BUFFER_FREE(nbuf);
}

void test_input_byte_unescapes_an_end_byte(void)
{
    const uint8_t frame[] = {
        (uint8_t)SLIP_START, (uint8_t)SLIP_ESCAPE, (uint8_t)SLIP_ESCAPE_END, (uint8_t)SLIP_END
    };

    mx_buf_t *nbuf = feed(frame, sizeof(frame));

    TEST_ASSERT_NOT_NULL(nbuf);
    TEST_ASSERT_EQUAL_UINT32(1, MX_NET_BUFFER_GET_PAYLOAD_SIZE(nbuf));
    TEST_ASSERT_EQUAL_UINT8(SLIP_END, ((uint8_t *)MX_NET_BUFFER_PAYLOAD(nbuf))[0]);

    MX_NET_BUFFER_FREE(nbuf);
}

void test_input_byte_unescapes_an_escape_byte(void)
{
    const uint8_t frame[] = {
        (uint8_t)SLIP_START, (uint8_t)SLIP_ESCAPE, (uint8_t)SLIP_ESCAPE_ES, (uint8_t)SLIP_END
    };

    mx_buf_t *nbuf = feed(frame, sizeof(frame));

    TEST_ASSERT_NOT_NULL(nbuf);
    TEST_ASSERT_EQUAL_UINT32(1, MX_NET_BUFFER_GET_PAYLOAD_SIZE(nbuf));
    TEST_ASSERT_EQUAL_UINT8(SLIP_ESCAPE, ((uint8_t *)MX_NET_BUFFER_PAYLOAD(nbuf))[0]);

    MX_NET_BUFFER_FREE(nbuf);
}

void test_input_byte_restarts_the_frame_on_a_mid_stream_start(void)
{
    /* An aborted frame (START, 0xAA) followed immediately by a fresh one
     * must discard the aborted bytes, not prepend them. */
    const uint8_t frame[] = {
        (uint8_t)SLIP_START, 0xAA,
        (uint8_t)SLIP_START, 0x01, 0x02,
        (uint8_t)SLIP_END
    };

    mx_buf_t *nbuf = feed(frame, sizeof(frame));

    TEST_ASSERT_NOT_NULL(nbuf);
    TEST_ASSERT_EQUAL_UINT32(2, MX_NET_BUFFER_GET_PAYLOAD_SIZE(nbuf));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(((uint8_t[]) {0x01, 0x02}), (uint8_t *)MX_NET_BUFFER_PAYLOAD(nbuf), 2);

    MX_NET_BUFFER_FREE(nbuf);
}

void test_input_byte_returns_null_before_a_frame_completes(void)
{
    TEST_ASSERT_NULL(slip_input_byte((uint8_t)SLIP_START));
    TEST_ASSERT_NULL(slip_input_byte(0x01));
    TEST_ASSERT_NULL(slip_input_byte(0x02));

    /* Finish the frame so the decoder's static state does not leak into
     * whichever test Unity happens to run next. */
    mx_buf_t *nbuf = slip_input_byte((uint8_t)SLIP_END);
    TEST_ASSERT_NOT_NULL(nbuf);
    MX_NET_BUFFER_FREE(nbuf);
}

/* ---- round trip: encode then decode the encoded bytes -------------------- */

void test_transfer_and_input_byte_round_trip_data_containing_every_special_byte(void)
{
    const uint8_t original[] = {
        0x00, (uint8_t)SLIP_START, 0x7E, (uint8_t)SLIP_END, 0xFF, (uint8_t)SLIP_ESCAPE, 0x10
    };
    uint16_t frame_len = 0;

    uint8_t *frame = slip_transfer((uint8_t *)original, sizeof(original), &frame_len);
    TEST_ASSERT_NOT_NULL(frame);

    mx_buf_t *nbuf = feed(frame, frame_len);

    TEST_ASSERT_NOT_NULL(nbuf);
    TEST_ASSERT_EQUAL_UINT32(sizeof(original), MX_NET_BUFFER_GET_PAYLOAD_SIZE(nbuf));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(original, (uint8_t *)MX_NET_BUFFER_PAYLOAD(nbuf), sizeof(original));

    MX_WIFI_FREE(frame);
    MX_NET_BUFFER_FREE(nbuf);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_transfer_wraps_plain_data_with_start_and_end);
    RUN_TEST(test_transfer_escapes_an_embedded_start_byte);
    RUN_TEST(test_transfer_escapes_an_embedded_end_byte);
    RUN_TEST(test_transfer_escapes_an_embedded_escape_byte);
    RUN_TEST(test_transfer_of_empty_input_is_just_start_and_end);

    RUN_TEST(test_input_byte_decodes_a_plain_frame);
    RUN_TEST(test_input_byte_unescapes_a_start_byte);
    RUN_TEST(test_input_byte_unescapes_an_end_byte);
    RUN_TEST(test_input_byte_unescapes_an_escape_byte);
    RUN_TEST(test_input_byte_restarts_the_frame_on_a_mid_stream_start);
    RUN_TEST(test_input_byte_returns_null_before_a_frame_completes);

    RUN_TEST(test_transfer_and_input_byte_round_trip_data_containing_every_special_byte);

    return UNITY_END();
}
