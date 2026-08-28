/**
 * @file    nx_api.h
 * @brief   Minimal stand-in for NetXDuo's nx_api.h, providing exactly the
 *          types/functions Platform/MX_CHIP/mx_wifi_azure_rtos.c and
 *          nx_driver_emw3080.h (declarations only, not
 *          nx_driver_framework.c's body - that file is not compiled by
 *          this test, see this directory's CMakeLists.txt) need.
 *
 *          NetXDuo itself is out of scope for this test directory (see
 *          test_mx_wifi_threadx.c's header comment - this one is about
 *          ThreadX, not NetXDuo); nx_packet_allocate()/nx_packet_release()
 *          are given plain abort()-on-call stubs in nx_stubs.c rather
 *          than mocked, since none of this directory's tests call the
 *          mx_net_buffer_alloc()/free() functions that reach them - see
 *          Tests/Integration/MX_WIFI_AzureRtos for NetX-focused coverage
 *          of those two.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-27
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FAKE_NX_API_H
#define FAKE_NX_API_H

#include "tx_api.h"

typedef unsigned char UCHAR;

#define NX_SUCCESS          0U
#define NX_NO_WAIT          0U
#define NX_RECEIVE_PACKET   1U

typedef struct NX_PACKET_POOL_STRUCT {
    ULONG nx_packet_pool_payload_size;
} NX_PACKET_POOL;

typedef struct NX_PACKET_STRUCT {
    struct NX_PACKET_STRUCT *nx_packet_next;
    UCHAR *nx_packet_prepend_ptr;
    UCHAR *nx_packet_append_ptr;
    UCHAR *nx_packet_data_start;
    UCHAR *nx_packet_data_end;
    ULONG nx_packet_length;
    NX_PACKET_POOL *nx_packet_pool_owner;
} NX_PACKET;

/* Opaque: mx_wifi_azure_rtos.c only ever stores/forwards these pointers,
 * never dereferences their contents. */
typedef struct NX_IP_STRUCT NX_IP;
typedef struct NX_INTERFACE_STRUCT NX_INTERFACE;
typedef struct NX_IP_DRIVER_STRUCT NX_IP_DRIVER;

UINT nx_packet_allocate(NX_PACKET_POOL *pool_ptr, NX_PACKET **packet_ptr, ULONG packet_type, ULONG wait_option);
UINT nx_packet_release(NX_PACKET *packet_ptr);

#endif /* FAKE_NX_API_H */
