/**
 * @file    http_responses.c
 * @brief   NX Web HTTP server request callbacks ("/", "/camera.html",
 *          "/sensors.html", "/sensors.json", "/image.jpg", "/stream.jpg").
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-24
 *
 * SPDX-License-Identifier: MIT
 */

#include "http_responses.h"

#include <stdio.h>
#include <string.h>

#include "http_pages.h"

/* Own policy constant, not Camera_Service's CAMERA_MAX_FRAME_SIZE - this
 * file has no dependency on camera_service.h at all, so it decides for
 * itself the largest image it is willing to buffer/serve. Sized the same
 * as CAMERA_MAX_FRAME_SIZE today because that is this project's only
 * image provider, but the two are free to diverge. */
#define HTTP_MAX_IMAGE_SIZE  (0xFFFCUL)

#define STREAM_BOUNDARY       "u585verificationframeworkboundary"
#define STREAM_CONTENT_TYPE   "multipart/x-mixed-replace;boundary=" STREAM_BOUNDARY

/* Own policy constant - a sensor snapshot is at most a few hundred bytes
 * (see Sensor_Service_ProvideJSON()'s doc comment), this is a generous
 * ceiling above that. */
#define HTTP_MAX_SENSOR_JSON_SIZE  (512UL)

/* One scratch buffer per server thread (HttpServer's vs. StreamServer's
 * own internal thread, both nx_web_http_server_create() spawns in
 * network_service.c) - sharing a single buffer between them would race if
 * a client hit "/image.jpg" while another was mid-stream on
 * "/stream.jpg". */
static uint8_t ImageFrameBuf[HTTP_MAX_IMAGE_SIZE];
static uint8_t StreamFrameBuf[HTTP_MAX_IMAGE_SIZE];
static char SensorJsonBuf[HTTP_MAX_SENSOR_JSON_SIZE];

static Http_Image_Provider_t image_provider = NULL;
static Http_Text_Provider_t sensor_provider = NULL;

void Http_Responses_RegisterImageProvider(Http_Image_Provider_t provider) {
    image_provider = provider;
}

void Http_Responses_RegisterSensorProvider(Http_Text_Provider_t provider) {
    sensor_provider = provider;
}

/**
  * HTTP server request callback. There is no FileX/media behind this
  * server, so every request must be answered here - matched resources
  * get a real response, anything else gets a 404. Resolves resource to a
  * body/content-type pair first, then does the (identical either way)
  * header-generate/append/send dance once - the static pages, the JSON
  * snapshot and the JPEG snapshot all end up going through the same tail.
  */
UINT Http_Server_Request_Notify(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource,
                                 NX_PACKET *packet_ptr) {

    NX_PACKET *response;
    UINT status;
    /* NX_WEB_HTTP_STATUS_OK/NOT_FOUND (nx_web_http_common.h) are the
       literal status-line strings ("200 OK", "404 Not Found"), not an
       integer code - nx_web_http_server_callback_generate_response_header()
       takes that string directly. */
    CHAR *status_code = NX_WEB_HTTP_STATUS_OK;
    CHAR *content_type = NX_NULL;
    const VOID *body_ptr = NX_NULL;
    uint32_t body_len = 0;

    NX_PARAMETER_NOT_USED(request_type);
    NX_PARAMETER_NOT_USED(packet_ptr);

    if (strcmp(resource, "/") == 0) {
        content_type = "text/html";
        body_ptr = IndexPage;
        /* Application/Network/http_pages.h declares these as incomplete
           array types ("extern const char X[];", no length), so sizeof()
           isn't available here - only http_pages.c, where they are
           actually defined, knows their size. strlen() works fine on any
           of these NUL-terminated strings regardless. */
        body_len = strlen(IndexPage);
    } else if (strcmp(resource, "/camera.html") == 0) {
        content_type = "text/html";
        body_ptr = CameraPage;
        body_len = strlen(CameraPage);
    } else if (strcmp(resource, "/sensors.html") == 0) {
        content_type = "text/html";
        body_ptr = SensorsPage;
        body_len = strlen(SensorsPage);
    } else if (strcmp(resource, "/hello") == 0) {
        content_type = "text/plain";
        body_ptr = HelloBody;
        body_len = strlen(HelloBody);
    } else if (strcmp(resource, "/sensors.json") == 0) {
        /* TX_NO_WAIT: give us whatever's current, don't wait for a new
           reading - Sensor_Service_ProvideJSON()'s doc comment. */
        if (sensor_provider != NULL &&
            sensor_provider(SensorJsonBuf, sizeof(SensorJsonBuf), &body_len, TX_NO_WAIT) != 0) {
            content_type = "application/json";
            body_ptr = SensorJsonBuf;
        } else {
            status_code = NX_WEB_HTTP_STATUS_NOT_FOUND;
        }
    } else if (strcmp(resource, "/image.jpg") == 0) {
        /* Copies out of the live capture buffer rather than borrowing a
           pointer into it, so the (possibly slow, over WiFi) send below
           never blocks the image provider's own capture thread - see
           Http_Image_Provider_t's doc comment. TX_NO_WAIT: give us
           whatever's current, don't wait for a new one. */
        if (image_provider != NULL &&
            image_provider(ImageFrameBuf, sizeof(ImageFrameBuf), &body_len, TX_NO_WAIT) != 0) {
            content_type = "image/jpeg";
            body_ptr = ImageFrameBuf;
        } else {
            /* No provider registered, or no frame captured yet (e.g. a
               request arrived before the first VSYNC) - fall through to
               404. */
            status_code = NX_WEB_HTTP_STATUS_NOT_FOUND;
        }
    } else {
        status_code = NX_WEB_HTTP_STATUS_NOT_FOUND;
    }

    /* body_ptr/body_len/content_type are only ever set together, in the
       success branches above - still NX_NULL/0/NX_NULL here for every
       404 case, exactly what a 404 response needs. */
    status = nx_web_http_server_callback_generate_response_header(server_ptr, &response, status_code, body_len,
                                                                    content_type, NX_NULL);

    if (status != NX_SUCCESS) {
        return NX_WEB_HTTP_CALLBACK_COMPLETED;
    }

    if (body_ptr != NX_NULL) {
        nx_packet_data_append(response, (VOID *)body_ptr, body_len,
                               server_ptr->nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
    }

    if (nx_web_http_server_callback_packet_send(server_ptr, response) != NX_SUCCESS) {
        nx_packet_release(response);
    }

    return NX_WEB_HTTP_CALLBACK_COMPLETED;
}

/**
  * "/stream.jpg" callback for StreamServer: sends the
  * multipart/x-mixed-replace header once, then loops for as long as the
  * client stays connected, waiting for the registered image provider to
  * signal each newly captured frame (TX_WAIT_FOREVER: block for a
  * genuinely new one) and sending it as one multipart part. Only ever
  * returns (completing the callback) once the client disconnects, the
  * connection errors, or no provider is registered - see
  * network_service.c's STREAM_SERVER_THREAD_STACK_SIZE comment for why
  * this lives on its own server/thread instead of HttpServer's.
  */
UINT Stream_Server_Request_Notify(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource,
                                   NX_PACKET *packet_ptr) {

    NX_PACKET *response;
    UINT status;

    NX_PARAMETER_NOT_USED(request_type);
    NX_PARAMETER_NOT_USED(packet_ptr);

    if (strcmp(resource, "/stream.jpg") != 0 || image_provider == NULL) {
        status = nx_web_http_server_callback_generate_response_header(server_ptr, &response,
                                                                        NX_WEB_HTTP_STATUS_NOT_FOUND, 0, NX_NULL,
                                                                        NX_NULL);
        if (status == NX_SUCCESS) {
            if (nx_web_http_server_callback_packet_send(server_ptr, response) != NX_SUCCESS) {
                nx_packet_release(response);
            }
        }
        return NX_WEB_HTTP_CALLBACK_COMPLETED;
    }

    status = nx_web_http_server_callback_generate_response_header(server_ptr, &response, NX_WEB_HTTP_STATUS_OK, 0,
                                                                    STREAM_CONTENT_TYPE, NX_NULL);
    if (status != NX_SUCCESS) {
        return NX_WEB_HTTP_CALLBACK_COMPLETED;
    }
    if (nx_web_http_server_callback_packet_send(server_ptr, response) != NX_SUCCESS) {
        return NX_WEB_HTTP_CALLBACK_COMPLETED;
    }

    while (1) {
        CHAR part_header[96];
        UINT part_header_len;
        uint32_t frame_len;

        /* Copies out of the live capture buffer rather than borrowing a
           pointer into it, so building/sending this part below - which
           can take a while if WiFi/TCP is the bottleneck - never blocks
           the image provider's own capture thread. Without this copy, a
           slow client visibly stalled the whole stream once the packet
           pool backed up. */
        if (image_provider(StreamFrameBuf, sizeof(StreamFrameBuf), &frame_len, TX_WAIT_FOREVER) == 0) {
            continue; /* raced ahead of the first captured frame - wait for the next one */
        }

        status = nx_web_http_server_response_packet_allocate(server_ptr, &response, NX_WAIT_FOREVER);
        if (status != NX_SUCCESS) {
            break;
        }

        part_header_len = (UINT)snprintf(part_header, sizeof(part_header),
                                          "\r\n--" STREAM_BOUNDARY "\r\nContent-Type: image/jpeg\r\nContent-Length: %lu\r\n\r\n",
                                          (unsigned long)frame_len);

        nx_packet_data_append(response, part_header, part_header_len,
                               server_ptr->nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
        nx_packet_data_append(response, StreamFrameBuf, frame_len,
                               server_ptr->nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        if (nx_web_http_server_callback_packet_send(server_ptr, response) != NX_SUCCESS) {
            nx_packet_release(response);
            break; /* client disconnected */
        }
    }

    return NX_WEB_HTTP_CALLBACK_COMPLETED;
}
