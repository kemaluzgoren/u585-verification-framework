/**
 * @file    network_service.c
 * @brief   WiFi/NetXDuo bring-up and HTTP server.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#include "network_service.h"

#include <stdio.h>
#include <string.h>

#include "nxd_dhcp_client.h"
#include "nx_web_http_server.h"
#include "nx_driver_emw3080.h"
#include "http_responses.h"
#include "main.h"

#define HTTP_SERVER_PORT                (80)

/* Must be >= MX_WIFI_BUFFER_SIZE (mx_wifi_conf_template.h): with network
 * bypass mode this is MX_WIFI_MTU_SIZE(1500) + MX_WIFI_BYPASS_HEADER_SIZE(28)
 * + MX_WIFI_PBUF_LINK_HLEN(14) = 1542. At 1536 mx_net_buffer_alloc()
 * (Platform/MX_CHIP/mx_wifi_azure_rtos.c) rejects every allocation because
 * the requested size exceeds this pool's payload size, so
 * process_txrx_poll() (Application/WiFi_Bus/wifi_bus.c) spins forever
 * relinquishing at MX_WIFI_SPI_THREAD_PRIORITY (1, the highest priority in
 * this project) and starves every lower-priority thread, including
 * Sensor_Service - which is why sensor readings stopped updating too. */
#define NX_APP_PACKET_PAYLOAD_SIZE       (1600)
#define NX_APP_PACKET_POOL_SIZE          ((NX_APP_PACKET_PAYLOAD_SIZE + sizeof(NX_PACKET)) * 16)

/* Sized for a full CAMERA_BUF_LENGTH (0xFFFC, Application/Camera/camera_service.c)
 * JPEG frame plus response-header overhead - the previous *4 sizing was
 * fine for the "/hello" text reply but far too small to hold an actual
 * captured image; nx_packet_data_append() chains packets from this pool
 * automatically, but only if enough of them exist to chain. */
#define HTTP_SERVER_PACKET_POOL_SIZE     ((NX_APP_PACKET_PAYLOAD_SIZE + sizeof(NX_PACKET)) * 48)
#define HTTP_SERVER_THREAD_STACK_SIZE    (4 * 1024)

/* Separate server/port/thread for "/stream.jpg": its request callback
 * loops for as long as the client stays connected, which would otherwise
 * tie up HttpServer's one internal thread and block "/hello" and
 * "/image.jpg" for everyone else - same reason ST's own
 * b-u585i_iot02a_camera_demo reference used two servers. Shares
 * HttpServerPool (also the reference's choice) rather than allocating a
 * second multi-ten-KB packet pool. */
#define STREAM_SERVER_PORT               (81)
#define STREAM_SERVER_THREAD_STACK_SIZE  (4 * 1024)

#define IP_THREAD_STACK_SIZE             (2 * 1024)
#define IP_THREAD_PRIORITY               (1)

#define NETX_MAIN_THREAD_STACK_SIZE      (2 * 1024)
#define NETX_MAIN_THREAD_PRIORITY        (8)

#define ARP_CACHE_SIZE                   (1024)

static NX_PACKET_POOL AppPool;
static NX_PACKET_POOL HttpServerPool;
static NX_IP IpInstance;
static NX_DHCP DhcpClient;
static NX_WEB_HTTP_SERVER HttpServer;
static NX_WEB_HTTP_SERVER StreamServer;

static TX_THREAD NetxMainThread;
static TX_SEMAPHORE IpAddressSemaphore;

static VOID netx_main_thread_entry(ULONG thread_input);
static VOID ip_address_change_notify(NX_IP *ip_instance, VOID *ptr);

UINT Network_Service_Init(TX_BYTE_POOL *byte_pool) {

    UINT ret;
    VOID *pointer;

    nx_system_initialize();

    /* Main packet pool: used by the IP instance, DHCP client and the WiFi driver. */
    if (tx_byte_allocate(byte_pool, &pointer, NX_APP_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
        return NX_NOT_ENABLED;
    }
    ret = nx_packet_pool_create(&AppPool, "NetX Main Packet Pool", NX_APP_PACKET_PAYLOAD_SIZE, pointer,
                                 NX_APP_PACKET_POOL_SIZE);
    if (ret != NX_SUCCESS) {
        printf("nx_packet_pool_create (main) failed: 0x%02x\r\n", ret);
        return ret;
    }

    /* IP instance, bound to the EMW3080 WiFi driver (Platform/MX_CHIP + Application/WiFi_Bus). */
    if (tx_byte_allocate(byte_pool, &pointer, IP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
        return NX_NOT_ENABLED;
    }
    ret = nx_ip_create(&IpInstance, "Main IP Instance", NX_NULL, NX_NULL, &AppPool, nx_driver_emw3080_entry,
                        pointer, IP_THREAD_STACK_SIZE, IP_THREAD_PRIORITY);
    if (ret != NX_SUCCESS) {
        printf("nx_ip_create failed: 0x%02x\r\n", ret);
        return ret;
    }

    if (tx_byte_allocate(byte_pool, &pointer, ARP_CACHE_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
        return NX_NOT_ENABLED;
    }
    ret = nx_arp_enable(&IpInstance, pointer, ARP_CACHE_SIZE);
    if (ret != NX_SUCCESS) {
        printf("nx_arp_enable failed: 0x%02x\r\n", ret);
        return ret;
    }

    ret = nx_icmp_enable(&IpInstance);
    if (ret != NX_SUCCESS) {
        printf("nx_icmp_enable failed: 0x%02x\r\n", ret);
        return ret;
    }

    ret = nx_udp_enable(&IpInstance);
    if (ret != NX_SUCCESS) {
        printf("nx_udp_enable failed: 0x%02x\r\n", ret);
        return ret;
    }

    ret = nx_tcp_enable(&IpInstance);
    if (ret != NX_SUCCESS) {
        printf("nx_tcp_enable failed: 0x%02x\r\n", ret);
        return ret;
    }

    /* DHCP client: obtains the IP address once the WiFi driver associates. */
    ret = nx_dhcp_create(&DhcpClient, &IpInstance, "DHCP Client");
    if (ret != NX_SUCCESS) {
        printf("nx_dhcp_create failed: 0x%02x\r\n", ret);
        return ret;
    }

    ret = nx_ip_address_change_notify(&IpInstance, ip_address_change_notify, NX_NULL);
    if (ret != NX_SUCCESS) {
        printf("nx_ip_address_change_notify failed: 0x%02x\r\n", ret);
        return ret;
    }

    ret = tx_semaphore_create(&IpAddressSemaphore, "IP Address Semaphore", 0);
    if (ret != TX_SUCCESS) {
        printf("tx_semaphore_create failed: 0x%02x\r\n", ret);
        return ret;
    }

    /* HTTP server: no FileX/media behind it - http_server_request_notify() answers
       every request itself and always completes it, so media_ptr can be NX_NULL. */
    if (tx_byte_allocate(byte_pool, &pointer, HTTP_SERVER_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
        return NX_NOT_ENABLED;
    }
    ret = nx_packet_pool_create(&HttpServerPool, "HTTP Server Packet Pool", NX_APP_PACKET_PAYLOAD_SIZE, pointer,
                                 HTTP_SERVER_PACKET_POOL_SIZE);
    if (ret != NX_SUCCESS) {
        printf("nx_packet_pool_create (http) failed: 0x%02x\r\n", ret);
        return ret;
    }

    if (tx_byte_allocate(byte_pool, &pointer, HTTP_SERVER_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
        return NX_NOT_ENABLED;
    }
    ret = nx_web_http_server_create(&HttpServer, "HTTP Server", &IpInstance, HTTP_SERVER_PORT, NX_NULL,
                                     pointer, HTTP_SERVER_THREAD_STACK_SIZE, &HttpServerPool, NX_NULL,
                                     Http_Server_Request_Notify);
    if (ret != NX_SUCCESS) {
        printf("nx_web_http_server_create failed: 0x%02x\r\n", ret);
        return ret;
    }

    if (tx_byte_allocate(byte_pool, &pointer, STREAM_SERVER_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
        return NX_NOT_ENABLED;
    }
    ret = nx_web_http_server_create(&StreamServer, "Stream Server", &IpInstance, STREAM_SERVER_PORT, NX_NULL,
                                     pointer, STREAM_SERVER_THREAD_STACK_SIZE, &HttpServerPool, NX_NULL,
                                     Stream_Server_Request_Notify);
    if (ret != NX_SUCCESS) {
        printf("nx_web_http_server_create (stream) failed: 0x%02x\r\n", ret);
        return ret;
    }

    /* Application thread: drives DHCP then starts the HTTP server once an
       address has been assigned. */
    if (tx_byte_allocate(byte_pool, &pointer, NETX_MAIN_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS) {
        return NX_NOT_ENABLED;
    }
    ret = tx_thread_create(&NetxMainThread, "NetX Main Thread", netx_main_thread_entry, 0,
                            pointer, NETX_MAIN_THREAD_STACK_SIZE,
                            NETX_MAIN_THREAD_PRIORITY, NETX_MAIN_THREAD_PRIORITY,
                            TX_NO_TIME_SLICE, TX_AUTO_START);
    if (ret != TX_SUCCESS) {
        printf("tx_thread_create (NetX Main) failed: 0x%02x\r\n", ret);
        return ret;
    }

    return ret;
}

/**
  * Invoked by NetXDuo once the IP instance has an address (DHCP bound, or
  * later re-bound). Releases the main thread so it can start the HTTP
  * server exactly once an address is available.
  */
static VOID ip_address_change_notify(NX_IP *ip_instance, VOID *ptr) {

    NX_PARAMETER_NOT_USED(ip_instance);
    NX_PARAMETER_NOT_USED(ptr);

    tx_semaphore_put(&IpAddressSemaphore);
}

/**
  * Application thread for the NetXDuo/WiFi stack: starts DHCP, waits for
  * an IP address, then starts the HTTP server.
  */
static VOID netx_main_thread_entry(ULONG thread_input) {

    UINT status;
    ULONG ip_address = 0;
    ULONG network_mask = 0;

    NX_PARAMETER_NOT_USED(thread_input);

    printf("Starting DHCP client...\r\n");
    status = nx_dhcp_start(&DhcpClient);
    if (status != NX_SUCCESS) {
        printf("nx_dhcp_start failed: 0x%02x\r\n", status);
    }

    if (tx_semaphore_get(&IpAddressSemaphore, TX_WAIT_FOREVER) != TX_SUCCESS) {
        Error_Handler();
    }

    nx_ip_address_get(&IpInstance, &ip_address, &network_mask);
    printf("IP address: %lu.%lu.%lu.%lu\r\n",
           (ip_address >> 24) & 0xFF, (ip_address >> 16) & 0xFF,
           (ip_address >> 8) & 0xFF, ip_address & 0xFF);

    status = nx_web_http_server_start(&HttpServer);
    if (status != NX_SUCCESS) {
        printf("nx_web_http_server_start failed: 0x%02x\r\n", status);
    } else {
        printf("HTTP server listening on port %d (try http://<ip address>/hello or /image.jpg)\r\n", HTTP_SERVER_PORT);
    }

    status = nx_web_http_server_start(&StreamServer);
    if (status != NX_SUCCESS) {
        printf("nx_web_http_server_start (stream) failed: 0x%02x\r\n", status);
    } else {
        printf("Stream server listening on port %d (try http://<ip address>:%d/stream.jpg)\r\n",
               STREAM_SERVER_PORT, STREAM_SERVER_PORT);
    }
}
