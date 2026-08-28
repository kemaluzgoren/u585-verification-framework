/**
 * @file    nx_stubs.c
 * @brief   abort()-on-call stubs for the two NetXDuo functions
 *          Platform/MX_CHIP/mx_wifi_azure_rtos.c calls
 *          (mx_net_buffer_alloc()/free(), which none of this directory's
 *          tests exercise - see nx_api.h's header comment). Needed only
 *          so the file links; a loud abort() instead of a plausible-looking
 *          fake result if a test ever does reach them by accident.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-27
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

#include "nx_api.h"

UINT nx_packet_allocate(NX_PACKET_POOL *pool_ptr, NX_PACKET **packet_ptr, ULONG packet_type, ULONG wait_option) {
    (void)pool_ptr;
    (void)packet_ptr;
    (void)packet_type;
    (void)wait_option;
    fprintf(stderr, "nx_stubs: nx_packet_allocate was called - out of scope for this ThreadX-focused test "
                     "directory (see Tests/Integration/MX_WIFI_AzureRtos for NetX coverage)\n");
    abort();
}

UINT nx_packet_release(NX_PACKET *packet_ptr) {
    (void)packet_ptr;
    fprintf(stderr, "nx_stubs: nx_packet_release was called - out of scope for this ThreadX-focused test "
                     "directory (see Tests/Integration/MX_WIFI_AzureRtos for NetX coverage)\n");
    abort();
}
