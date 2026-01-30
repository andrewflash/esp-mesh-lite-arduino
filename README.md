# ESP-Mesh-Lite Arduino Library

Arduino/PlatformIO compatible wrapper for Espressif's [ESP-Mesh-Lite](https://github.com/espressif/esp-mesh-lite) library.

## Features

- Self-organizing mesh network for ESP32 devices
- Arduino-friendly C++ wrapper class (`MeshLite`)
- Supports ESP32, ESP32-S2, ESP32-S3, ESP32-C2, ESP32-C3, ESP32-C6
- Self-contained: pre-compiled libraries included (no submodules)
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

| Method | Description |
|--------|-------------|
| `begin()` | Initialize mesh with default config |
| `start()` | Start mesh networking |
| `stop()` | Stop and disconnect |
| `setRouterCredentials(ssid, pass)` | Set external WiFi credentials |
| `setSoftAPCredentials(ssid, pass)` | Set mesh node SoftAP credentials |
| `setMeshId(id)` | Set mesh network ID |
| `getMeshId()` | Get current mesh ID |
| `getLevel()` | Get node level (1=root) |
| `isRoot()` | Check if this is root node |
| `isLeafNode()` | Check if this is a leaf node |
| `setLeafNode(enable)` | Set node as leaf (no children) |
| `getNodeCount()` | Get total nodes in mesh |
| `getRootIP(buf, len)` | Get root node IP address |
| `sendToRoot(payload)` | Send JSON message to root |
| `sendToParent(payload)` | Send JSON message to parent |
| `broadcastToChildren(payload)` | Broadcast JSON to children |
| `onEvent(callback)` | Register event callback |
| `onMessage(type, resp, callback)` | Register message handler |
| `allowJoining(allow)` | Enable/disable new node joining |
| `scan(timeoutMs)` | Trigger WiFi scan |

## Project Structure

```
esp-mesh-lite-arduino/
├── library.json            # PlatformIO library manifest
├── library.properties      # Arduino IDE library manifest
├── extra_script.py         # Build script for chip-specific linking
├── keywords.txt            # Arduino IDE syntax highlighting
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
