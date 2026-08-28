/**
 * @file    tx_user.h
 * @brief   ThreadX user-configuration override, active only for this host
 *          test build (via -DTX_INCLUDE_USER_DEFINE_FILE, see this
 *          directory's CMakeLists.txt - the firmware build does not
 *          define that symbol, so this file plays no part in it).
 *
 *          Middlewares/ST/threadx/common/inc/tx_api.h defines ALIGN_TYPE
 *          (the type ThreadX uses to store a pointer inside a byte-pool
 *          block header - see tx_byte_release.c) as ULONG by default,
 *          which is exactly pointer-sized on this project's real Cortex-M
 *          target. On this 64-bit Windows/MinGW LLP64 host, sizeof(void*)
 *          is 8 but sizeof(unsigned long) is still 4: a real
 *          TX_BYTE_POOL* pointer written into an ALIGN_TYPE-sized slot
 *          gets truncated, corrupting the pool - reproduced as a segfault
 *          inside _tx_byte_release() the first time a test here freed a
 *          block after writing into it. tx_api.h's own comment
 *          ("must... also be large enough to hold a pointer type") and
 *          its ALIGN_TYPE_DEFINED guard make this exact override the
 *          intended fix, not a hack around it.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-27
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TX_USER_H
#define TX_USER_H

#define ALIGN_TYPE_DEFINED
typedef unsigned long long ALIGN_TYPE;

#endif /* TX_USER_H */
