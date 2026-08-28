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

/* Creates the IP instance (bound to the EMW3080 WiFi driver), DHCP
 * client and HTTP server, and starts the thread that brings them up.
 * Called from NetXDuo/App/app_netxduo.c's MX_NetXDuo_Init(), which owns
 * the byte_pool passed in. */
UINT Network_Service_Init(TX_BYTE_POOL *byte_pool);

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_SERVICE_H */
