/**
 * @file    main.h
 * @brief   Empty stand-in for Core/Inc/main.h (STM32 HAL, not
 *          host-compilable). Real mx_wifi_conf_template.h includes
 *          "main.h" "to inherit some declarations from the current
 *          platform" - the only one Platform/MX_WIFI/core/mx_rtos_abs.c's
 *          bare-metal (MX_WIFI_USE_CMSIS_OS==0) branch actually needs is
 *          HAL_GetTick() (busy-wait timeouts) and HAL_Delay() (used by
 *          mx_wifi_bare_os.h's DELAY_MS()).
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FAKE_MAIN_H
#define FAKE_MAIN_H

#include <stdint.h>

/* mx_rtos_abs.c's bare-metal noos_sem_wait() uses this CMSIS volatile
 * qualifier macro (normally from core_cm*.h, pulled in transitively). */
#ifndef __IO
#define __IO volatile
#endif

uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t ms);

#endif /* FAKE_MAIN_H */
