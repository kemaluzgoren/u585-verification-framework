/**
 * @file    system.c
 * @brief   Application system interface.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-17
 *
 * SPDX-License-Identifier: MIT
 */

#include "system.h"
#include "i2c_bus.h"
#include "light_sensor.h"
#include "humidity_sensor.h"
#include "pressure_sensor.h"
#include "magneto_sensor.h"
#include "imu_sensor.h"
#include "sensor_service.h"
#include "camera_service.h"
#include "http_responses.h"

#include "main.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern DCMI_HandleTypeDef hdcmi;

#define SENSOR_SERVICE_THREAD_STACK_SIZE   (1024U)
#define SENSOR_SERVICE_THREAD_PRIORITY     (16U)

static TX_THREAD sensor_service_thread;

/* All MEMS sensors on this board (light, humidity/temp, pressure/temp,
 * magneto, IMU) share this single physical I2C2 peripheral, so they are
 * handed the same I2C_Bus_t instance rather than each wrapping hi2c2
 * separately. That is what lets i2c_bus.c serialize their transfers with
 * one lock once RTOS support is added. */
static I2C_Bus_t hi2c2_bus;

/* The camera connector is wired to the dedicated I2C1 peripheral, not
 * the sensors' shared I2C2 bus - its own I2C_Bus_t instance. */
static I2C_Bus_t hi2c1_bus;


void System_Init(void) {

    I2C_Bus_Init(&hi2c2_bus, &hi2c2);

    if (light_sensor_init(&hi2c2_bus) != 0) {
        Error_Handler();
    }

    if (humidity_sensor_init(&hi2c2_bus) != 0) {
        Error_Handler();
    }

    if (pressure_sensor_init(&hi2c2_bus) != 0) {
        Error_Handler();
    }

    if (magneto_sensor_init(&hi2c2_bus) != 0) {
        Error_Handler();
    }

    if (imu_sensor_init(&hi2c2_bus) != 0) {
        Error_Handler();
    }

    I2C_Bus_Init(&hi2c1_bus, &hi2c1);

    if (Camera_Service_Init(&hi2c1_bus, &hdcmi) != 0) {
        Error_Handler();
    }

    /* Network_Service (Application/Network) never includes camera_service.h -
       it only knows the Http_Image_Provider_t shape (http_responses.h).
       This is the one place, the composition root, that wires a concrete
       image source in. */
    Http_Responses_RegisterImageProvider(Camera_Service_ProvideFrame);
}


UINT System_Start(TX_BYTE_POOL *byte_pool) {

    VOID *sensor_service_thread_stack;
    UINT ret;

    sensor_service_init();

    ret = tx_byte_allocate(byte_pool, &sensor_service_thread_stack, SENSOR_SERVICE_THREAD_STACK_SIZE, TX_NO_WAIT);
    if (ret != TX_SUCCESS) {
        return ret;
    }

    ret = tx_thread_create(&sensor_service_thread, "Sensor Service Thread", sensor_service_task, 0,
                            sensor_service_thread_stack, SENSOR_SERVICE_THREAD_STACK_SIZE,
                            SENSOR_SERVICE_THREAD_PRIORITY, SENSOR_SERVICE_THREAD_PRIORITY,
                            TX_NO_TIME_SLICE, TX_AUTO_START);
    if (ret != TX_SUCCESS) {
        return ret;
    }

    return Camera_Service_Start(byte_pool);
}
