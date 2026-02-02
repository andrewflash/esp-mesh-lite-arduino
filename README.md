# ESP-Mesh-Lite Arduino Library

[![Version](https://img.shields.io/badge/version-1.2.0-blue.svg)](CHANGELOG.md)
[![License](https://img.shields.io/badge/license-Apache--2.0-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32-orange.svg)](https://www.espressif.com)

Arduino/PlatformIO compatible wrapper for Espressif's [ESP-Mesh-Lite](https://github.com/espressif/esp-mesh-lite) library.

## Features

- Self-organizing mesh network for ESP32 devices
- Arduino-friendly C++ wrapper class (`MeshLite`)
- Supports ESP32, ESP32-S2, ESP32-S3, ESP32-C2, ESP32-C3, ESP32-C6
- Self-contained: pre-compiled libraries included (no submodules)
- **Multi-root topology** - Multiple nodes can connect directly to router
- **Typed messaging** - Request-response pattern with automatic retry
- ESP-NOW support for low-latency communication
- Works with PlatformIO and Arduino IDE

## Requirements

**Important:** ESP-Mesh-Lite requires **ESP-IDF v5.0 or higher**. The standard PlatformIO espressif32 platform only supports ESP-IDF v4.x due to licensing restrictions.

To use this library, you need one of:
- **PlatformIO** with [pioarduino](https://github.com/pioarduino/platform-espressif32) custom platform (ESP-IDF 5.x based)
- **Arduino IDE** with ESP32 board support 3.x+ (uses ESP-IDF 5.x)

## Installation

### PlatformIO (Recommended)

Use the pioarduino custom platform which provides ESP-IDF 5.x support:

```ini
[env]
; Use pioarduino platform for ESP-IDF 5.x support
platform = https://github.com/pioarduino/platform-espressif32/releases/download/stable/platform-espressif32.zip
framework = arduino
lib_extra_dirs = ../lib

[env:esp32s3]
board = esp32-s3-devkitc-1
```

> **Note:** The official `platform = espressif32` uses ESP-IDF 4.x and will NOT work with this library.

### Manual Installation

1. Download or clone this repository
2. Place it in your PlatformIO project's `lib/` folder

## Usage

### Basic Example

```cpp
#include <ESP_Mesh_Lite.h>

MeshLite mesh;

void meshEventCallback(esp_event_base_t event_base, int32_t event_id, void* event_data) {
    Serial.printf("[MESH] Event ID: %ld\n", event_id);
}

void setup() {
    Serial.begin(115200);

    // Set event callback (optional, before begin)
    mesh.onEvent(meshEventCallback);

    // Configure credentials
    mesh.setRouterCredentials("YourWiFi", "password");
    mesh.setSoftAPCredentials("ESP", "12345678");
    mesh.setMeshId(77);

    // Initialize and start
    mesh.begin();
    mesh.start();
}

void loop() {
    Serial.printf("Level: %d, Is Root: %s\n",
                  mesh.getLevel(),
                  mesh.isRoot() ? "Yes" : "No");
    delay(5000);
}
```

### Configuration

Add build flags in `platformio.ini`:

```ini
build_flags =
    -DCONFIG_MESH_LITE_ID=77
    -DCONFIG_MESH_LITE_MAXIMUM_LEVEL_ALLOWED=10
    -DCONFIG_BRIDGE_SOFTAP_SSID=\"ESP\"
    -DCONFIG_BRIDGE_SOFTAP_PASSWORD=\"12345678\"
```

## API Reference

### MeshLite Class

#### Initialization

| Method | Description |
|--------|-------------|
| `begin()` | Initialize mesh with default config |
| `start()` | Start mesh networking |
| `stop()` | Stop and disconnect |

#### Configuration

| Method | Description |
|--------|-------------|
| `setRouterCredentials(ssid, pass)` | Set external WiFi credentials |
| `setSoftAPCredentials(ssid, pass)` | Set mesh node SoftAP credentials |
| `setMeshId(id)` | Set mesh network ID |
| `setMaxLevel(level)` | Set maximum mesh depth |
| `setLeafNode(enable)` | Set node as leaf (no children) |
| `allowJoining(allow)` | Enable/disable new node joining |

#### Networking Mode (Multi-Root Support)

| Method | Description |
|--------|-------------|
| `setNetworkingMode(routerFirst, rssiThreshold)` | Enable router-first mode for multi-root topology |
| `setFusionConfig(startTimeSec, frequencySec)` | Configure topology optimization timing |
| `setReconnectInterval(parentInterval, parentCount, scanInterval)` | Configure reconnection behavior |

**Router-First Mode:** When enabled, each node tries to connect directly to the router first. Multiple nodes can be root simultaneously. Nodes only form mesh hierarchy when they can't reach the router.

```cpp
// Enable router-first mode with -75 dBm threshold
mesh.setNetworkingMode(true, -75);

// Start fusion after 30s, check every 60s
mesh.setFusionConfig(30, 60);

// Reconnect: 3s interval, 2 attempts, then scan every 5s
mesh.setReconnectInterval(3, 2, 5);
```

#### WiFi Protocol Modes

| Method | Description |
|--------|-------------|
| `setWiFiProtocol(sta, softap)` | Set WiFi protocol modes for STA and SoftAP |

ESP32 supports 802.11 b/g/n and a proprietary Long Range (LR) mode. LR provides extended range (~1km line-of-sight) but only works between ESP32 devices.

```cpp
// Available protocol constants (can be OR'd together):
// MESH_WIFI_PROTOCOL_11B    - 802.11b only
// MESH_WIFI_PROTOCOL_11G    - 802.11g only
// MESH_WIFI_PROTOCOL_11N    - 802.11n only
// MESH_WIFI_PROTOCOL_LR     - ESP32 Long Range only
// MESH_WIFI_PROTOCOL_BGN    - 802.11 b/g/n (standard)
// MESH_WIFI_PROTOCOL_BGNLR  - All modes including LR (default)

// STA: B/G/N/LR to connect to router (B/G/N) and mesh parents (LR)
// SoftAP: LR-only for maximum mesh range (ESP32 children only)
mesh.setWiFiProtocol(MESH_WIFI_PROTOCOL_BGNLR, MESH_WIFI_PROTOCOL_LR);
```

#### Status

| Method | Description |
|--------|-------------|
| `getMeshId()` | Get current mesh ID |
| `getLevel()` | Get node level (1=root) |
| `isRoot()` | Check if this is root node |
| `isLeafNode()` | Check if this is a leaf node |
| `getNodeCount()` | Get total nodes in mesh |
| `getRootIP(buf, len)` | Get root node IP address |
| `getConnectionProtocol()` | Get current WiFi PHY mode |

#### Communication (Raw)

| Method | Description |
|--------|-------------|
| `sendToRoot(payload)` | Send raw JSON message to root |
| `sendToParent(payload)` | Send raw JSON message to parent |
| `broadcastToChildren(payload)` | Broadcast raw JSON to children |

#### Communication (Typed Messages)

Typed messages allow request-response matching and automatic retry.

| Method | Description |
|--------|-------------|
| `sendTypedToRoot(msgType, respType, payload, maxRetry)` | Send typed message to root |
| `sendTypedToChildren(msgType, respType, payload, maxRetry)` | Send typed message to children |
| `onMessage(msgType, respType, callback)` | Register handler for typed messages |

**Example:**
```cpp
// Register handler for "status" messages
mesh.onMessage("status", "status_ack", [](cJSON* payload, uint32_t seq) -> cJSON* {
    // Process payload
    return nullptr;  // or return response
});

// Send typed message with retry
cJSON* data = cJSON_CreateObject();
cJSON_AddNumberToObject(data, "level", mesh.getLevel());
mesh.sendTypedToRoot("status", "status_ack", data);
cJSON_Delete(data);
```

#### Events

| Method | Description |
|--------|-------------|
| `onEvent(callback)` | Register event callback |
| `scan(timeoutMs)` | Trigger WiFi scan |

## Project Structure

```
esp-mesh-lite-arduino/
├── library.json            # PlatformIO library manifest
├── library.properties      # Arduino IDE library manifest
├── extra_script.py         # Build script for chip-specific linking
├── keywords.txt            # Arduino IDE syntax highlighting
├── CHANGELOG.md            # Version history
├── README.md
├── lib/                    # Pre-compiled libraries by chip
│   ├── esp32/
│   │   └── libesp_mesh_lite.a
│   ├── esp32c2/
│   │   └── libesp_mesh_lite.a
│   ├── esp32c3/
│   │   └── libesp_mesh_lite.a
│   ├── esp32c6/
│   │   └── libesp_mesh_lite.a
│   ├── esp32s2/
│   │   └── libesp_mesh_lite.a
│   └── esp32s3/
│       └── libesp_mesh_lite.a
└── src/
    ├── ESP_Mesh_Lite.cpp   # Arduino C++ wrapper implementation
    ├── ESP_Mesh_Lite.h     # Main Arduino wrapper header
    └── core/               # Core mesh-lite source files
        ├── esp_bridge.h
        ├── esp_bridge_compat.c
        ├── esp_bridge_compat.h
        ├── esp_mesh_lite.c
        ├── esp_mesh_lite.h
        ├── esp_mesh_lite_config.h
        ├── esp_mesh_lite_core.h
        ├── esp_mesh_lite_espnow.c
        ├── esp_mesh_lite_espnow.h
        ├── esp_mesh_lite_log.c
        ├── esp_mesh_lite_log.h
        ├── esp_mesh_lite_port.c
        ├── esp_mesh_lite_port.h
        ├── esp_mesh_lite_wireless_debug.h
        ├── mesh_lite.pb-c.c
        └── mesh_lite.pb-c.h
```

## Dependencies

- PlatformIO with pioarduino platform (ESP-IDF 5.x based), or Arduino IDE with ESP32 board support 3.x+
- Arduino framework for ESP32
- No external dependencies required (cJSON and ESP-NOW provided by ESP32 Arduino core)

## Multi-Root Topology

By default, ESP-Mesh-Lite forms a single-root tree where one node connects to the router and others connect through the mesh. With **router-first mode**, multiple nodes can independently connect to the same router:

```
Normal Mode (single root):        Router-First Mode (multi-root):
      [Router]                          [Router]
         │                             /   │   \
      [Root]                       [A]   [B]   [C]  ← All are root (level 1)
       /   \
    [A]     [B]

When router unavailable for C:
      [Router]
       /   \
    [A]   [B]  ← Both still root
           │
         [C]   ← Falls back to mesh (level 2)
```

**Enable with:**
```cpp
mesh.setNetworkingMode(true, -75);  // router-first, RSSI threshold
mesh.setFusionConfig(30, 60);       // Periodically try to reconnect to router
mesh.setReconnectInterval(3, 2, 5); // Fallback timing when router lost
```

## Mesh Events

The library provides these mesh events via the `onEvent()` callback:

| Event | Description |
|-------|-------------|
| `ESP_MESH_LITE_EVENT_CORE_STARTED` | Mesh core has started |
| `ESP_MESH_LITE_EVENT_CORE_INHERITED_NET_SEGMENT_CHANGED` | Network topology changed |
| `ESP_MESH_LITE_EVENT_CORE_ROUTER_INFO_CHANGED` | Router info updated |
| `ESP_MESH_LITE_EVENT_NODE_JOIN` | A node joined the mesh |
| `ESP_MESH_LITE_EVENT_NODE_LEAVE` | A node left the mesh |
| `ESP_MESH_LITE_EVENT_NODE_CHANGE` | Node info changed |

## Limitations

- Core mesh logic is in pre-compiled libraries from Espressif (compiled with ESP-IDF 5.x)
- **Requires ESP-IDF 5.x** - Standard PlatformIO `espressif32` platform uses ESP-IDF 4.x and will NOT work
- Some advanced features may not be available in Arduino port

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Linker errors (undefined references) | Use pioarduino platform, not standard `espressif32` |
| `esp_app_get_description` missing | You're using ESP-IDF 4.x - switch to pioarduino |
| Crash on node connect | Avoid calling `getNodeCount()` in tight loops during connection events |
| SoftAP password not working | The library applies password after WiFi starts |
| ESP-NOW errors | Ensure ESP-NOW is available in your Arduino core |

## License

Apache-2.0 (same as original ESP-Mesh-Lite)

## Credits

- [Espressif Systems](https://www.espressif.com) - Original ESP-Mesh-Lite library
