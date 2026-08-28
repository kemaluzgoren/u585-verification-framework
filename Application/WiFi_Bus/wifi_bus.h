/**
 * @file    wifi_bus.h
 * @brief   SPI transport for the onboard EMW3080 WiFi module (mx_wifi).
 *
 * Implements the IO backend that Platform/MX_WIFI/mx_wifi.c and
 * Platform/MX_CHIP/nx_driver_emw3080.c call into: mxwifi_probe(),
 * wifi_obj_get(), process_txrx_poll(), mxchip_WIFI_ISR() and
 * HAL_SPI_TransferCallback() (declared by mx_wifi_io.h, not redeclared
 * here). Nothing in this module needs to be called directly by the rest
 * of the application - nx_driver_emw3080_entry() drives it once NetXDuo
 * brings the IP instance up.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef WIFI_BUS_H
#define WIFI_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mx_wifi.h"

#ifdef __cplusplus
}
#endif

#endif /* WIFI_BUS_H */
