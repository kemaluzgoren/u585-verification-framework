/**
 * @file    network_service.h
 * @brief   WiFi/NetXDuo bring-up and HTTP server.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tx_api.h"
#include "nx_api.h"

/* Port StreamServer (network_service.c) listens on for "/stream.jpg" -
 * public so Application/Network/http_pages.c's CameraPage can embed the
 * real value rather than a second, easily-forgotten hardcoded "81".
 * Deliberately unparenthesized (unlike most numeric macros in this
 * project): http_pages.c stringifies this token as-is via XSTR() to
 * build a URL, where "(81)" would be wrong - every other use is as a
 * plain function argument, never inside a larger expression, so the
 * usual "wrap macro bodies in parens" precaution does not apply here. */
#define STREAM_SERVER_PORT  81

/* Creates the IP instance (bound to the EMW3080 WiFi driver), DHCP
 * client and HTTP server, and starts the thread that brings them up.
 * Called from NetXDuo/App/app_netxduo.c's MX_NetXDuo_Init(), which owns
 * the byte_pool passed in. */
UINT Network_Service_Init(TX_BYTE_POOL *byte_pool);

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_SERVICE_H */
