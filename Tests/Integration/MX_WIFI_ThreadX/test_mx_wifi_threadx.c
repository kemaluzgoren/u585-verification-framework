/**
 * @file    test_mx_wifi_threadx.c
 * @brief   Integration tests for the seam between
 *          Platform/MX_CHIP/mx_wifi_azure_rtos.c (mx_wifi's RTOS glue) and
 *          ThreadX - linked here against ThreadX's own real
 *          Middlewares/ST/threadx/common/src/*.c byte-pool and queue
 *          logic (see threadx_host_glue.c's header comment for how/why),
 *          rather than a CMock mock of tx_api.h as
 *          Tests/Integration/MX_WIFI_AzureRtos uses. That means a genuine
 *          byte-pool allocator - real fragmentation, real exhaustion,
 *          real free-list reuse - backs every mx_wifi_malloc()/free()
 *          call below, not a canned mock return value.
 *
 *          mx_wifi_thread_init()/mx_wifi_fifo_*() also call
 *          tx_thread_create()/tx_queue_create() etc. tx_queue_create() is
 *          plain portable C and is linked for real too, but
 *          tx_thread_create() needs Cortex-M assembly
 *          (_tx_thread_stack_build) that cannot run on host - see
 *          threadx_host_glue.c. test_thread_init_fails_without_reaching_thread_create
 *          below still exercises mx_wifi_thread_init() for real by making
 *          its internal mx_wifi_malloc() call fail (a real, undersized
 *          pool), and the mere fact this test binary links at all -
 *          without tx_thread_create.c/tx_thread_stack_build.S present -
 *          is a compile-time guarantee that no path in this file ever
 *          reaches real thread creation, not just a runtime assertion.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-27
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

#include "unity.h"
#include "mx_wifi.h"
#define NX_DRIVER_SOURCE
#include "nx_driver_emw3080.h"

/* mx_wifi_azure_rtos.c declares this extern; normally defined by
 * nx_driver_framework.c, which this test does not compile (mx_net_buffer_alloc/free,
 * the only functions that touch it, are out of scope here - see nx_api.h). */
NX_DRIVER_INFORMATION nx_driver_information;

void setUp(void) {
    memset(&nx_driver_information, 0, sizeof(nx_driver_information));
}

void tearDown(void) {
}

/* mx_wifi_alloc_init() creates mx_wifi_azure_rtos.c's own internal byte
 * pool exactly once per process; every test below shares that one real
 * pool (mirroring how the pool is created exactly once in firmware too),
 * so tests that need a clean pool allocate/free their own scratch blocks
 * rather than assuming the pool starts empty. */
void test_alloc_init_succeeds(void) {
    UINT status = mx_wifi_alloc_init();
    TEST_ASSERT_EQUAL_UINT(NX_SUCCESS, status);
}

void test_malloc_returns_real_writable_memory(void) {
    void *p = mx_wifi_malloc(64);
    TEST_ASSERT_NOT_NULL(p);

    /* Prove it is real memory, not a mock's canned pointer - write well
     * within the requested 64 bytes (the pool stores the next block's
     * header immediately after the usable region, with no padding, so
     * writing exactly up to a block's requested size corrupts pool
     * metadata - a real footgun this real allocator exposes that a mock
     * never could). */
    memset(p, 0xAA, 32);
    TEST_ASSERT_EQUAL_UINT8(0xAA, ((unsigned char *)p)[0]);
    TEST_ASSERT_EQUAL_UINT8(0xAA, ((unsigned char *)p)[31]);

    mx_wifi_free(p);
}

void test_malloc_fails_once_the_real_pool_is_exhausted(void) {
    /* mx_wifi_byte_pool_size (mx_wifi_azure_rtos.c) is 4KB; requesting
     * more than that can never fit, exercising _tx_byte_allocate()'s real
     * "not enough total space" rejection - the same class of bug as the
     * NX_APP_MEM_POOL_SIZE regression Tests/Integration/MX_WIFI_AzureRtos
     * already covers for NetX's packet pool, here for mx_wifi's own pool. */
    void *p = mx_wifi_malloc(64 * 1024);
    TEST_ASSERT_NULL(p);
}

void test_free_then_malloc_reuses_the_freed_space(void) {
    void *p1 = mx_wifi_malloc(128);
    TEST_ASSERT_NOT_NULL(p1);

    mx_wifi_free(p1);

    /* Real free-list coalescing/reuse behavior - meaningless to assert
     * against a mock, since a mock has no actual free list to reuse. */
    void *p2 = mx_wifi_malloc(128);
    TEST_ASSERT_EQUAL_PTR(p1, p2);

    mx_wifi_free(p2);
}

void test_thread_init_fails_without_reaching_thread_create(void) {
    /* Exhaust the real pool first (mx_wifi_thread_init()'s stack size
     * request cannot possibly fit afterwards), so its internal
     * mx_wifi_malloc() call genuinely fails and it returns before ever
     * calling tx_thread_create() - which, as this file's header comment
     * explains, is not even linked into this test binary. Frees every
     * filler block afterwards so later tests still have a usable pool -
     * this is the shared process-wide pool, see this file's top comment. */
    void *filler[128];
    int filler_count = 0;
    while (filler_count < 128) {
        void *p = mx_wifi_malloc(64);
        if (p == NULL) {
            break;
        }
        filler[filler_count++] = p;
    }

    TX_THREAD thread;
    UINT status = mx_wifi_thread_init(&thread, "test", NULL, 0, 256, 1);

    TEST_ASSERT_EQUAL_UINT(TX_NO_MEMORY, status);

    for (int i = 0; i < filler_count; i++) {
        mx_wifi_free(filler[i]);
    }
}

void test_fifo_init_and_deinit_use_a_real_queue(void) {
    TX_QUEUE queue;
    UINT status = mx_wifi_fifo_init(&queue, "test queue", 8);
    TEST_ASSERT_EQUAL_UINT(TX_SUCCESS, status);
    TEST_ASSERT_NOT_NULL(queue.tx_queue_start);

    status = mx_wifi_fifo_deinit(&queue);
    TEST_ASSERT_EQUAL_UINT(TX_SUCCESS, status);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_alloc_init_succeeds);
    RUN_TEST(test_malloc_returns_real_writable_memory);
    RUN_TEST(test_malloc_fails_once_the_real_pool_is_exhausted);
    RUN_TEST(test_free_then_malloc_reuses_the_freed_space);
    RUN_TEST(test_thread_init_fails_without_reaching_thread_create);
    RUN_TEST(test_fifo_init_and_deinit_use_a_real_queue);

    return UNITY_END();
}
