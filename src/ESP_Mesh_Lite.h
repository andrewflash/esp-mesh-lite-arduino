/*
 * SPDX-FileCopyrightText: 2024 ESP-Mesh-Lite Arduino Port
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <Arduino.h>
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
#include "core/esp_mesh_lite.h"
#include "core/esp_bridge.h"
}

typedef void (*MeshLiteEventCallback)(esp_event_base_t event_base, int32_t event_id, void* event_data);
typedef cJSON* (*MeshLiteMessageCallback)(cJSON* payload, uint32_t seq);

class MeshLite {
public:
    MeshLite();
    ~MeshLite();

    // Initialization
    bool begin();
    bool begin(esp_mesh_lite_config_t& config);
    void start();
    bool stop();

    // Configuration
    bool setRouterCredentials(const char* ssid, const char* password);
    bool setSoftAPCredentials(const char* ssid, const char* password);
    void setMeshId(uint8_t meshId, bool forceUpdateNVS = false);
    bool setMaxLevel(uint8_t level);
    bool setLeafNode(bool enable);
    bool allowJoining(bool allow);

    // Status
    uint8_t getMeshId();
    uint8_t getLevel();
    bool isRoot();
    bool isLeafNode();
    uint32_t getNodeCount();
    const char* getDeviceCategory();
    bool getRootIP(char* ip, size_t len);

    // Communication
    bool sendToRoot(const char* payload);
    bool sendToParent(const char* payload);
    bool broadcastToChildren(const char* payload);

    // Events
    void onEvent(MeshLiteEventCallback callback);
    bool onMessage(const char* messageType, const char* responseType, MeshLiteMessageCallback callback);

    // Scanning
    bool scan(uint32_t timeoutMs = 3000);

    // Raw config access
    esp_mesh_lite_config_t* getConfig();

private:
    esp_mesh_lite_config_t _config;
    bool _initialized;
    bool _started;
    MeshLiteEventCallback _eventCallback;

    char _routerSsid[33];
    char _routerPassword[65];
    char _softapSsid[33];
    char _softapPassword[65];

    bool initNVS();
    bool initNetif();
    bool initWiFi();

    static void eventHandler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
};

extern MeshLite MeshLiteInstance;
