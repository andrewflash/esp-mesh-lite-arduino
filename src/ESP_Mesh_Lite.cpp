/*
 * SPDX-FileCopyrightText: 2024 ESP-Mesh-Lite Arduino Port
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ESP_Mesh_Lite.h"

// Global instance
MeshLite MeshLiteInstance;

MeshLite::MeshLite()
    : _initialized(false)
    , _started(false)
    , _eventCallback(nullptr)
    , _msgActionCount(0)
    , _staProtocol(MESH_WIFI_PROTOCOL_BGNLR)    // STA: support all modes for router compatibility
    , _softapProtocol(MESH_WIFI_PROTOCOL_BGNLR) // SoftAP: support all modes including LR for mesh
{
    // Initialize configuration with defaults
    memset(&_config, 0, sizeof(_config));
    memset(_msgActions, 0, sizeof(_msgActions));
    _config.vendor_id[0] = CONFIG_MESH_LITE_VENDOR_ID_0;
    _config.vendor_id[1] = CONFIG_MESH_LITE_VENDOR_ID_1;
    _config.mesh_id = CONFIG_MESH_LITE_ID;
    _config.max_connect_number = CONFIG_BRIDGE_SOFTAP_MAX_CONNECT_NUMBER;
    _config.max_router_number = CONFIG_MESH_LITE_MAX_ROUTER_NUMBER;
    _config.max_level = CONFIG_MESH_LITE_MAXIMUM_LEVEL_ALLOWED;
    _config.max_node_number = CONFIG_MESH_LITE_MAXIMUM_NODE_NUMBER;
    _config.join_mesh_ignore_router_status = CONFIG_JOIN_MESH_IGNORE_ROUTER_STATUS;
    _config.join_mesh_without_configured_wifi = CONFIG_JOIN_MESH_WITHOUT_CONFIGURED_WIFI_INFO;
    _config.leaf_node = CONFIG_LEAF_NODE;
    _config.ota_data_len = CONFIG_OTA_DATA_LEN;
    _config.ota_wnd = CONFIG_OTA_WND_DEFAULT;

    // Initialize credential buffers
    memset(_routerSsid, 0, sizeof(_routerSsid));
    memset(_routerPassword, 0, sizeof(_routerPassword));
    strncpy(_softapSsid, CONFIG_BRIDGE_SOFTAP_SSID, sizeof(_softapSsid) - 1);
    strncpy(_softapPassword, CONFIG_BRIDGE_SOFTAP_PASSWORD, sizeof(_softapPassword) - 1);

    _config.softap_ssid = _softapSsid;
    _config.softap_password = _softapPassword;
    _config.device_category = CONFIG_DEVICE_CATEGORY;
}

MeshLite::~MeshLite()
{
    if (_started) {
        stop();
    }
}

bool MeshLite::initNVS()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret == ESP_OK;
}

bool MeshLite::initNetif()
{
    return esp_netif_init() == ESP_OK;
}

bool MeshLite::initWiFi()
{
    esp_err_t ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        return false;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK) {
        return false;
    }

    esp_bridge_create_all_netif();

    return esp_wifi_set_mode(WIFI_MODE_APSTA) == ESP_OK;
}

void MeshLite::eventHandler(void* arg, esp_event_base_t event_base,
                           int32_t event_id, void* event_data)
{
    MeshLite* self = static_cast<MeshLite*>(arg);
    if (self && self->_eventCallback) {
        self->_eventCallback(event_base, event_id, event_data);
    }
}

bool MeshLite::begin()
{
    return begin(_config);
}

bool MeshLite::begin(esp_mesh_lite_config_t& config)
{
    if (_initialized) {
        return true;
    }

    if (!initNVS() || !initNetif() || !initWiFi()) {
        return false;
    }

    memcpy(&_config, &config, sizeof(esp_mesh_lite_config_t));
    _config.softap_ssid = _softapSsid;
    _config.softap_password = _softapPassword;
    _config.device_category = CONFIG_DEVICE_CATEGORY;

    esp_mesh_lite_init(&_config);

    if (strlen(_routerSsid) > 0) {
        mesh_lite_sta_config_t sta_config = {};
        strncpy((char*)sta_config.ssid, _routerSsid, sizeof(sta_config.ssid) - 1);
        strncpy((char*)sta_config.password, _routerPassword, sizeof(sta_config.password) - 1);
        esp_mesh_lite_set_router_config(&sta_config);
    }

    if (_eventCallback) {
        esp_event_handler_register(ESP_MESH_LITE_EVENT, ESP_EVENT_ANY_ID, eventHandler, this);
    }

    _initialized = true;
    return true;
}

void MeshLite::start()
{
    if (!_initialized || _started) {
        return;
    }

    esp_wifi_start();

    // Configure WiFi protocols for STA and SoftAP
    // STA: typically B/G/N/LR to support both routers (B/G/N) and mesh parents (LR)
    // SoftAP: can be LR-only for extended mesh range, or B/G/N/LR for compatibility
    esp_wifi_set_protocol(WIFI_IF_STA, _staProtocol);
    esp_wifi_set_protocol(WIFI_IF_AP, _softapProtocol);

    // Configure SoftAP password and hidden SSID
    wifi_config_t ap_config = {};
    esp_wifi_get_config(WIFI_IF_AP, &ap_config);

    if (strlen(_softapPassword) > 0) {
        strncpy((char*)ap_config.ap.password, _softapPassword, sizeof(ap_config.ap.password) - 1);
        ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    }

    esp_wifi_set_config(WIFI_IF_AP, &ap_config);

    esp_mesh_lite_start();
    _started = true;
}

bool MeshLite::stop()
{
    if (!_started) {
        return true;
    }

    if (esp_mesh_lite_disconnect() != ESP_OK) {
        return false;
    }

    _started = false;
    return true;
}

bool MeshLite::setRouterCredentials(const char* ssid, const char* password)
{
    if (!ssid) {
        return false;
    }

    strncpy(_routerSsid, ssid, sizeof(_routerSsid) - 1);
    _routerSsid[sizeof(_routerSsid) - 1] = '\0';

    if (password) {
        strncpy(_routerPassword, password, sizeof(_routerPassword) - 1);
        _routerPassword[sizeof(_routerPassword) - 1] = '\0';
    } else {
        _routerPassword[0] = '\0';
    }

    if (_initialized) {
        mesh_lite_sta_config_t sta_config = {};
        strncpy((char*)sta_config.ssid, _routerSsid, sizeof(sta_config.ssid) - 1);
        strncpy((char*)sta_config.password, _routerPassword, sizeof(sta_config.password) - 1);
        return esp_mesh_lite_set_router_config(&sta_config) == ESP_OK;
    }

    return true;
}

bool MeshLite::setSoftAPCredentials(const char* ssid, const char* password)
{
    if (!ssid) {
        return false;
    }

    strncpy(_softapSsid, ssid, sizeof(_softapSsid) - 1);
    _softapSsid[sizeof(_softapSsid) - 1] = '\0';

    if (password) {
        strncpy(_softapPassword, password, sizeof(_softapPassword) - 1);
        _softapPassword[sizeof(_softapPassword) - 1] = '\0';
    } else {
        _softapPassword[0] = '\0';
    }

    if (_initialized) {
        return esp_mesh_lite_set_softap_info(_softapSsid, _softapPassword) == ESP_OK;
    }

    return true;
}

void MeshLite::setMeshId(uint8_t meshId, bool forceUpdateNVS)
{
    _config.mesh_id = meshId;
    if (_initialized) {
        esp_mesh_lite_set_mesh_id(meshId, forceUpdateNVS);
    }
}

uint8_t MeshLite::getMeshId()
{
    if (_initialized) {
        return esp_mesh_lite_get_mesh_id();
    }
    return _config.mesh_id;
}

uint8_t MeshLite::getLevel()
{
    if (!_initialized || !_started) {
        return 0;
    }
    return esp_mesh_lite_get_level();
}

bool MeshLite::isRoot()
{
    return getLevel() == ROOT;
}

bool MeshLite::isLeafNode()
{
    if (!_initialized) {
        return _config.leaf_node;
    }
    return esp_mesh_lite_is_leaf_node();
}

bool MeshLite::setLeafNode(bool enable)
{
    _config.leaf_node = enable;
    if (_initialized) {
        return esp_mesh_lite_set_leaf_node(enable) == ESP_OK;
    }
    return true;
}

uint32_t MeshLite::getNodeCount()
{
#if CONFIG_MESH_LITE_NODE_INFO_REPORT
    if (_initialized && _started) {
        return esp_mesh_lite_get_mesh_node_number();
    }
#endif
    return 0;
}

const char* MeshLite::getDeviceCategory()
{
    if (_initialized) {
        return esp_mesh_lite_get_device_category();
    }
    return _config.device_category;
}

bool MeshLite::sendToRoot(const char* payload)
{
    if (!_initialized || !_started) {
        return false;
    }
    return esp_mesh_lite_send_msg_to_root(payload) == ESP_OK;
}

bool MeshLite::sendToParent(const char* payload)
{
    if (!_initialized || !_started) {
        return false;
    }
    return esp_mesh_lite_send_msg_to_parent(payload) == ESP_OK;
}

bool MeshLite::broadcastToChildren(const char* payload)
{
    if (!_initialized || !_started) {
        return false;
    }
    return esp_mesh_lite_send_broadcast_msg_to_child(payload) == ESP_OK;
}

bool MeshLite::sendTypedToRoot(const char* msgType, const char* respType, cJSON* payload, uint8_t maxRetry)
{
    if (!_initialized || !_started || !msgType || !payload) {
        return false;
    }

    esp_mesh_lite_msg_config_t config = {
        .json_msg = {
            .send_msg = msgType,
            .expect_msg = respType,
            .max_retry = maxRetry,
            .retry_interval = 1000,
            .req_payload = payload,
            .resend = esp_mesh_lite_send_msg_to_root,
            .send_fail = NULL
        }
    };

    return esp_mesh_lite_send_msg(ESP_MESH_LITE_JSON_MSG, &config) == ESP_OK;
}

bool MeshLite::sendTypedToChildren(const char* msgType, const char* respType, cJSON* payload, uint8_t maxRetry)
{
    if (!_initialized || !_started || !msgType || !payload) {
        return false;
    }

    esp_mesh_lite_msg_config_t config = {
        .json_msg = {
            .send_msg = msgType,
            .expect_msg = respType,
            .max_retry = maxRetry,
            .retry_interval = 1000,
            .req_payload = payload,
            .resend = esp_mesh_lite_send_broadcast_msg_to_child,
            .send_fail = NULL
        }
    };

    return esp_mesh_lite_send_msg(ESP_MESH_LITE_JSON_MSG, &config) == ESP_OK;
}

void MeshLite::onEvent(MeshLiteEventCallback callback)
{
    _eventCallback = callback;

    // If already initialized, register handler for mesh events
    if (_initialized && callback) {
        esp_event_handler_register(ESP_MESH_LITE_EVENT, ESP_EVENT_ANY_ID, eventHandler, this);
    }
}

bool MeshLite::onMessage(const char* messageType, const char* responseType, MeshLiteMessageCallback callback)
{
    if (!messageType || !callback) {
        return false;
    }

    // Check capacity
    if (_msgActionCount >= MAX_MSG_ACTIONS) {
        return false;
    }

    // Use persistent class member array - no dynamic allocation
    esp_mesh_lite_msg_action_t* action = &_msgActions[_msgActionCount];
    action->type = messageType;
    action->rsp_type = responseType;
    action->process = callback;

    if (esp_mesh_lite_msg_action_list_register(action) == ESP_OK) {
        _msgActionCount++;
        return true;
    }
    return false;
}

bool MeshLite::allowJoining(bool allow)
{
    if (!_initialized) {
        return false;
    }
    return esp_mesh_lite_allow_others_to_join(allow) == ESP_OK;
}

bool MeshLite::setNetworkingMode(bool routerFirst, int8_t rssiThreshold)
{
    esp_mesh_lite_networking_mode_t mode = routerFirst
        ? ESP_MESH_LITE_ROUTER
        : ESP_MESH_LITE_MESH;
    return esp_mesh_lite_set_networking_mode(mode, rssiThreshold) == ESP_OK;
}

bool MeshLite::setFusionConfig(uint32_t startTimeSec, uint32_t frequencySec)
{
    esp_mesh_lite_fusion_config_t config = {
        .fusion_rssi_threshold = -85,
        .fusion_start_time_sec = startTimeSec,
        .fusion_frequency_sec = frequencySec
    };
    return esp_mesh_lite_set_fusion_config(&config) == ESP_OK;
}

void MeshLite::setReconnectInterval(uint32_t parentInterval, uint32_t parentCount, uint32_t scanInterval)
{
    esp_mesh_lite_set_wifi_reconnect_interval(parentInterval, parentCount, scanInterval);
}

void MeshLite::setWiFiProtocol(uint8_t staProtocol, uint8_t softapProtocol)
{
    _staProtocol = staProtocol;
    _softapProtocol = softapProtocol;
}

bool MeshLite::setMaxLevel(uint8_t level)
{
    if (level < 1 || level > 15) {
        return false;
    }
    _config.max_level = level;
    return true;
}

bool MeshLite::getRootIP(char* ip, size_t len)
{
    if (!_initialized || !_started || !ip || len < 16) {
        return false;
    }

    esp_ip_addr_t ip_addr;
    if (esp_mesh_lite_get_root_ip(IPADDR_TYPE_V4, &ip_addr) != ESP_OK) {
        return false;
    }

    // Use ESP-IDF helper to convert IP to string (handles byte order)
    esp_ip4addr_ntoa(&ip_addr.u_addr.ip4, ip, len);
    return true;
}

uint8_t MeshLite::getConnectionProtocol()
{
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) != ESP_OK) {
        return 0;
    }

    // Build protocol bitmap from AP's supported/negotiated PHY modes
    uint8_t protocol = 0;
    if (ap_info.phy_11b) protocol |= WIFI_PROTOCOL_11B;
    if (ap_info.phy_11g) protocol |= WIFI_PROTOCOL_11G;
    if (ap_info.phy_11n) protocol |= WIFI_PROTOCOL_11N;
    if (ap_info.phy_lr)  protocol |= WIFI_PROTOCOL_LR;

    return protocol;
}

bool MeshLite::scan(uint32_t timeoutMs)
{
    if (!_initialized || !_started) {
        return false;
    }
    return esp_mesh_lite_wifi_scan_start(nullptr, timeoutMs / portTICK_PERIOD_MS) == ESP_OK;
}

esp_mesh_lite_config_t* MeshLite::getConfig()
{
    return &_config;
}
