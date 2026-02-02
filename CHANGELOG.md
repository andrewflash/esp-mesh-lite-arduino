# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.0] - 2025-02-02

### Added

- **WiFi Protocol Configuration**
  - `setWiFiProtocol(staProtocol, softapProtocol)` - Configure WiFi protocol modes for STA and SoftAP interfaces
  - Protocol constants: `MESH_WIFI_PROTOCOL_11B`, `_11G`, `_11N`, `_LR`, `_BGN`, `_BGNLR`, `_LR_ONLY`
  - Long Range (LR) mode support for extended mesh range between ESP32 devices

- **PHY Mode Query**
  - `getConnectionProtocol()` - Retrieve current WiFi connection protocol mode

### Removed

- **Hidden SSID Support**
  - Removed `setSoftAPHidden()` - ESP-Mesh-Lite's internal WiFi scan doesn't support hidden SSIDs (uses `show_hidden=false`)
  - Hidden SoftAP SSIDs prevent mesh nodes from discovering parents

## [1.1.0] - 2025-01-31

### Added

- **Multi-Root Topology Support**
  - `setNetworkingMode(routerFirst, rssiThreshold)` - Enable router-first mode where multiple nodes can connect directly to the router as independent roots
  - `setFusionConfig(startTimeSec, frequencySec)` - Configure topology optimization timing for periodic router reconnection attempts
  - `setReconnectInterval(parentInterval, parentCount, scanInterval)` - Configure mesh fallback behavior when router connection is lost

- **Typed Messaging System**
  - `sendTypedToRoot(msgType, respType, payload, maxRetry)` - Send typed messages to root with automatic retry and response matching
  - `sendTypedToChildren(msgType, respType, payload, maxRetry)` - Send typed messages to children with automatic retry
  - `onMessage(msgType, respType, callback)` - Register handlers for specific message types

### Changed

- Reorganized API documentation in README with categorized sections
- Added Multi-Root Topology section with ASCII diagrams explaining the feature

### Fixed

- Message handler registration now uses persistent storage to prevent crashes from stack-allocated callbacks

## [1.0.0] - 2025-01-15

### Added

- Initial release
- Arduino/PlatformIO wrapper for ESP-Mesh-Lite
- Support for ESP32, ESP32-S2, ESP32-S3, ESP32-C2, ESP32-C3, ESP32-C6
- Pre-compiled libraries for all supported chips
- Basic mesh networking functionality:
  - `begin()` / `start()` / `stop()` lifecycle management
  - `setRouterCredentials()` / `setSoftAPCredentials()` configuration
  - `setMeshId()` / `getMeshId()` mesh identification
  - `getLevel()` / `isRoot()` / `isLeafNode()` status queries
  - `sendToRoot()` / `sendToParent()` / `broadcastToChildren()` communication
  - `onEvent()` event handling
  - `allowJoining()` / `setLeafNode()` node configuration
- Event system with mesh lifecycle events
- ESP-NOW support for low-latency communication
- Comprehensive documentation and examples

[1.2.0]: https://github.com/andrewflash/esp-mesh-lite-arduino/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/andrewflash/esp-mesh-lite-arduino/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/andrewflash/esp-mesh-lite-arduino/releases/tag/v1.0.0
