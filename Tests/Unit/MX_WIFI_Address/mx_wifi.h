/**
 * @file    mx_wifi.h
 * @brief   Minimal stand-in for Platform/MX_WIFI/mx_wifi.h. This
 *          directory is listed before Platform/MX_WIFI in
 *          test_mx_address's INCLUDE_DIRS, so mx_address.c's
 *          #include "mx_wifi.h" resolves here instead of the real header.
 *
 *          mx_address.c only needs MX_ASSERT and MX_MAX_IP_LEN from the
 *          real header - which otherwise drags in mx_wifi_conf.h ->
 *          mx_wifi_azure_rtos_conf.h -> tx_api.h/nx_api.h, ThreadX/NetXDuo
 *          headers this host-native x86 test build has no business
 *          compiling against.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FAKE_MX_WIFI_H
#define FAKE_MX_WIFI_H

#define MX_MAX_IP_LEN   (16)
#define MX_ASSERT(A)    ((void)0)

#endif /* FAKE_MX_WIFI_H */
