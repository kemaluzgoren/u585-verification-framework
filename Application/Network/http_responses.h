/**
 * @file    http_responses.h
 * @brief   NX Web HTTP server request callbacks ("/hello", "/image.jpg",
 *          "/stream.jpg").
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-24
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef HTTP_RESPONSES_H
#define HTTP_RESPONSES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "tx_api.h"
#include "nx_web_http_server.h"

/* Interface this file needs from whatever produces images, defined here
 * (the consumer) rather than by the producer - this file has no idea
 * Application/Camera/camera_service.c exists, only that *something* can
 * be registered that behaves like this. timeout == TX_NO_WAIT means "hand
 * back whatever's current right now, don't wait for a new one" (used for
 * "/image.jpg"); any other value means "block up to timeout ticks for a
 * genuinely new frame" (used for "/stream.jpg"'s multipart loop). Returns
 * 0 (nothing copied) or 1 (dest[0..*len) now holds a frame), matching
 * Camera_Service_CopyLatestFrame()/Camera_Service_ProvideFrame(). */
typedef int32_t (*Http_Image_Provider_t)(uint8_t *dest, uint32_t dest_capacity, uint32_t *len, ULONG timeout);

/* Wires an image provider in for "/image.jpg" and "/stream.jpg" to call.
 * Until this is called (or if called with NULL), both resources answer
 * 404 - matches this file's behavior before an image feature existed at
 * all. Called once from Application/System/system.c after camera bring-up
 * succeeds. */
void Http_Responses_RegisterImageProvider(Http_Image_Provider_t provider);

/* Request callback for the main HTTP server (Application/Network/network_service.c's
 * HttpServer, port 80): answers "/hello" and "/image.jpg", 404s anything
 * else. Passed straight to nx_web_http_server_create(). */
UINT Http_Server_Request_Notify(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource,
                                 NX_PACKET *packet_ptr);

/* Request callback for the streaming HTTP server (network_service.c's
 * StreamServer, port 81): answers "/stream.jpg" with an MJPEG
 * multipart/x-mixed-replace stream for as long as the client stays
 * connected, 404s anything else. Passed straight to
 * nx_web_http_server_create(). */
UINT Stream_Server_Request_Notify(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource,
                                   NX_PACKET *packet_ptr);

#ifdef __cplusplus
}
#endif

#endif /* HTTP_RESPONSES_H */
