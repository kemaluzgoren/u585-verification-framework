/**
 * @file    mx_wifi_conf.h
 * @brief   Platform configuration required by Platform/MX_WIFI (mx_wifi.c)
 *          and Platform/MX_CHIP (nx_driver_emw3080.c). Selects SPI transport,
 *          NetXDuo network-bypass mode and the ThreadX RTOS glue.
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

#if (MX_WIFI_NETWORK_BYPASS_MODE != 1)
#error The NetX driver requires bypass mode
#endif

#if (MX_WIFI_USE_CMSIS_OS != 0)
#error The NetX driver does not support CMSIS OS
#endif

#include "wifi_credentials.h"
#include "mx_wifi_azure_rtos_conf.h"

/* mx_wifi_conf_template.h unconditionally pulls in mx_wifi_bare_os.h when
 * MX_WIFI_USE_CMSIS_OS is 0, but that header redefines LOCK_DECLARE,
 * SEM_DECLARE, THREAD_DECLARE, DELAY_MS, WAIT_FOREVER, SEM_OK, THREAD_OK
 * and FIFO_OK without guarding them - clobbering the ThreadX versions
 * mx_wifi_azure_rtos_conf.h just defined above. Pre-defining its include
 * guard makes the physical #include still resolve (the file must exist
 * on the include path) while skipping its body, since we are neither
 * CMSIS-RTOS nor bare-metal but ThreadX/NetXDuo directly. */
#define MX_WIFI_BARE_OS_H
#include "mx_wifi_conf_template.h"

#ifdef __cplusplus
}
#endif

#endif /* MX_WIFI_CONF_H */
