/**
 * @file    threadx_host_glue.c
 * @brief   Minimal host "port" that lets ThreadX's own real
 *          Middlewares/ST/threadx/common/src/*.c object-management logic
 *          (byte pools, queues) run and be tested on the host compiler,
 *          instead of mocking tx_api.h.
 *
 *          ThreadX's only vendored port here (ports/cortex_m33/gnu) is
 *          Cortex-M assembly for context switching, which cannot run on
 *          the host - but that assembly is isolated to a handful of
 *          scheduler entry points (_tx_thread_system_suspend/resume,
 *          _tx_thread_create's stack builder, ...) that the *creation*
 *          and *non-blocking get/put* paths of a byte pool/queue never
 *          reach. This file supplies:
 *
 *            - tx_port.h's TX_DISABLE_INLINE interrupt-disable hooks
 *              (_tx_thread_interrupt_disable/restore) as host no-ops -
 *              defining TX_DISABLE_INLINE (see this directory's
 *              CMakeLists.txt) routes tx_port.h's TX_DISABLE/TX_RESTORE
 *              through these instead of inline PRIMASK/CPSID assembly.
 *            - storage for the small set of module-level globals each
 *              real common/src file expects some translation unit to
 *              define (created-object linked lists, _tx_thread_current_ptr,
 *              ...) - normally provided by tx_thread_initialize.c and
 *              friends, which this harness does not compile (they pull in
 *              the rest of the scheduler).
 *            - abort()-on-call stubs for the scheduler hooks
 *              (_tx_thread_system_suspend/resume/..., _tx_thread_create/
 *              delete/terminate) that a genuinely blocking wait or a real
 *              thread would need. TX_NO_WAIT calls and puts/creates that
 *              never need to wake anyone never reach these - see each
 *              stub's comment - so a test hitting one is a sign the test
 *              (or the code under test) strayed outside that boundary,
 *              not something to silently tolerate.
 *
 *          This is the same "compile the real vendor logic against a
 *          minimal host stand-in" approach Tests/Unit/MX_WIFI_Slip and
 *          MX_WIFI_Hci already use for mx_wifi's bare-metal fallback path
 *          - applied here to ThreadX's own portable-C core instead of a
 *          CMock mock of tx_api.h.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-27
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

#define TX_SOURCE_CODE
#include "tx_api.h"
#include "tx_thread.h"
#include "tx_semaphore.h"
#include "tx_byte_pool.h"
#include "tx_mutex.h"
#include "tx_queue.h"

/* ---- tx_port.h TX_DISABLE_INLINE hooks: no real interrupts on a host
 * test process, so these are plain no-ops rather than PRIMASK/CPSID
 * assembly. ---- */

UINT _tx_thread_interrupt_disable(VOID) {
    return 0;
}

VOID _tx_thread_interrupt_restore(UINT previous_posture) {
    (VOID)previous_posture;
}

/* ---- Scheduler-wide globals each real common/src file expects to exist
 * somewhere - normally defined by tx_thread_initialize.c/tx_initialize_high_level.c,
 * which this harness does not compile. ---- */

volatile UINT _tx_thread_preempt_disable = 0;
volatile ULONG _tx_thread_system_state = 0;

TX_SEMAPHORE *_tx_semaphore_created_ptr = TX_NULL;
ULONG _tx_semaphore_created_count = 0;

TX_BYTE_POOL *_tx_byte_pool_created_ptr = TX_NULL;
ULONG _tx_byte_pool_created_count = 0;

TX_MUTEX *_tx_mutex_created_ptr = TX_NULL;
ULONG _tx_mutex_created_count = 0;

TX_QUEUE *_tx_queue_created_ptr = TX_NULL;
ULONG _tx_queue_created_count = 0;

/* Priority-inheritance hook: only ever wired up by real thread-delete
 * scheduler logic. This project's own mutexes are all created with
 * TX_NO_INHERIT (see Application/Camera/camera_service.c's frame_mutex),
 * so this pointer is declared to satisfy the linker but never expected to
 * be invoked. */
VOID (*_tx_thread_mutex_release)(TX_THREAD *thread_ptr) = TX_NULL;

TX_THREAD *_tx_thread_execute_ptr = TX_NULL;
TX_THREAD *_tx_thread_priority_list[TX_MAX_PRIORITIES];
ULONG _tx_thread_preempted_maps[TX_MAX_PRIORITIES / 32];

/* Represents this single host test process as "the current ThreadX
 * thread" - required because tx_mutex_get()/put() unconditionally call
 * _tx_mutex_priority_change(_tx_thread_current_ptr, ...). Its state is
 * deliberately never TX_READY, so that call always takes the "thread not
 * currently ready" branch (a plain priority field assignment) instead of
 * the ready-list-manipulating branch, which needs a fully initialized
 * scheduler ready list this single-threaded harness does not provide. */
static TX_THREAD host_test_thread;
TX_THREAD *_tx_thread_current_ptr = &host_test_thread;

__attribute__((constructor)) static void host_test_thread_init(void) {
    host_test_thread.tx_thread_state = TX_SUSPENDED; /* anything != TX_READY */
    host_test_thread.tx_thread_priority = 16;
}

/* ---- Scheduler hooks a real blocking suspend/resume or a real thread
 * would need. Every test in this directory only uses TX_NO_WAIT gets and
 * puts/creates that never need to wake a waiter, so none of these are
 * ever reached at runtime - if one is, abort() loudly rather than
 * silently returning a made-up result. ---- */

static void host_glue_abort(const char *fn) {
    fprintf(stderr,
            "threadx_host_glue: %s was called - this test exercised a real thread "
            "suspend/resume/create path, which this single-threaded host harness "
            "deliberately does not support (see threadx_host_glue.c's header comment)\n",
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

/* tx_thread_create/delete/terminate genuinely cannot run on this host
 * harness: the real implementations call _tx_thread_stack_build(), which
 * only exists as ARM assembly
 * (ports/cortex_m33/gnu/src/tx_thread_stack_build.S) - there is no
 * portable-C fallback, since building an initial exception-return stack
 * frame is inherently CPU-architecture-specific. See this directory's
 * test file for how the code paths that would reach these are still
 * tested (by making the tx_byte_allocate() call inside
 * mx_wifi_thread_init() fail first, for real). */

UINT _tx_thread_create(TX_THREAD *thread_ptr, CHAR *name_ptr, VOID (*entry_function)(ULONG), ULONG entry_input,
                        VOID *stack_start, ULONG stack_size, UINT priority, UINT preempt_threshold,
                        ULONG time_slice, UINT auto_start) {
    (VOID)thread_ptr;
    (VOID)name_ptr;
    (VOID)entry_function;
    (VOID)entry_input;
    (VOID)stack_start;
    (VOID)stack_size;
    (VOID)priority;
    (VOID)preempt_threshold;
    (VOID)time_slice;
    (VOID)auto_start;
    host_glue_abort("_tx_thread_create");
    return TX_NOT_DONE;
}

UINT _tx_thread_delete(TX_THREAD *thread_ptr) {
    (VOID)thread_ptr;
    host_glue_abort("_tx_thread_delete");
    return TX_NOT_DONE;
}

UINT _tx_thread_terminate(TX_THREAD *thread_ptr) {
    (VOID)thread_ptr;
    host_glue_abort("_tx_thread_terminate");
    return TX_NOT_DONE;
}
