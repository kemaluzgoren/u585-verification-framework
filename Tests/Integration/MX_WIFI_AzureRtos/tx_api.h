/**
 * @file    tx_api.h
 * @brief   Minimal stand-in for ThreadX's tx_api.h, providing exactly the
 *          types/functions Platform/MX_CHIP/mx_wifi_azure_rtos.c calls.
 *          This project only pulled the cortex_m33 ThreadX port into the
 *          repo, which is not host-compilable, so the real header cannot
 *          be used for this host-native test; this directory is listed
 *          before any real ThreadX include path in
 *          test_mx_wifi_azure_rtos's INCLUDE_DIRS so #include "tx_api.h"
 *          resolves here.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FAKE_TX_API_H
#define FAKE_TX_API_H

#include <stddef.h>
#include <stdint.h>

typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef char CHAR;
typedef void VOID;

#define TX_SUCCESS      0U
#define TX_NO_MEMORY    0x10U
#define TX_NO_WAIT      0U
#define TX_WAIT_FOREVER 0xFFFFFFFFUL
#define TX_AUTO_START   1U
#define TX_NO_TIME_SLICE 0U
#define TX_1_ULONG      1U

typedef struct TX_BYTE_POOL_STRUCT {
    int dummy;
} TX_BYTE_POOL;

typedef struct TX_THREAD_STRUCT {
    VOID *tx_thread_stack_start;
} TX_THREAD;

typedef struct TX_QUEUE_STRUCT {
    VOID *tx_queue_start;
} TX_QUEUE;

extern TX_THREAD *_tx_thread_current_ptr;

UINT tx_byte_pool_create(TX_BYTE_POOL *pool_ptr, CHAR *name_ptr, VOID *pool_start, ULONG pool_size);
UINT tx_byte_allocate(TX_BYTE_POOL *pool_ptr, VOID **memory_ptr, ULONG memory_size, ULONG wait_option);
UINT tx_byte_release(VOID *memory_ptr);

UINT tx_thread_create(TX_THREAD *thread_ptr, CHAR *name_ptr, VOID (*entry_function)(ULONG), ULONG entry_input,
                       VOID *stack_start, ULONG stack_size, UINT priority, UINT preempt_threshold,
                       ULONG time_slice, UINT auto_start);
UINT tx_thread_delete(TX_THREAD *thread_ptr);
UINT tx_thread_terminate(TX_THREAD *thread_ptr);

UINT tx_queue_create(TX_QUEUE *queue_ptr, CHAR *name_ptr, UINT message_size, VOID *queue_start, ULONG queue_size);
UINT tx_queue_delete(TX_QUEUE *queue_ptr);
UINT tx_queue_send(TX_QUEUE *queue_ptr, VOID *source_ptr, ULONG wait_option);
UINT tx_queue_receive(TX_QUEUE *queue_ptr, VOID *destination_ptr, ULONG wait_option);

#endif /* FAKE_TX_API_H */
