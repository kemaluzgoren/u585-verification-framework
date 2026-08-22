/**
 * @file    system.h
 * @brief   Application system interface.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-17
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef APPLICATION_SYSTEM_H_
#define APPLICATION_SYSTEM_H_

#include "tx_api.h"

void System_Init(void);


UINT System_Start(TX_BYTE_POOL *byte_pool);

#endif  /* APPLICATION_SYSTEM_H_ */