/**
 * @file    test_mx_wifi_azure_rtos.c
 * @brief   Integration tests for the seam between
 *          Platform/MX_CHIP/mx_wifi_azure_rtos.c (mx_wifi's RTOS glue)
 *          and the ThreadX/NetXDuo APIs it calls - mocked here via the
 *          local tx_api.h/nx_api.h/mx_wifi.h stand-ins (see their header
 *          comments), not this project's real headers.
 *
 *          test_buffer_alloc_rejects_a_size_larger_than_the_pool_payload
 *          is a direct regression test for the bug found and fixed this
 *          session: NX_APP_PACKET_PAYLOAD_SIZE (Application/Network/
 *          network_service.c) was set below MX_WIFI_BUFFER_SIZE, so this
 *          exact guard clause rejected every allocation and
 *          process_txrx_poll() (Application/WiFi_Bus/wifi_bus.c) spun
 *          forever at the highest thread priority in the project,
 *          starving Sensor_Service.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

#include "unity.h"
#include "mx_wifi.h"
#define NX_DRIVER_SOURCE
#include "nx_driver_emw3080.h"
#include "Mocktx_api.h"
#include "Mocknx_api.h"

/* mx_wifi_azure_rtos.c declares these extern; it expects some
 * translation unit to define them - normally nx_driver_framework.c
 * (nx_driver_information) and ThreadX itself (_tx_thread_current_ptr).
 * Neither is compiled into this test (see this directory's
 * CMakeLists.txt), so this file provides them instead. */
NX_DRIVER_INFORMATION nx_driver_information;
TX_THREAD *_tx_thread_current_ptr;

void setUp(void) {

    Mocktx_api_Init();
    Mocknx_api_Init();
    memset(&nx_driver_information, 0, sizeof(nx_driver_information));
    _tx_thread_current_ptr = NULL;
}

void tearDown(void) {

    Mocktx_api_Verify();
    Mocktx_api_Destroy();
    Mocknx_api_Verify();
    Mocknx_api_Destroy();
}

/* ---- mx_net_buffer_alloc --------------------------------------------- */

void test_buffer_alloc_returns_null_without_a_packet_pool(void)
{
    nx_driver_information.nx_driver_information_packet_pool_ptr = NULL;

    /* No nx_packet_allocate_Expect* call set up: CMock fails the test if
     * the code under test calls it anyway. */
    NX_PACKET *packet = mx_net_buffer_alloc(64);

    TEST_ASSERT_NULL(packet);
}

void test_buffer_alloc_rejects_a_size_larger_than_the_pool_payload(void)
{
    NX_PACKET_POOL pool = {0};
    pool.nx_packet_pool_payload_size = 1536; /* the exact undersized value this session shipped with */
    nx_driver_information.nx_driver_information_packet_pool_ptr = &pool;

    NX_PACKET *packet = mx_net_buffer_alloc(1542); /* MX_WIFI_BUFFER_SIZE that exposed the bug */

    TEST_ASSERT_NULL(packet);
}

void test_buffer_alloc_accepts_a_size_equal_to_the_pool_payload(void)
{
    NX_PACKET_POOL pool = {0};
    UCHAR backing[128];
    NX_PACKET raw_packet = {0};
    NX_PACKET *raw_packet_ptr = &raw_packet;

    pool.nx_packet_pool_payload_size = 100;
    nx_driver_information.nx_driver_information_packet_pool_ptr = &pool;

    raw_packet.nx_packet_prepend_ptr = backing;
    raw_packet.nx_packet_data_end = backing + sizeof(backing);

    nx_packet_allocate_ExpectAndReturn(&pool, NULL, NX_RECEIVE_PACKET, NX_NO_WAIT, NX_SUCCESS);
    nx_packet_allocate_IgnoreArg_packet_ptr();
    nx_packet_allocate_ReturnThruPtr_packet_ptr(&raw_packet_ptr);

    NX_PACKET *packet = mx_net_buffer_alloc(100);

    TEST_ASSERT_EQUAL_PTR(&raw_packet, packet);
}

void test_buffer_alloc_sets_up_the_packet_offsets_on_success(void)
{
    NX_PACKET_POOL pool = {0};
    UCHAR backing[128];
    NX_PACKET raw_packet = {0};
    const uint32_t n = 40;

    pool.nx_packet_pool_payload_size = 1600;
    nx_driver_information.nx_driver_information_packet_pool_ptr = &pool;

    NX_PACKET *raw_packet_ptr = &raw_packet;

    raw_packet.nx_packet_next = &raw_packet; /* must be cleared to NULL */
    raw_packet.nx_packet_prepend_ptr = backing;
    raw_packet.nx_packet_data_end = backing + sizeof(backing);

    nx_packet_allocate_ExpectAndReturn(&pool, NULL, NX_RECEIVE_PACKET, NX_NO_WAIT, NX_SUCCESS);
    nx_packet_allocate_IgnoreArg_packet_ptr();
    nx_packet_allocate_ReturnThruPtr_packet_ptr(&raw_packet_ptr);

    NX_PACKET *packet = mx_net_buffer_alloc(n);

    TEST_ASSERT_NOT_NULL(packet);
    TEST_ASSERT_NULL(packet->nx_packet_next);
    TEST_ASSERT_EQUAL_PTR(backing + 2, packet->nx_packet_prepend_ptr);
    TEST_ASSERT_EQUAL_PTR(backing + 2 + n, packet->nx_packet_append_ptr);
    TEST_ASSERT_EQUAL_UINT32(n, packet->nx_packet_length);
}

void test_buffer_alloc_returns_null_when_nx_packet_allocate_fails(void)
{
    NX_PACKET_POOL pool = {0};
    pool.nx_packet_pool_payload_size = 1600;
    nx_driver_information.nx_driver_information_packet_pool_ptr = &pool;

    nx_packet_allocate_ExpectAndReturn(&pool, NULL, NX_RECEIVE_PACKET, NX_NO_WAIT, TX_NO_MEMORY);
    nx_packet_allocate_IgnoreArg_packet_ptr();

    NX_PACKET *packet = mx_net_buffer_alloc(64);

    TEST_ASSERT_NULL(packet);
}

/* ---- mx_net_buffer_free ------------------------------------------------ */

void test_buffer_free_releases_the_packet(void)
{
    NX_PACKET packet = {0};

    nx_packet_release_ExpectAndReturn(&packet, NX_SUCCESS);

    mx_net_buffer_free(&packet);
}

/* ---- mx_wifi_malloc / mx_wifi_free -------------------------------------- */

void test_malloc_returns_the_allocated_pointer(void)
{
    int allocated_block;
    VOID *allocated_block_ptr = &allocated_block;

    /* tx_byte_allocate() is fully mocked, so mx_wifi_malloc()'s internal
     * byte pool is never touched for real - mx_wifi_alloc_init() need
     * not run first. */
    tx_byte_allocate_ExpectAndReturn(NULL, NULL, 64, TX_NO_WAIT, TX_SUCCESS);
    tx_byte_allocate_IgnoreArg_pool_ptr();
    tx_byte_allocate_IgnoreArg_memory_ptr();
    tx_byte_allocate_ReturnThruPtr_memory_ptr(&allocated_block_ptr);

    void *result = mx_wifi_malloc(64);

    TEST_ASSERT_EQUAL_PTR(&allocated_block, result);
}

void test_malloc_returns_null_when_the_pool_is_exhausted(void)
{
    tx_byte_allocate_ExpectAndReturn(NULL, NULL, 64, TX_NO_WAIT, TX_NO_MEMORY);
    tx_byte_allocate_IgnoreArg_pool_ptr();
    tx_byte_allocate_IgnoreArg_memory_ptr();

    void *result = mx_wifi_malloc(64);

    TEST_ASSERT_NULL(result);
}

void test_free_releases_the_block(void)
{
    int block;

    tx_byte_release_ExpectAndReturn(&block, TX_SUCCESS);

    mx_wifi_free(&block);
}

/* ---- mx_wifi_thread_init ------------------------------------------------ */

void test_thread_init_does_not_create_a_thread_when_the_stack_allocation_fails(void)
{
    tx_byte_allocate_ExpectAndReturn(NULL, NULL, 1024, TX_NO_WAIT, TX_NO_MEMORY);
    tx_byte_allocate_IgnoreArg_pool_ptr();
    tx_byte_allocate_IgnoreArg_memory_ptr();

    /* No tx_thread_create_Expect* call set up: CMock fails the test if
     * the code under test creates the thread on a NULL stack anyway. */
    TX_THREAD thread;
    UINT status = mx_wifi_thread_init(&thread, "test", NULL, 0, 1024, 1);

    TEST_ASSERT_EQUAL_UINT(TX_NO_MEMORY, status);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_buffer_alloc_returns_null_without_a_packet_pool);
    RUN_TEST(test_buffer_alloc_rejects_a_size_larger_than_the_pool_payload);
    RUN_TEST(test_buffer_alloc_accepts_a_size_equal_to_the_pool_payload);
    RUN_TEST(test_buffer_alloc_sets_up_the_packet_offsets_on_success);
    RUN_TEST(test_buffer_alloc_returns_null_when_nx_packet_allocate_fails);

    RUN_TEST(test_buffer_free_releases_the_packet);

    RUN_TEST(test_malloc_returns_the_allocated_pointer);
    RUN_TEST(test_malloc_returns_null_when_the_pool_is_exhausted);
    RUN_TEST(test_free_releases_the_block);

    RUN_TEST(test_thread_init_does_not_create_a_thread_when_the_stack_allocation_fails);

    return UNITY_END();
}
