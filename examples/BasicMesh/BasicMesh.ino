/*
 * BasicMesh.ino - ESP-Mesh-Lite Arduino Example
 *
 * This example demonstrates basic mesh networking setup using the
 * ESP-Mesh-Lite Arduino library. Nodes will automatically form a
 * self-organizing mesh network.
 *
 * Hardware: ESP32, ESP32-S2, ESP32-S3, ESP32-C2, ESP32-C3, or ESP32-C6
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ESP_Mesh_Lite.h>

// WiFi credentials for the router (root node connects to this)
#define ROUTER_SSID     "YOUR_WIFI_SSID"
#define ROUTER_PASSWORD "YOUR_WIFI_PASSWORD"

// Mesh network credentials (nodes use this to connect to each other)
#define MESH_SOFTAP_SSID     "ESP-Mesh-Lite"
#define MESH_SOFTAP_PASSWORD "mesh12345"

// Mesh configuration
#define MESH_ID         1    // Unique mesh network identifier (1-255)
#define MESH_MAX_LEVEL  6    // Maximum mesh depth/layers

// Event callback - called when mesh events occur
void meshEventCallback(esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == ESP_MESH_LITE_EVENT) {
        switch (event_id) {
            case ESP_MESH_LITE_EVENT_NODE_JOIN:
                Serial.println("[Mesh] A new node joined the network");
                break;
            case ESP_MESH_LITE_EVENT_NODE_LEAVE:
                Serial.println("[Mesh] A node left the network");
                break;
            default:
                Serial.printf("[Mesh] Event: %ld\n", event_id);
                break;
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("=================================");
    Serial.println(" ESP-Mesh-Lite Arduino Example");
    Serial.println("=================================");

    // Set router credentials (for root node to connect to internet)
    MeshLiteInstance.setRouterCredentials(ROUTER_SSID, ROUTER_PASSWORD);

    // Set SoftAP credentials (for mesh nodes to connect to each other)
    MeshLiteInstance.setSoftAPCredentials(MESH_SOFTAP_SSID, MESH_SOFTAP_PASSWORD);

    // Register event callback
    MeshLiteInstance.onEvent(meshEventCallback);

    // Initialize the mesh
    if (!MeshLiteInstance.begin()) {
        Serial.println("[Error] Failed to initialize mesh!");
        while (1) { delay(1000); }
    }

    // Configure mesh parameters
    MeshLiteInstance.setMeshId(MESH_ID);
    MeshLiteInstance.setMaxLevel(MESH_MAX_LEVEL);

    // Start the mesh network
    MeshLiteInstance.start();

    Serial.println("[Mesh] Started, waiting for network formation...");
}

void loop() {
    // Print mesh status every 10 seconds
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint >= 10000) {
        lastPrint = millis();

        Serial.println();
        Serial.println("--- Mesh Status ---");
        Serial.printf("  Level: %d\n", MeshLiteInstance.getLevel());
        Serial.printf("  Is Root: %s\n", MeshLiteInstance.isRoot() ? "Yes" : "No");
        Serial.printf("  Node Count: %lu\n", MeshLiteInstance.getNodeCount());
        Serial.printf("  Mesh ID: %d\n", MeshLiteInstance.getMeshId());

        if (MeshLiteInstance.isRoot()) {
            char ip[16];
            if (MeshLiteInstance.getRootIP(ip, sizeof(ip))) {
                Serial.printf("  Root IP: %s\n", ip);
            }
        }
        Serial.println("-------------------");
    }

    delay(100);
}
