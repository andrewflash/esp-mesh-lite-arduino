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

// WiFi protocol modes (can be OR'd together)
// Note: LR mode is ESP32-specific and provides extended range but lower data rate
// LR mode only works between ESP32 devices, not with standard WiFi routers
enum MeshLiteWiFiProtocol : uint8_t {
    MESH_WIFI_PROTOCOL_11B   = WIFI_PROTOCOL_11B,       // 802.11b
    MESH_WIFI_PROTOCOL_11G   = WIFI_PROTOCOL_11G,       // 802.11g
    MESH_WIFI_PROTOCOL_11N   = WIFI_PROTOCOL_11N,       // 802.11n
    MESH_WIFI_PROTOCOL_LR    = WIFI_PROTOCOL_LR,        // ESP32 Long Range mode
    MESH_WIFI_PROTOCOL_BGN   = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N,
    MESH_WIFI_PROTOCOL_BGNLR = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR,
    MESH_WIFI_PROTOCOL_LR_ONLY = WIFI_PROTOCOL_LR,      // LR only (mesh-only, no router)
};

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
    bool setNetworkingMode(bool routerFirst, int8_t rssiThreshold = -75);
    bool setFusionConfig(uint32_t startTimeSec = 0, uint32_t frequencySec = 60);
    void setReconnectInterval(uint32_t parentInterval, uint32_t parentCount, uint32_t scanInterval);
    void setSoftAPHidden(bool hidden);
    void setWiFiProtocol(uint8_t staProtocol, uint8_t softapProtocol);

    // Status
    uint8_t getMeshId();
    uint8_t getLevel();
    bool isRoot();
    bool isLeafNode();
    uint32_t getNodeCount();
    const char* getDeviceCategory();
    bool getRootIP(char* ip, size_t len);

    // Communication (raw)
    bool sendToRoot(const char* payload);
    bool sendToParent(const char* payload);
    bool broadcastToChildren(const char* payload);

    // Communication (typed messages - use with onMessage handlers)
    bool sendTypedToRoot(const char* msgType, const char* respType, cJSON* payload, uint8_t maxRetry = 1);
    bool sendTypedToChildren(const char* msgType, const char* respType, cJSON* payload, uint8_t maxRetry = 1);

    // Events
    void onEvent(MeshLiteEventCallback callback);
    bool onMessage(const char* messageType, const char* responseType, MeshLiteMessageCallback callback);

    // Scanning
    bool scan(uint32_t timeoutMs = 3000);

    // Raw config access
    esp_mesh_lite_config_t* getConfig();

private:
    static const uint8_t MAX_MSG_ACTIONS = 4;

    esp_mesh_lite_config_t _config;
    bool _initialized;
    bool _started;
    MeshLiteEventCallback _eventCallback;
    esp_mesh_lite_msg_action_t _msgActions[MAX_MSG_ACTIONS];
    uint8_t _msgActionCount;

    char _routerSsid[33];
    char _routerPassword[65];
    char _softapSsid[33];
    char _softapPassword[65];
    bool _softapHidden;
    uint8_t _staProtocol;
    uint8_t _softapProtocol;

    bool initNVS();
    bool initNetif();
    bool initWiFi();

    static void eventHandler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
};

extern MeshLite MeshLiteInstance;
