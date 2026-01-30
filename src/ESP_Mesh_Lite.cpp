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
{
    // Initialize configuration with defaults
    memset(&_config, 0, sizeof(_config));
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

    if (strlen(_softapPassword) > 0) {
        wifi_config_t ap_config = {};
        esp_wifi_get_config(WIFI_IF_AP, &ap_config);
        strncpy((char*)ap_config.ap.password, _softapPassword, sizeof(ap_config.ap.password) - 1);
        ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    }

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

    esp_mesh_lite_msg_action_t action = {
        .type = messageType,
        .rsp_type = responseType,
        .process = callback
    };

    return esp_mesh_lite_msg_action_list_register(&action) == ESP_OK;
}

bool MeshLite::allowJoining(bool allow)
{
    if (!_initialized) {
        return false;
    }
    return esp_mesh_lite_allow_others_to_join(allow) == ESP_OK;
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

    // Extract bytes directly from memory (lwIP stores in network byte order)
    uint8_t* bytes = (uint8_t*)&ip_addr.u_addr.ip4.addr;
    snprintf(ip, len, "%u.%u.%u.%u", bytes[0], bytes[1], bytes[2], bytes[3]);
    return true;
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
