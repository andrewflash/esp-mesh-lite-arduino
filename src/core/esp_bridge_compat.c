/*
 * SPDX-FileCopyrightText: 2024 ESP-Mesh-Lite Arduino Port
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_bridge_compat.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include <string.h>

static esp_bridge_network_segment_check_cb_t s_segment_check_cb = NULL;
static esp_netif_t *s_sta_netif = NULL;
static esp_netif_t *s_ap_netif = NULL;

bool esp_bridge_network_segment_check_register(bool (*custom_check_cb)(uint32_t ip))
{
    if (custom_check_cb == NULL) {
        return false;
    }
    s_segment_check_cb = custom_check_cb;
    return true;
}

esp_err_t esp_bridge_netif_network_segment_conflict_update(esp_netif_t *esp_netif)
{
    return ESP_OK;
}

esp_netif_t *esp_bridge_create_station_netif(esp_netif_ip_info_t *ip_info,
                                              uint8_t mac[6],
                                              bool data_forwarding,
                                              bool enable_dhcps)
{
    if (s_sta_netif != NULL) {
        return s_sta_netif;
    }

    s_sta_netif = esp_netif_create_default_wifi_sta();
    if (s_sta_netif == NULL) {
        return NULL;
    }

    if (mac != NULL) {
        esp_netif_set_mac(s_sta_netif, mac);
    }

    if (ip_info != NULL) {
        esp_netif_dhcpc_stop(s_sta_netif);
        esp_netif_set_ip_info(s_sta_netif, ip_info);
    }

    return s_sta_netif;
}

esp_netif_t *esp_bridge_create_softap_netif(esp_netif_ip_info_t *ip_info,
                                             uint8_t mac[6],
                                             bool data_forwarding,
                                             bool enable_dhcps)
{
    if (s_ap_netif != NULL) {
        return s_ap_netif;
    }

    s_ap_netif = esp_netif_create_default_wifi_ap();
    if (s_ap_netif == NULL) {
        return NULL;
    }

    if (mac != NULL) {
        esp_netif_set_mac(s_ap_netif, mac);
    }

    if (ip_info != NULL) {
        esp_netif_dhcps_stop(s_ap_netif);
        esp_netif_set_ip_info(s_ap_netif, ip_info);
        if (enable_dhcps) {
            esp_netif_dhcps_start(s_ap_netif);
        }
    }

    return s_ap_netif;
}

esp_err_t esp_bridge_wifi_set(wifi_mode_t mode,
                               const char *ssid,
                               const char *password,
                               const char *bssid)
{
    esp_err_t ret = esp_wifi_set_mode(mode);
    if (ret != ESP_OK) {
        return ret;
    }

    if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) {
        wifi_config_t sta_config = {};
        if (ssid != NULL) {
            strncpy((char *)sta_config.sta.ssid, ssid, sizeof(sta_config.sta.ssid) - 1);
        }
        if (password != NULL) {
            strncpy((char *)sta_config.sta.password, password, sizeof(sta_config.sta.password) - 1);
        }
        if (bssid != NULL) {
            sta_config.sta.bssid_set = true;
            memcpy(sta_config.sta.bssid, bssid, 6);
        }
        ret = esp_wifi_set_config(WIFI_IF_STA, &sta_config);
    }

    return ret;
}

esp_err_t esp_bridge_wifi_set_config(wifi_interface_t interface, wifi_config_t *conf)
{
    if (conf == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    return esp_wifi_set_config(interface, conf);
}

void esp_bridge_create_all_netif(void)
{
    esp_bridge_create_station_netif(NULL, NULL, true, false);
    esp_bridge_create_softap_netif(NULL, NULL, true, true);
}

esp_err_t esp_bridge_update_dns_info(esp_netif_t *external_netif,
                                      esp_netif_t *data_forwarding_netif)
{
    if (external_netif == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_netif_dns_info_t dns_info;
    esp_err_t ret = esp_netif_get_dns_info(external_netif, ESP_NETIF_DNS_MAIN, &dns_info);
    if (ret != ESP_OK) {
        return ret;
    }

    if (data_forwarding_netif != NULL) {
        ret = esp_netif_set_dns_info(data_forwarding_netif, ESP_NETIF_DNS_MAIN, &dns_info);
    } else if (s_ap_netif != NULL) {
        ret = esp_netif_set_dns_info(s_ap_netif, ESP_NETIF_DNS_MAIN, &dns_info);
    }

    return ret;
}
