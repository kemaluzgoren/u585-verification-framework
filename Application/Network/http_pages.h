/**
 * @file    http_pages.h
 * @brief   Static HTML/text content for the dashboard served by
 *          Application/Network/http_responses.c. Kept in its own
 *          file/module so presentation content (what a page says) stays
 *          separate from HTTP protocol/routing logic (how a request gets
 *          answered) - http_responses.c only references these by name,
 *          never edits page markup inline.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-29
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef HTTP_PAGES_H
#define HTTP_PAGES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Home page ("/"): links to CameraPage and SensorsPage. */
extern const char IndexPage[];

/* "/camera.html": embeds StreamServer's MJPEG stream (network_service.h's
 * STREAM_SERVER_PORT) via a plain <img> tag - a browser renders
 * multipart/x-mixed-replace natively, no JS decoding needed, only a
 * one-line inline script to build the "host:port" the <img> src needs
 * (a plain relative URL cannot express a different port). */
extern const char CameraPage[];

/* "/sensors.html": polls "/sensors.json" (Application/Network/http_responses.c)
 * once a second via fetch() and renders it as formatted JSON text. */
extern const char SensorsPage[];

/* "/hello": plain-text liveness check body. */
extern const char HelloBody[];

#ifdef __cplusplus
}
#endif

#endif /* HTTP_PAGES_H */
