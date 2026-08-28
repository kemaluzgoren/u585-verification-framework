/**
 * @file    mx_wifi.h
 * @brief   Minimal stand-in for Platform/MX_WIFI/mx_wifi.h. Real
 *          mx_wifi.h drags in mx_wifi_conf.h -> mx_wifi_azure_rtos_conf.h
 *          -> tx_api.h/nx_api.h (fine, so this fake includes the local
 *          fakes too) but also -> mx_wifi_conf_template.h -> main.h
 *          (STM32 HAL, not host-compilable) plus the whole
 *          MX_WIFIObject_t API surface mx_wifi_azure_rtos.c never
 *          touches - it only needs MX_ASSERT/MX_STAT/MX_STAT_LOG and the
 *          prototypes of the functions it defines (normally declared by
 *          mx_wifi_azure_rtos_conf.h, transitively included from here).
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FAKE_MX_WIFI_H
#define FAKE_MX_WIFI_H

#include "tx_api.h"
#include "nx_api.h"

#define MX_ASSERT(A)    ((void)0)
#define MX_STAT(A)
#define MX_STAT_LOG()

UINT mx_wifi_alloc_init(void);
void *mx_wifi_malloc(size_t size);
void mx_wifi_free(void *p);

NX_PACKET *mx_net_buffer_alloc(uint32_t n);
void mx_net_buffer_free(NX_PACKET *packet_ptr);

UINT mx_wifi_thread_init(TX_THREAD *thread_ptr, CHAR *name_ptr, VOID (*entry_function)(ULONG), ULONG entry_input,
                          ULONG stack_size, UINT priority);
UINT mx_wifi_thread_deinit(TX_THREAD *thread_ptr);
void mx_wifi_thread_terminate(void);

UINT mx_wifi_fifo_init(TX_QUEUE *queue_ptr, CHAR *name_ptr, ULONG size);
UINT mx_wifi_fifo_deinit(TX_QUEUE *queue_ptr);
UINT mx_wifi_fifo_push(TX_QUEUE *queue_ptr, void *source_ptr, ULONG wait_option);
void *mx_wifi_fifo_pop(TX_QUEUE *queue_ptr, ULONG wait_option);

#endif /* FAKE_MX_WIFI_H */
