/**
 * @file    camera_service.c
 * @brief   OV5640/DCMI JPEG capture.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-24
 *
 * SPDX-License-Identifier: MIT
 */

#include "camera_service.h"

#include <string.h>

#include "ov5640.h"
#include "stm32u5xx_hal.h"
#include "main.h"

#define CAMERA_I2C_ADDRESS          (0x78U) /* OV5640 SCCB default (7-bit 0x3C, HAL 8-bit form) */
#define CAMERA_I2C_TIMEOUT          (100U)

/* Worst-case compressed frame size at 640x480 - matches ST's own
 * b-u585i_iot02a_camera_demo reference, which found this margin
 * sufficient across lighting/detail conditions at this resolution.
 * CAMERA_MAX_FRAME_SIZE (camera_service.h) is the same value, exposed so
 * callers can size their own Camera_Service_CopyLatestFrame() destination
 * buffer against it. */
#define CAMERA_BUF_LENGTH           ((uint32_t)CAMERA_MAX_FRAME_SIZE)

#define CAMERA_THREAD_STACK_SIZE    (1024U)
#define CAMERA_THREAD_PRIORITY      (4U)

static DCMI_HandleTypeDef *dcmi_handle = NULL;

static I2C_Bus_t *bus = NULL;
static OV5640_Object_t camera_obj;

static TX_THREAD camera_thread;
static TX_SEMAPHORE vsync_semaphore;
static TX_SEMAPHORE frame_ready_semaphore;
static TX_MUTEX frame_mutex;

/* Double-buffered: one half fills via DMA while the other half (the
 * frame captured on the previous VSYNC) is what current_frame points at
 * for HTTP responses to read. */
static uint32_t camera_buf[2][CAMERA_BUF_LENGTH / sizeof(uint32_t)];
static volatile uint32_t camera_buf_len;
static volatile uint8_t camera_error;

static uint8_t *current_frame_buf = NULL;
static uint32_t current_frame_len = 0;

static void camera_thread_entry(ULONG thread_input);

static int32_t Camera_IO_Init(void);
static int32_t Camera_IO_DeInit(void);
static int32_t Camera_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Camera_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t Camera_IO_GetTick(void);

int32_t Camera_Service_Init(I2C_Bus_t *i2c_bus, DCMI_HandleTypeDef *hdcmi) {

    int32_t ret;

    if (i2c_bus == NULL || hdcmi == NULL) {
        ret = OV5640_ERROR;
    } else {
        bus = i2c_bus;
        dcmi_handle = hdcmi;

        OV5640_IO_t io = {0};
        io.Init     = Camera_IO_Init;
        io.DeInit   = Camera_IO_DeInit;
        io.Address  = CAMERA_I2C_ADDRESS;
        io.WriteReg = Camera_IO_Write;
        io.ReadReg  = Camera_IO_Read;
        io.GetTick  = Camera_IO_GetTick;

        ret = OV5640_RegisterBusIO(&camera_obj, &io);
        if (ret == OV5640_OK) {
            ret = OV5640_Init(&camera_obj, OV5640_R640x480, OV5640_JPEG);
        }
    }

    return ret;
}

UINT Camera_Service_Start(TX_BYTE_POOL *byte_pool) {

    VOID *camera_thread_stack;
    UINT ret;

    ret = tx_semaphore_create(&vsync_semaphore, "Camera Vsync Semaphore", 0);
    if (ret != TX_SUCCESS) {
        return ret;
    }

    ret = tx_semaphore_create(&frame_ready_semaphore, "Camera Frame Ready Semaphore", 0);
    if (ret != TX_SUCCESS) {
        return ret;
    }

    ret = tx_mutex_create(&frame_mutex, "Camera Frame Mutex", TX_NO_INHERIT);
    if (ret != TX_SUCCESS) {
        return ret;
    }

    ret = tx_byte_allocate(byte_pool, &camera_thread_stack, CAMERA_THREAD_STACK_SIZE, TX_NO_WAIT);
    if (ret != TX_SUCCESS) {
        return ret;
    }

    ret = tx_thread_create(&camera_thread, "Camera Thread", camera_thread_entry, 0,
                            camera_thread_stack, CAMERA_THREAD_STACK_SIZE,
                            CAMERA_THREAD_PRIORITY, CAMERA_THREAD_PRIORITY,
                            TX_NO_TIME_SLICE, TX_AUTO_START);

    return ret;
}

int32_t Camera_Service_CopyLatestFrame(uint8_t *dest, uint32_t dest_capacity, uint32_t *len) {

    int32_t ret = 0;

    tx_mutex_get(&frame_mutex, TX_WAIT_FOREVER);

    if (current_frame_buf != NULL && current_frame_len <= dest_capacity) {
        memcpy(dest, current_frame_buf, current_frame_len);
        *len = current_frame_len;
        ret = 1;
    }

    tx_mutex_put(&frame_mutex);

    return ret;
}

int32_t Camera_Service_WaitForNextFrame(ULONG timeout) {

    return (tx_semaphore_get(&frame_ready_semaphore, timeout) == TX_SUCCESS) ? 1 : 0;
}

int32_t Camera_Service_ProvideFrame(uint8_t *dest, uint32_t dest_capacity, uint32_t *len, ULONG timeout) {

    if (timeout != TX_NO_WAIT) {
        if (Camera_Service_WaitForNextFrame(timeout) == 0) {
            return 0;
        }
    }

    return Camera_Service_CopyLatestFrame(dest, dest_capacity, len);
}

/**
  * Waits for each VSYNC (start of a new frame, signalled by
  * HAL_DCMI_VsyncEventCallback below), publishes the frame DMA just
  * finished filling as the current one, then restarts DMA into the
  * other half of the double buffer for the next frame.
  */
static void camera_thread_entry(ULONG thread_input) {

    uint8_t buf_index = 0;

    (void)thread_input;

    /* Matches ST's own b-u585i_iot02a_camera_demo reference
     * (Camera/app_camera.c): OV5640_Init() (Camera_Service_Init(), run
     * from System_Init() before this thread exists) only configures the
     * sensor over I2C - its PLL/streaming output needs a moment to settle
     * before DCMI/DMA capture is started against it. */
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);

    camera_error = 0;
    HAL_DCMI_Start_DMA(dcmi_handle, DCMI_MODE_CONTINUOUS, (uint32_t)camera_buf[buf_index],
                        CAMERA_BUF_LENGTH / sizeof(uint32_t));

    while (1) {

        if (tx_semaphore_get(&vsync_semaphore, TX_WAIT_FOREVER) != TX_SUCCESS) {
            Error_Handler();
        }

        if (camera_error == 0) {
            tx_mutex_get(&frame_mutex, TX_WAIT_FOREVER);
            current_frame_buf = (uint8_t *)camera_buf[buf_index];
            current_frame_len = camera_buf_len;
            tx_mutex_put(&frame_mutex);

            tx_semaphore_ceiling_put(&frame_ready_semaphore, 1);

            buf_index ^= 1U;
        }

        camera_error = 0;
        HAL_DMA_Start_IT(dcmi_handle->DMA_Handle, (uint32_t)&dcmi_handle->Instance->DR,
                          (uint32_t)camera_buf[buf_index], CAMERA_BUF_LENGTH);
    }
}

/**
  * Fires once per frame at VSYNC, i.e. right as the frame DCMI/DMA just
  * finished filling camera_buf[buf_index] becomes complete - reads how
  * much of the buffer the DMA actually used (a compressed JPEG frame
  * rarely fills all of CAMERA_BUF_LENGTH) and hands off to
  * camera_thread_entry() to publish it and restart DMA into the other
  * buffer half.
  */
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi_handle) {

    camera_buf_len = CAMERA_BUF_LENGTH - __HAL_DMA_GET_COUNTER(hdcmi_handle->DMA_Handle);

    if (camera_buf_len != 0U) {
        HAL_DMA_Abort(hdcmi_handle->DMA_Handle);
        tx_semaphore_put(&vsync_semaphore);
    }
}

void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi_handle) {

    (void)hdcmi_handle;
    camera_error = 1;
}

static int32_t Camera_IO_Init(void) {
    return OV5640_OK;
}

static int32_t Camera_IO_DeInit(void) {
    return OV5640_OK;
}

static int32_t Camera_IO_Write(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemWrite(bus, DevAddress, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length, CAMERA_I2C_TIMEOUT);
}

static int32_t Camera_IO_Read(uint16_t DevAddress, uint16_t Reg, uint8_t *pData, uint16_t Length) {
    return I2C_Bus_MemRead(bus, DevAddress, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length, CAMERA_I2C_TIMEOUT);
}

static int32_t Camera_IO_GetTick(void) {
    return (int32_t)HAL_GetTick();
}
