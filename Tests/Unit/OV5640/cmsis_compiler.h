/**
 * @file    cmsis_compiler.h
 * @brief   Empty stand-in for CMSIS's cmsis_compiler.h so ov5640_reg.h's
 *          #include <cmsis_compiler.h> resolves on this host-native x86
 *          build without pulling in real ARM Cortex-M intrinsics (which
 *          would not compile for the host target anyway). ov5640_reg.h
 *          does not actually use any CMSIS macro from it.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FAKE_CMSIS_COMPILER_H
#define FAKE_CMSIS_COMPILER_H

/* The real header pulls this in transitively (via cmsis_gcc.h); ov5640_reg.h
 * relies on that rather than including it directly. */
#include <stdint.h>

#endif
