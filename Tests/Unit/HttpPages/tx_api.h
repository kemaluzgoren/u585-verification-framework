/**
 * @file    tx_api.h
 * @brief   Minimal stand-in for ThreadX's tx_api.h - the real
 *          Application/Network/network_service.h #include "tx_api.h" is
 *          a quote-include, so it always resolves relative to
 *          network_service.h's own directory first, before any -I search
 *          path; there is no way to shadow network_service.h itself the
 *          way other tests shadow a header several #include hops away
 *          (see e.g. Tests/Unit/OV5640's cmsis_compiler.h). Faking
 *          tx_api.h/nx_api.h instead lets the real network_service.h
 *          compile as-is - it only needs UINT and TX_BYTE_POOL from this
 *          file for Network_Service_Init()'s declaration, which
 *          http_pages.c (the thing actually under test here) never calls.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-29
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FAKE_TX_API_H
#define FAKE_TX_API_H

typedef unsigned int UINT;

typedef struct TX_BYTE_POOL_STRUCT {
    int dummy;
} TX_BYTE_POOL;

#endif /* FAKE_TX_API_H */
