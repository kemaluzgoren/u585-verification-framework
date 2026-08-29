/**
 * @file    threadx_host_glue.c
 * @brief   Minimal host "port" that lets ThreadX's own real
 *          Middlewares/ST/threadx/common/src/tx_mutex_*.c object-management
 *          logic run and be tested on the host compiler, instead of
 *          mocking tx_api.h - same technique as
 *          Tests/Integration/MX_WIFI_ThreadX/threadx_host_glue.c, trimmed
 *          down to only what tx_mutex_create()/get()/put() need (this
 *          test never touches byte pools, semaphores or queues, unlike
 *          that one). See that file's header comment for the fuller
 *          explanation of the general approach.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-29
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

#define TX_SOURCE_CODE
#include "tx_api.h"
#include "tx_thread.h"
#include "tx_mutex.h"

/* ---- tx_port.h TX_DISABLE_INLINE hooks: no real interrupts on a host
 * test process, so these are plain no-ops rather than PRIMASK/CPSID
 * assembly (see this test's CMakeLists.txt for TX_DISABLE_INLINE). ---- */

UINT _tx_thread_interrupt_disable(VOID) {
    return 0;
}

VOID _tx_thread_interrupt_restore(UINT previous_posture) {
    (VOID)previous_posture;
}

/* ---- Scheduler-wide globals tx_mutex_*.c expects to exist somewhere -
 * normally defined by tx_thread_initialize.c, which this harness does
 * not compile. ---- */

volatile UINT _tx_thread_preempt_disable = 0;
volatile ULONG _tx_thread_system_state = 0;

TX_MUTEX *_tx_mutex_created_ptr = TX_NULL;
ULONG _tx_mutex_created_count = 0;

/* Priority-inheritance hook: only ever wired up by real thread-delete
 * scheduler logic. Declared to satisfy the linker but never expected to
 * be invoked here regardless of inherit mode (see host_test_thread_init()
 * below for why). */
VOID (*_tx_thread_mutex_release)(TX_THREAD *thread_ptr) = TX_NULL;

TX_THREAD *_tx_thread_execute_ptr = TX_NULL;
TX_THREAD *_tx_thread_priority_list[TX_MAX_PRIORITIES];
ULONG _tx_thread_preempted_maps[TX_MAX_PRIORITIES / 32];

/* Represents this single host test process as "the current ThreadX
 * thread" - required because tx_mutex_get()/put() unconditionally call
 * _tx_mutex_priority_change(_tx_thread_current_ptr, ...) even for
 * sensor_service.c's TX_INHERIT mutex. Its state is deliberately never
 * TX_READY, so that call always takes the "thread not currently ready"
 * branch (a plain priority field assignment) instead of the
 * ready-list-manipulating branch, which needs a fully initialized
 * scheduler ready list this single-threaded harness does not provide -
 * true regardless of inherit mode, since this test only ever has one
 * "thread" get/put its own mutex, never a second thread contending for
 * it (the actual inheritance-triggering scenario). */
static TX_THREAD host_test_thread;
TX_THREAD *_tx_thread_current_ptr = &host_test_thread;

__attribute__((constructor)) static void host_test_thread_init(void) {
    host_test_thread.tx_thread_state = TX_SUSPENDED; /* anything != TX_READY */
    host_test_thread.tx_thread_priority = 16;
}

/* ---- Scheduler hooks a real blocking suspend/resume would need. This
 * test only ever does uncontended TX_NO_WAIT-equivalent mutex get/put (a
 * single "thread", never actually contended), so none of these are ever
 * reached at runtime - if one is, abort() loudly rather than silently
 * returning a made-up result. ---- */

static void host_glue_abort(const char *fn) {
    fprintf(stderr,
            "threadx_host_glue: %s was called - this test only exercises uncontended mutex "
            "get/put, which this single-threaded host harness deliberately does not go beyond\n",
            fn);
    abort();
}

VOID _tx_thread_system_suspend(TX_THREAD *thread_ptr) {
    (VOID)thread_ptr;
    host_glue_abort("_tx_thread_system_suspend");
}

VOID _tx_thread_system_ni_suspend(TX_THREAD *thread_ptr, ULONG wait_option) {
    (VOID)thread_ptr;
    (VOID)wait_option;
    host_glue_abort("_tx_thread_system_ni_suspend");
}

VOID _tx_thread_system_resume(TX_THREAD *thread_ptr) {
    (VOID)thread_ptr;
    host_glue_abort("_tx_thread_system_resume");
}

VOID _tx_thread_system_ni_resume(TX_THREAD *thread_ptr) {
    (VOID)thread_ptr;
    host_glue_abort("_tx_thread_system_ni_resume");
}

VOID _tx_thread_system_preempt_check(VOID) {
    /* no-op: nothing ever preempts a single host test process */
}

/* sensor_service_task() (Application/Sensors/Sensor_Service/sensor_service.c) -
 * the thread entry this test never calls, only sensor_service_run()
 * directly - still sits in the same object file, so the linker wants
 * this symbol resolved regardless. */
UINT _tx_thread_sleep(ULONG timer_ticks) {
    (VOID)timer_ticks;
    host_glue_abort("_tx_thread_sleep");
    return TX_NOT_DONE;
}
