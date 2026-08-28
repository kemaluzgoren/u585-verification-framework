/**
 * @file    wifi_bus.c
 * @brief   SPI transport for the onboard EMW3080 WiFi module (mx_wifi).
 *
 * Adapted from ST's reference io_pattern/mx_wifi_spi.c (mx_wifi component)
 * to this board's CubeMX pin labels (WRLS_*) and hspi2, and to live in
 * Application/ instead of Platform/MX_WIFI so the SPI/GPIO wiring stays
 * next to the rest of this project's peripheral glue (see I2C_Bus for the
 * same pattern applied to the sensor I2C bus).
 *
 * CubeMX prerequisites (see .ioc):
 *  - SPI2 (WRLS_SPI2_*): Master, Full-Duplex, NSS = Software.
 *  - WRLS_WKUP_W (PF15, module reset): GPIO_Output - already configured
 *    by MX_GPIO_Init(), used as-is here.
 *  - WRLS_SPI2_NSS (PB12, chip select): CubeMX leaves this pin
 *    unconfigured because SPI2 NSS is software-managed, so this file
 *    initializes it as a plain GPIO output itself (wifi_bus_cs_gpio_init).
 *  - WRLS_NOTIFY (PD14) and WRLS_FLOW (PG15): must be GPIO_MODE_IT_RISING
 *    with their EXTI line enabled in NVIC (EXTI14_IRQn / EXTI15_IRQn) -
 *    this still needs to be set in CubeMX and regenerated; the code below
 *    assumes the interrupt fires and only implements the callback.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include "mx_wifi.h"
#include "mx_wifi_conf.h"
#include "mx_wifi_io.h"
#include "core/mx_wifi_hci.h"
#include "nx_driver_emw3080.h"

#include "main.h"

#if !defined(MX_WIFI_USE_SPI) || (MX_WIFI_USE_SPI != 1)
#error "wifi_bus.c is the SPI backend; MX_WIFI_USE_SPI must be 1"
#endif

extern SPI_HandleTypeDef hspi2;

#pragma pack(1)
typedef struct _spi_header {
    uint8_t  type;
    uint16_t len;
    uint16_t lenx;
    uint8_t  dummy[3];
} spi_header_t;
#pragma pack()

/* Module control lines (onboard EMW3080, fixed by the board schematic). */
#define WIFI_RESET_PORT     WRLS_WKUP_W_GPIO_Port
#define WIFI_RESET_PIN      WRLS_WKUP_W_Pin

#define WIFI_CS_PORT        WRLS_SPI2_NSS_GPIO_Port
#define WIFI_CS_PIN         WRLS_SPI2_NSS_Pin

#define WIFI_IRQ_PORT       WRLS_NOTIFY_GPIO_Port
#define WIFI_IRQ_PIN        WRLS_NOTIFY_Pin

#define WIFI_FLOW_PORT      WRLS_FLOW_GPIO_Port
#define WIFI_FLOW_PIN       WRLS_FLOW_Pin

/* SPI protocol (module side, fixed by mx_wifi). */
#define SPI_WRITE           ((uint8_t)0x0A)
#define SPI_READ            ((uint8_t)0x0B)
#define SPI_DATA_SIZE       (MX_WIFI_HCI_DATA_SIZE)

#define WIFI_HW_RESET()                                                   \
    do {                                                                  \
        HAL_GPIO_WritePin(WIFI_RESET_PORT, WIFI_RESET_PIN, GPIO_PIN_RESET); \
        HAL_Delay(100);                                                   \
        HAL_GPIO_WritePin(WIFI_RESET_PORT, WIFI_RESET_PIN, GPIO_PIN_SET);   \
        HAL_Delay(1200);                                                  \
    } while (0)

#define WIFI_CS_HIGH()                                                    \
    HAL_GPIO_WritePin(WIFI_CS_PORT, WIFI_CS_PIN, GPIO_PIN_SET)

#define WIFI_CS_LOW()                                                     \
    HAL_GPIO_WritePin(WIFI_CS_PORT, WIFI_CS_PIN, GPIO_PIN_RESET)

#define WIFI_IRQ_IS_HIGH() \
    (GPIO_PIN_SET == HAL_GPIO_ReadPin(WIFI_IRQ_PORT, WIFI_IRQ_PIN))

#define WIFI_FLOW_IS_LOW() \
    (GPIO_PIN_RESET == HAL_GPIO_ReadPin(WIFI_FLOW_PORT, WIFI_FLOW_PIN))

static MX_WIFIObject_t MxWifiObj;
static SPI_HandleTypeDef *const HSpiMX = &hspi2;

static LOCK_DECLARE(SpiTxLock);
static SEM_DECLARE(SpiTxRxSem);
static SEM_DECLARE(SpiFlowRiseSem);
static SEM_DECLARE(SpiTransferDoneSem);

static uint8_t *SpiTxData = NULL;
static uint16_t SpiTxLen = 0;

static THREAD_DECLARE(WifiBusTxRxThreadId);
static volatile bool WifiBusTxRxTaskQuit = false;

static void wifi_bus_txrx_task(THREAD_CONTEXT_TYPE argument);
static int8_t wifi_bus_txrx_start(void);
static int8_t wifi_bus_txrx_stop(void);
static void wifi_bus_cs_gpio_init(void);

static HAL_StatusTypeDef wifi_bus_transmit_receive(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint8_t *rxdata,
                                                    uint16_t datalen, uint32_t timeout);
static HAL_StatusTypeDef wifi_bus_transmit(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint16_t datalen,
                                            uint32_t timeout);
static HAL_StatusTypeDef wifi_bus_receive(SPI_HandleTypeDef *hspi, uint8_t *rxdata, uint16_t datalen,
                                           uint32_t timeout);
static int8_t wifi_bus_wait_flow_high(uint32_t timeout);

/* Explicit GPIO_Init for the CS pin: CubeMX does not generate one for it
 * because SPI2's NSS is configured as software-managed (hspi2.Init.NSS =
 * SPI_NSS_SOFT), so the pin is otherwise left unconfigured. */
static void wifi_bus_cs_gpio_init(void) {

    GPIO_InitTypeDef gpio_init = {0};

    gpio_init.Pin = WIFI_CS_PIN;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(WIFI_CS_PORT, &gpio_init);

    WIFI_CS_HIGH();
}

static void wifi_bus_io_delay(uint32_t ms) {

    DELAY_MS(ms);
}

static int8_t wifi_bus_io_init(uint16_t mode) {

    int8_t ret = 0;

    if (MX_WIFI_RESET == mode) {
        WIFI_HW_RESET();
    } else {
        ret = wifi_bus_txrx_start();
    }

    return ret;
}

static int8_t wifi_bus_io_deinit(void) {

    wifi_bus_txrx_stop();
    return 0;
}

void HAL_SPI_TransferCallback(void *hspi) {

    (void)hspi;
    SEM_SIGNAL(SpiTransferDoneSem);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {

    if (hspi == HSpiMX) {
        MX_ASSERT(false);
    }
}

/* HAL_SPI_TransferCallback() above is mx_wifi's own bridge name, not a real
 * HAL callback - HAL_SPI_TransmitReceive_DMA/_Transmit_DMA/_Receive_DMA
 * actually complete through these three weak HAL callbacks. Without them
 * SpiTransferDoneSem is never signalled, so wifi_bus_transmit_receive()
 * (called with WAIT_FOREVER from process_txrx_poll()'s task loop) blocks
 * forever on a transfer that already finished at the DMA level - the
 * outer mipc_request() then times out after its own 10s waiting for an
 * answer that was actually sitting there unprocessed the whole time. */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {

    if (hspi == HSpiMX) {
        HAL_SPI_TransferCallback(hspi);
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {

    if (hspi == HSpiMX) {
        HAL_SPI_TransferCallback(hspi);
    }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {

    if (hspi == HSpiMX) {
        HAL_SPI_TransferCallback(hspi);
    }
}

/* Called from HAL_GPIO_EXTI_Rising_Callback() below when the NOTIFY or
 * FLOW pin rises - the module uses these two lines to signal "data ready
 * to send/receive" and "ready to accept the next SPI byte" respectively. */
void mxchip_WIFI_ISR(uint16_t isr_source) {

    if (WIFI_IRQ_PIN == isr_source) {
        SEM_SIGNAL(SpiTxRxSem);
    }
    if (WIFI_FLOW_PIN == isr_source) {
        SEM_SIGNAL(SpiFlowRiseSem);
    }
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin) {

    if (GPIO_Pin == WIFI_IRQ_PIN) {
        mxchip_WIFI_ISR(WIFI_IRQ_PIN);
        nx_driver_emw3080_interrupt();
    } else if (GPIO_Pin == WIFI_FLOW_PIN) {
        mxchip_WIFI_ISR(WIFI_FLOW_PIN);
        nx_driver_emw3080_interrupt();
    }
}

static int8_t wifi_bus_wait_flow_high(uint32_t timeout) {

    int8_t ret = 0;

    if (SEM_WAIT(SpiFlowRiseSem, timeout, NULL) != SEM_OK) {
        ret = -1;
    }
    if (WIFI_FLOW_IS_LOW()) {
        ret = -1;
    }

    return ret;
}

static uint16_t wifi_bus_write(uint8_t *data, uint16_t len) {

    uint16_t sent;

    LOCK(SpiTxLock);

    if ((NULL == data) || (0 == len) || (len > SPI_DATA_SIZE)) {
        SpiTxLen = 0;
        sent = 0;
    } else {
        SpiTxData = data;
        SpiTxLen = len;
        SEM_SIGNAL(SpiTxRxSem);
        sent = len;
    }

    UNLOCK(SpiTxLock);

    return sent;
}

static uint16_t wifi_bus_read(uint8_t *buffer, uint16_t buff_size) {

    (void)buffer;
    (void)buff_size;
    return 0;
}

static HAL_StatusTypeDef wifi_bus_transmit_receive(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint8_t *rxdata,
                                                    uint16_t datalen, uint32_t timeout) {

    HAL_StatusTypeDef ret;

#if (DMA_ON_USE == 1)
    ret = HAL_SPI_TransmitReceive_DMA(hspi, txdata, rxdata, datalen);
    SEM_WAIT(SpiTransferDoneSem, timeout, NULL);
#else
    ret = HAL_SPI_TransmitReceive(hspi, txdata, rxdata, datalen, timeout);
#endif

    return ret;
}

static HAL_StatusTypeDef wifi_bus_transmit(SPI_HandleTypeDef *hspi, uint8_t *txdata, uint16_t datalen,
                                            uint32_t timeout) {

    HAL_StatusTypeDef ret;

#if (DMA_ON_USE == 1)
    ret = HAL_SPI_Transmit_DMA(hspi, txdata, datalen);
    SEM_WAIT(SpiTransferDoneSem, timeout, NULL);
#else
    ret = HAL_SPI_Transmit(hspi, txdata, datalen, timeout);
#endif

    return ret;
}

static HAL_StatusTypeDef wifi_bus_receive(SPI_HandleTypeDef *hspi, uint8_t *rxdata, uint16_t datalen,
                                           uint32_t timeout) {

    HAL_StatusTypeDef ret;

#if (DMA_ON_USE == 1)
    ret = HAL_SPI_Receive_DMA(hspi, rxdata, datalen);
    SEM_WAIT(SpiTransferDoneSem, timeout, NULL);
#else
    ret = HAL_SPI_Receive(hspi, rxdata, datalen, timeout);
#endif

    return ret;
}

/* One full SPI transaction with the module: header exchange (tells each
 * side how much the other wants to send) followed by the data payload,
 * all under one CS assertion. Called in a loop by wifi_bus_txrx_task(). */
void process_txrx_poll(uint32_t timeout) {

    static mx_buf_t *netb = NULL;

    WIFI_CS_HIGH();

    while (netb == NULL) {
        netb = MX_NET_BUFFER_ALLOC(MX_WIFI_BUFFER_SIZE);
        if (netb == NULL) {
            DELAY_MS(1);
        }
    }

    if (SEM_WAIT(SpiTxRxSem, timeout, NULL) == SEM_OK) {

        LOCK(SpiTxLock);
        {
            spi_header_t mheader = {0, 0, 0, {0}};
            spi_header_t sheader = {0, 0, 0, {0}};
            uint8_t *txdata = NULL;
            bool is_continue = true;

            if (SpiTxData == NULL) {
                if (!WIFI_IRQ_IS_HIGH()) {
                    is_continue = false;
                    if (WifiBusTxRxTaskQuit) {
                        MX_NET_BUFFER_FREE(netb);
                        netb = NULL;
                    }
                }
            } else {
                mheader.len = SpiTxLen;
                txdata = SpiTxData;
            }

            if (is_continue) {
                mheader.type = SPI_WRITE;
                mheader.lenx = (uint16_t)(~mheader.len);

                WIFI_CS_LOW();

                if (wifi_bus_wait_flow_high(timeout) == 0) {

                    if (HAL_OK == wifi_bus_transmit_receive(HSpiMX, (uint8_t *)&mheader, (uint8_t *)&sheader,
                                                             sizeof(mheader), timeout)) {

                        if ((sheader.type == SPI_READ) && (((sheader.len ^ sheader.lenx) & 0xFFFFU) == 0xFFFFU) &&
                            !((sheader.len == 0) && (mheader.len == 0))) {

                            if ((sheader.len <= SPI_DATA_SIZE) && (mheader.len <= SPI_DATA_SIZE)) {

                                uint16_t datalen = (mheader.len > sheader.len) ? mheader.len : sheader.len;
                                uint8_t *rxdata = NULL;

                                if (sheader.len > 0) {
                                    rxdata = MX_NET_BUFFER_PAYLOAD(netb);
                                }

                                if (wifi_bus_wait_flow_high(timeout) == 0) {

                                    HAL_StatusTypeDef ret;

                                    if (NULL != txdata) {
                                        SpiTxData = NULL;
                                        SpiTxLen = 0;
                                        ret = (NULL != rxdata)
                                                  ? wifi_bus_transmit_receive(HSpiMX, txdata, rxdata, datalen, timeout)
                                                  : wifi_bus_transmit(HSpiMX, txdata, datalen, timeout);
                                    } else {
                                        ret = wifi_bus_receive(HSpiMX, rxdata, datalen, timeout);
                                    }

                                    if ((HAL_OK == ret) && (sheader.len > 0)) {
                                        MX_NET_BUFFER_SET_PAYLOAD_SIZE(netb, sheader.len);
                                        mx_wifi_hci_input(netb);
                                        netb = NULL;
                                    }
                                }
                            }
                        }
                    }
                }

                WIFI_CS_HIGH();
            }
        }
        UNLOCK(SpiTxLock);
    }
}

static void wifi_bus_txrx_task(THREAD_CONTEXT_TYPE argument) {

    (void)argument;

    WifiBusTxRxTaskQuit = false;

    while (!WifiBusTxRxTaskQuit) {
        process_txrx_poll(WAIT_FOREVER);
    }

    WifiBusTxRxTaskQuit = false;

    THREAD_TERMINATE();
    THREAD_DEINIT(WifiBusTxRxThreadId);
}

static int8_t wifi_bus_txrx_start(void) {

    int8_t ret = 0;

    LOCK_INIT(SpiTxLock);
    SEM_INIT(SpiTxRxSem, 2);
    SEM_INIT(SpiFlowRiseSem, 1);
    SEM_INIT(SpiTransferDoneSem, 1);

    if (THREAD_OK != THREAD_INIT(WifiBusTxRxThreadId, wifi_bus_txrx_task, NULL, MX_WIFI_SPI_THREAD_STACK_SIZE,
                                  MX_WIFI_SPI_THREAD_PRIORITY)) {
        ret = -1;
    } else {
        WIFI_CS_HIGH();
    }

    return ret;
}

static int8_t wifi_bus_txrx_stop(void) {

    WifiBusTxRxTaskQuit = true;
    SEM_SIGNAL(SpiTxRxSem);

    while (WifiBusTxRxTaskQuit) {
        DELAY_MS(500);
    }

    THREAD_DEINIT(WifiBusTxRxThreadId);
    SEM_DEINIT(SpiTxRxSem);
    SEM_DEINIT(SpiFlowRiseSem);
    LOCK_DEINIT(SpiTxLock);

    return 0;
}

/* Called by nx_driver_emw3080.c (_nx_driver_emw3080_initialize). */
int32_t mxwifi_probe(void **ll_drv_context) {

    int32_t ret = -1;

    wifi_bus_cs_gpio_init();

    if (MX_WIFI_RegisterBusIO(&MxWifiObj, wifi_bus_io_init, wifi_bus_io_deinit, wifi_bus_io_delay, wifi_bus_write,
                               wifi_bus_read) == MX_WIFI_STATUS_OK) {
        if (NULL != ll_drv_context) {
            *ll_drv_context = &MxWifiObj;
        }
        ret = 0;
    }

    return ret;
}

MX_WIFIObject_t *wifi_obj_get(void) {

    return &MxWifiObj;
}
