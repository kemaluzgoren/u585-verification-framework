/**
 * @file    camera_service.h
 * @brief   OV5640/DCMI JPEG capture.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-24
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef CAMERA_SERVICE_H
#define CAMERA_SERVICE_H

#include <stdint.h>

#include "tx_api.h"
#include "i2c_bus.h"
#include "stm32u5xx_hal.h"

/* Brings up the OV5640 over I2C (SCCB) and configures it for 640x480
 * JPEG output. Must run before Camera_Service_Start(). hdcmi is the
 * DCMI handle to drive capture through (owned/instantiated by
 * Application/System/system.c, the composition root - camera_service.c
 * itself no longer reaches for a CubeMX-generated global directly).
 * Mirrors light_sensor_init()/humidity_sensor_init() etc.'s (bus, then
 * driver init) shape. */
int32_t Camera_Service_Init(I2C_Bus_t *i2c_bus, DCMI_HandleTypeDef *hdcmi);

/* Starts DCMI/DMA capture and the ThreadX thread that keeps it running.
 * Called from Application/System/system.c's System_Start(), the same
 * byte_pool-based allocation point Sensor_Service and Network_Service
 * use for their own threads. */
UINT Camera_Service_Start(TX_BYTE_POOL *byte_pool);

/* Upper bound on a captured JPEG frame's size - matches CAMERA_BUF_LENGTH
 * (camera_service.c). Callers size their own copy destination buffer
 * against this. */
#define CAMERA_MAX_FRAME_SIZE  (0xFFFCUL)

/* Copies the most recently captured whole JPEG frame into dest (whose
 * capacity the caller reports via dest_capacity) and reports its actual
 * length via *len. This copies - rather than handing back a pointer into
 * the live capture buffer - specifically so the caller can take as long
 * as it needs to send/process that copy (e.g. a slow WiFi/TCP send)
 * without blocking camera_thread_entry() from restarting DCMI capture
 * into that same buffer for the next frame; an earlier pointer-lending
 * version of this API held a lock across the network send and produced
 * visible stalls in the MJPEG stream once the packet pool backed up.
 * Returns 0 (leaving *len untouched) if no frame has completed capture
 * yet (e.g. a request arrives before the first VSYNC) or the captured
 * frame does not fit in dest_capacity; otherwise 1. */
int32_t Camera_Service_CopyLatestFrame(uint8_t *dest, uint32_t dest_capacity, uint32_t *len);

/* Blocks until the frame captured when this was called returns has not
 * already been handed out by this function before - i.e. a genuinely new
 * one, published since the last call. For an MJPEG-style consumer that
 * calls this in a loop before each Camera_Service_CopyLatestFrame(). Returns
 * 0 on timeout, 1 once a new frame is ready. */
int32_t Camera_Service_WaitForNextFrame(ULONG timeout);

/* Combines the two above behind one signature so a caller (e.g.
 * Application/Network/http_responses.c) can be handed a single function
 * pointer that serves both a "just give me whatever's current, don't
 * wait" request (timeout == TX_NO_WAIT: equivalent to calling
 * Camera_Service_CopyLatestFrame() directly) and a "block for the next
 * genuinely new frame" request (any other timeout: equivalent to
 * Camera_Service_WaitForNextFrame() followed by
 * Camera_Service_CopyLatestFrame()) - without that caller needing to know
 * camera_service.h exists at all. Return value/semantics otherwise match
 * Camera_Service_CopyLatestFrame(). */
int32_t Camera_Service_ProvideFrame(uint8_t *dest, uint32_t dest_capacity, uint32_t *len, ULONG timeout);

#endif /* CAMERA_SERVICE_H */
