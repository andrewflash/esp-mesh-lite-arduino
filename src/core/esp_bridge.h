/*
 * SPDX-FileCopyrightText: 2024 ESP-Mesh-Lite Arduino Port
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * @file esp_bridge.h
 * @brief ESP-IoT-Bridge Compatibility Header for Arduino
 *
 * This header redirects to the Arduino-compatible bridge implementation.
 * It provides the same interface as the ESP-IDF esp_bridge.h but with
 * simplified implementations suitable for Arduino/PlatformIO.
 */

// Include the Arduino-compatible implementation
#include "esp_bridge_compat.h"
