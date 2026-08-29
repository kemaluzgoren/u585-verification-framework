/**
 * @file    http_pages.c
 * @brief   Static HTML/text content for the dashboard - see http_pages.h.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-29
 *
 * SPDX-License-Identifier: MIT
 */

#include "http_pages.h"

#include "network_service.h"

/* Stringifies a macro's expanded value (STREAM_SERVER_PORT -> "81") so it
 * can be spliced into a string-literal-concatenated page below - the
 * extra indirection is needed because #x stringifies the raw token "x",
 * not whatever it expands to. */
#define STR(x)  #x
#define XSTR(x) STR(x)

const char IndexPage[] =
    "<!DOCTYPE html><html><head><title>B-U585I-IOT02A Dashboard</title></head><body>"
    "<h1>B-U585I-IOT02A Dashboard</h1>"
    "<ul><li><a href=\"/camera.html\">Camera</a></li><li><a href=\"/sensors.html\">Sensors</a></li></ul>"
    "</body></html>";

const char CameraPage[] =
    "<!DOCTYPE html><html><head><title>Camera</title></head><body>"
    "<p><a href=\"/\">Home</a></p><h1>Camera</h1>"
    "<img id=\"s\" alt=\"camera stream\" style=\"max-width:100%\">"
    "<script>document.getElementById('s').src='http://'+location.hostname+':" XSTR(STREAM_SERVER_PORT) "/stream.jpg';"
    "</script>"
    "</body></html>";

const char SensorsPage[] =
    "<!DOCTYPE html><html><head><title>Sensors</title></head><body>"
    "<p><a href=\"/\">Home</a></p><h1>Sensors</h1>"
    "<pre id=\"d\">loading...</pre>"
    "<script>"
    "function p(){fetch('/sensors.json').then(function(r){return r.json();})"
    ".then(function(d){document.getElementById('d').textContent=JSON.stringify(d,null,2);});}"
    "p();setInterval(p,1000);"
    "</script></body></html>";

const char HelloBody[] = "u585-verification-framework is alive\r\n";
