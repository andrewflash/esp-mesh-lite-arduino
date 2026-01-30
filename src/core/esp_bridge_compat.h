/*
 * SPDX-FileCopyrightText: 2024 ESP-Mesh-Lite Arduino Port
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * @file esp_bridge_compat.h
 * @brief ESP-IoT-Bridge Compatibility Layer for Arduino
 *
 * This header provides compatibility definitions and stub implementations
 * for esp_bridge functions required by ESP-Mesh-Lite when running on Arduino.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_err.h"

// Configuration defines for bridge functionality
#ifndef CONFIG_BRIDGE_EXTERNAL_NETIF_STATION
#define CONFIG_BRIDGE_EXTERNAL_NETIF_STATION 1
#endif

#ifndef CONFIG_BRIDGE_DATA_FORWARDING_NETIF_SOFTAP
#define CONFIG_BRIDGE_DATA_FORWARDING_NETIF_SOFTAP 1
#endif

#ifndef CONFIG_BRIDGE_SOFTAP_MAX_CONNECT_NUMBER
#define CONFIG_BRIDGE_SOFTAP_MAX_CONNECT_NUMBER 6
#endif

/**
 * @brief Network segment check callback type
 */
typedef bool (*esp_bridge_network_segment_check_cb_t)(uint32_t ip);

/**
 * @brief Register a callback for network segment conflict checking
 *
 * @param custom_check_cb Callback function to check if IP is in use
 * @return true on success
 */
bool esp_bridge_network_segment_check_register(bool (*custom_check_cb)(uint32_t ip));

/**
 * @brief Update network interface when segment conflict is detected
 *
 * @param esp_netif Network interface to update (can be NULL)
 * @return ESP_OK on success
 */
esp_err_t esp_bridge_netif_network_segment_conflict_update(esp_netif_t *esp_netif);

/**
 * @brief Create station network interface for bridge
 *
 * @param ip_info Custom IP info (NULL for auto)
 * @param mac Custom MAC address (NULL for auto)
 * @param data_forwarding Enable data forwarding
 * @param enable_dhcps Enable DHCP server
 * @return Network interface handle
 */
esp_netif_t *esp_bridge_create_station_netif(esp_netif_ip_info_t *ip_info,
                                              uint8_t mac[6],
                                              bool data_forwarding,
                                              bool enable_dhcps);

/**
 * @brief Create SoftAP network interface for bridge
 *
 * @param ip_info Custom IP info (NULL for auto)
 * @param mac Custom MAC address (NULL for auto)
 * @param data_forwarding Enable data forwarding
 * @param enable_dhcps Enable DHCP server
 * @return Network interface handle
 */
esp_netif_t *esp_bridge_create_softap_netif(esp_netif_ip_info_t *ip_info,
                                             uint8_t mac[6],
                                             bool data_forwarding,
                                             bool enable_dhcps);

/**
 * @brief Set WiFi configuration for bridge
 *
 * @param mode WiFi mode
 * @param ssid SSID
 * @param password Password
 * @param bssid BSSID (can be NULL)
 * @return ESP_OK on success
 */
esp_err_t esp_bridge_wifi_set(wifi_mode_t mode,
                               const char *ssid,
                               const char *password,
                               const char *bssid);

/**
 * @brief Set WiFi config
 *
 * @param interface WiFi interface
 * @param conf WiFi configuration
 * @return ESP_OK on success
 */
esp_err_t esp_bridge_wifi_set_config(wifi_interface_t interface, wifi_config_t *conf);

/**
 * @brief Create all enabled network interfaces
 */
void esp_bridge_create_all_netif(void);

/**
 * @brief Update DNS information
 *
 * @param external_netif Source netif
 * @param data_forwarding_netif Target netif (NULL for all)
 * @return ESP_OK on success
 */
esp_err_t esp_bridge_update_dns_info(esp_netif_t *external_netif,
                                      esp_netif_t *data_forwarding_netif);

#ifdef __cplusplus
}
#endif
