/**
 * @file    mx_wifi_conf.h
 * @brief   Local stand-in for Application/WiFi_Bus/mx_wifi_conf.h, used
 *          only by this host-native test. The real one selects
 *          MX_WIFI_USE_CMSIS_OS=0 + mx_wifi_azure_rtos_conf.h (ThreadX),
 *          which is exactly the RTOS dependency this test avoids; this
 *          one selects MX_WIFI_USE_CMSIS_OS=0 too but lets
 *          mx_wifi_conf_template.h pull in the REAL, vendor-shipped
 *          Platform/MX_WIFI/mx_wifi_bare_os.h instead - a pure-C,
 *          malloc()-based buffer/lock implementation with no RTOS calls,
 *          meant by ST for exactly this "no OS" case.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef MX_WIFI_CONF_H
#define MX_WIFI_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#define MX_WIFI_USE_SPI                (1)
#define MX_WIFI_NETWORK_BYPASS_MODE    (1)
#define MX_WIFI_TX_BUFFER_NO_COPY      (1)
#define MX_WIFI_USE_CMSIS_OS           (0)
#define DMA_ON_USE                     (1)

#include "mx_wifi_conf_template.h"

#ifdef __cplusplus
}
#endif

#endif /* MX_WIFI_CONF_H */
