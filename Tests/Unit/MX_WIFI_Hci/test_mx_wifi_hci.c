/**
 * @file    test_mx_wifi_hci.c
 * @brief   Unit tests for Platform/MX_WIFI/core/mx_wifi_hci.c, scoped to
 *          mx_wifi_hci_init/_send/_deinit with MX_WIFI_USE_SPI=1 (this
 *          project's actual transport - see mx_wifi_conf.h in this
 *          directory) where _send is a thin pass-through to the
 *          registered low-level send function, not SLIP framing.
 *
 *          mx_wifi_hci_recv()/_input() are NOT covered here: they pull
 *          data through the bare-metal FIFO's blocking pop, which busy-
 *          waits on HAL_GetTick() - exercising that path meaningfully
 *          needs a HAL_GetTick() stub that actually advances per call,
 *          which is more machinery than this pass-through logic
 *          warrants. mx_wifi_slip.c's Tests/Unit/MX_WIFI_Slip already
 *          covers the SLIP codec these would otherwise indirectly touch.
 *
 *          No RTOS mocking: mx_wifi_conf.h in this directory selects the
 *          real, vendor-shipped bare-metal implementation
 *          (Platform/MX_WIFI/mx_wifi_bare_os.h + the non-CMSIS branch of
 *          Platform/MX_WIFI/core/mx_rtos_abs.c), not ThreadX/NetXDuo.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

#include "unity.h"
#include "mx_wifi_conf.h"
#include "core/mx_wifi_hci.h"
#include "io_pattern/mx_wifi_io.h"

/* Link-time stand-ins - see main.h's header comment. mx_rtos_abs.c's
 * noos_fifo_push()/pop()/sem_wait() reference HAL_GetTick() for their
 * busy-wait timeouts; mx_wifi_hci_init()/_deinit() only reach
 * noos_fifo_init()/_deinit(), which do not, so returning 0 is enough. */
uint32_t HAL_GetTick(void) { return 0; }
void HAL_Delay(uint32_t ms) { (void)ms; }

/* mx_wifi_hci_recv() (compiled into this test binary as part of
 * mx_wifi_hci.c, even though nothing here calls it) takes this as a
 * FIFO_POP() idle callback - never invoked by the tests below. */
void process_txrx_poll(uint32_t timeout) { (void)timeout; }

static uint8_t g_last_sent_data[64];
static uint16_t g_last_sent_len;
static uint16_t g_next_return_value;

static uint16_t test_low_level_send(uint8_t *data, uint16_t size)
{
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(sizeof(g_last_sent_data), size);
    memcpy(g_last_sent_data, data, size);
    g_last_sent_len = size;

    return g_next_return_value;
}

void setUp(void)
{
    memset(g_last_sent_data, 0, sizeof(g_last_sent_data));
    g_last_sent_len = 0;
    g_next_return_value = 0;

    TEST_ASSERT_EQUAL_INT32(0, mx_wifi_hci_init(test_low_level_send));
}

void tearDown(void)
{
    TEST_ASSERT_EQUAL_INT32(0, mx_wifi_hci_deinit());
}

void test_send_forwards_the_payload_unmodified_to_the_low_level_function(void)
{
    uint8_t payload[] = {0xAA, 0xBB, 0xCC};
    g_next_return_value = sizeof(payload);

    int32_t ret = mx_wifi_hci_send(payload, sizeof(payload));

    TEST_ASSERT_EQUAL_INT32(0, ret);
    TEST_ASSERT_EQUAL_UINT16(sizeof(payload), g_last_sent_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, g_last_sent_data, sizeof(payload));
}

void test_send_reports_failure_when_the_low_level_function_sends_fewer_bytes(void)
{
    uint8_t payload[] = {0x01, 0x02, 0x03, 0x04};
    g_next_return_value = sizeof(payload) - 1; /* short write */

    int32_t ret = mx_wifi_hci_send(payload, sizeof(payload));

    TEST_ASSERT_EQUAL_INT32(-1, ret);
}

void test_send_reports_success_for_a_zero_length_payload(void)
{
    uint8_t payload[1] = {0};
    g_next_return_value = 0;

    int32_t ret = mx_wifi_hci_send(payload, 0);

    TEST_ASSERT_EQUAL_INT32(0, ret);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_send_forwards_the_payload_unmodified_to_the_low_level_function);
    RUN_TEST(test_send_reports_failure_when_the_low_level_function_sends_fewer_bytes);
    RUN_TEST(test_send_reports_success_for_a_zero_length_payload);

    return UNITY_END();
}
