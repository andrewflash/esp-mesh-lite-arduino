/*
 * SPDX-FileCopyrightText: 2024 ESP-Mesh-Lite Arduino Port
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * @brief ESP-Mesh-Lite Arduino Configuration Header
 *
 * This header provides configuration options for ESP-Mesh-Lite when using Arduino framework.
 * These macros replace the ESP-IDF Kconfig system for Arduino compatibility.
 *
 * Override these values in platformio.ini build_flags or in your code before including
 * ESP_Mesh_Lite.h
 */

#ifdef __cplusplus
extern "C" {
#endif

// Enable/disable mesh-lite
#ifndef CONFIG_MESH_LITE_ENABLE
#define CONFIG_MESH_LITE_ENABLE 1
#endif

// Mesh Lite version
#ifndef MESH_LITE_VER_MAJOR
#define MESH_LITE_VER_MAJOR 1
#endif

#ifndef MESH_LITE_VER_MINOR
#define MESH_LITE_VER_MINOR 0
#endif

#ifndef MESH_LITE_VER_PATCH
#define MESH_LITE_VER_PATCH 0
#endif

// Vendor ID (2 bytes) - used for mesh network identification
#ifndef CONFIG_MESH_LITE_VENDOR_ID_0
#define CONFIG_MESH_LITE_VENDOR_ID_0 71
#endif

#ifndef CONFIG_MESH_LITE_VENDOR_ID_1
#define CONFIG_MESH_LITE_VENDOR_ID_1 87
#endif

// Mesh Network ID
#ifndef CONFIG_MESH_LITE_ID
#define CONFIG_MESH_LITE_ID 77
#endif

// Maximum number of router traces (1-5)
#ifndef CONFIG_MESH_LITE_MAX_ROUTER_NUMBER
#define CONFIG_MESH_LITE_MAX_ROUTER_NUMBER 3
#endif

// Maximum allowed network depth/level (1-15)
#ifndef CONFIG_MESH_LITE_MAXIMUM_LEVEL_ALLOWED
#define CONFIG_MESH_LITE_MAXIMUM_LEVEL_ALLOWED 5
#endif

// Maximum number of stations that can connect to SoftAP
#ifndef CONFIG_BRIDGE_SOFTAP_MAX_CONNECT_NUMBER
#define CONFIG_BRIDGE_SOFTAP_MAX_CONNECT_NUMBER 6
#endif

// SoftAP configuration
#ifndef CONFIG_BRIDGE_SOFTAP_SSID
#define CONFIG_BRIDGE_SOFTAP_SSID "ESP_Mesh_Lite"
#endif

#ifndef CONFIG_BRIDGE_SOFTAP_PASSWORD
#define CONFIG_BRIDGE_SOFTAP_PASSWORD "12345678"
#endif

// Device category identifier
#ifndef CONFIG_DEVICE_CATEGORY
#define CONFIG_DEVICE_CATEGORY "ESP32"
#endif

// Join mesh network behavior
#ifndef CONFIG_JOIN_MESH_WITHOUT_CONFIGURED_WIFI_INFO
#define CONFIG_JOIN_MESH_WITHOUT_CONFIGURED_WIFI_INFO 0
#endif

#ifndef CONFIG_JOIN_MESH_IGNORE_ROUTER_STATUS
#define CONFIG_JOIN_MESH_IGNORE_ROUTER_STATUS 0
#endif

// Leaf node configuration
#ifndef CONFIG_LEAF_NODE
#define CONFIG_LEAF_NODE 0
#endif

// Node information reporting
#ifndef CONFIG_MESH_LITE_NODE_INFO_REPORT
#define CONFIG_MESH_LITE_NODE_INFO_REPORT 0
#endif

#ifndef CONFIG_MESH_LITE_REPORT_INTERVAL
#define CONFIG_MESH_LITE_REPORT_INTERVAL 300
#endif

#ifndef CONFIG_MESH_LITE_MAXIMUM_NODE_NUMBER
#define CONFIG_MESH_LITE_MAXIMUM_NODE_NUMBER 50
#endif

// OTA configuration
#ifndef CONFIG_ESP_MESH_LITE_OTA_ENABLE
#define CONFIG_ESP_MESH_LITE_OTA_ENABLE 0
#endif

#ifndef CONFIG_OTA_DATA_LEN
#define CONFIG_OTA_DATA_LEN 1376
#endif

#ifndef CONFIG_OTA_WND_DEFAULT
#define CONFIG_OTA_WND_DEFAULT 8256
#endif

#ifdef __cplusplus
}
#endif
