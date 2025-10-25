#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

#define MAX_WIFI_NETWORKS 3
#define WIFI_CONNECT_TIMEOUT_MS 15000

struct WiFiNetwork {
  char ssid[33];      // Max 32 chars + null terminator
  char password[64];  // Max 63 chars + null terminator
};

class WiFiManager {
private:
  Preferences preferences;
  WiFiNetwork networks[MAX_WIFI_NETWORKS];
  int lastSuccessfulIndex = -1;
  int currentAttemptIndex = 0;
  unsigned long lastConnectionAttempt = 0;
  bool isConnecting = false;
  bool initialized = false;

public:
  WiFiManager() {
    // Don't load in constructor - wait for init() to be called
    memset(networks, 0, sizeof(networks));
  }

  void init() {
    if (initialized) return;

    Serial.println("[WiFiManager] Initializing...");
    delay(100); // Give NVS time to initialize

    loadNetworks();
    loadLastSuccessful();
    initialized = true;

    Serial.println("[WiFiManager] Initialization complete!");
  }

  void loadNetworks() {
    if (!preferences.begin("wifi", true)) { // read-only
      Serial.println("[WiFiManager] ERROR: Failed to open preferences (read)!");
      return;
    }

    for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
      String ssidKey = "ssid" + String(i);
      String passKey = "pass" + String(i);

      // Only try to read if the key exists to avoid error messages
      if (preferences.isKey(ssidKey.c_str())) {
        String ssid = preferences.getString(ssidKey.c_str(), "");
        String pass = preferences.getString(passKey.c_str(), "");

        if (ssid.length() > 0) {
          ssid.toCharArray(networks[i].ssid, sizeof(networks[i].ssid));
          pass.toCharArray(networks[i].password, sizeof(networks[i].password));
          Serial.printf("[WiFiManager] Loaded network %d: %s\n", i, networks[i].ssid);
        }
      }
    }
    preferences.end();
  }

  void loadLastSuccessful() {
    if (!preferences.begin("wifi", true)) {
      Serial.println("[WiFiManager] ERROR: Failed to open preferences (read)!");
      return;
    }

    lastSuccessfulIndex = preferences.getInt("lastIdx", -1);
    if (lastSuccessfulIndex >= 0) {
      Serial.printf("[WiFiManager] Last successful network index: %d\n", lastSuccessfulIndex);
    }
    preferences.end();
  }

  void saveLastSuccessful(int index) {
    if (!preferences.begin("wifi", false)) {
      Serial.println("[WiFiManager] ERROR: Failed to open preferences (write)!");
      return;
    }

    bool ok = preferences.putInt("lastIdx", index);
    preferences.end();

    if (!ok) {
      Serial.println("[WiFiManager] ERROR: Failed to save last successful index!");
      return;
    }

    lastSuccessfulIndex = index;
    Serial.printf("[WiFiManager] Saved last successful network index: %d\n", index);
  }

  void setNetwork(int index, const char *ssid, const char *password) {
    if (index < 0 || index >= MAX_WIFI_NETWORKS) {
      Serial.printf("[WiFiManager] ERROR: Invalid network index: %d\n", index);
      return;
    }

    if (!preferences.begin("wifi", false)) {
      Serial.println("[WiFiManager] ERROR: Failed to open preferences (write)!");
      return;
    }

    String ssidKey = "ssid" + String(index);
    String passKey = "pass" + String(index);

    bool ssidOk = preferences.putString(ssidKey.c_str(), ssid);
    bool passOk = preferences.putString(passKey.c_str(), password);

    preferences.end();

    if (!ssidOk || !passOk) {
      Serial.printf("[WiFiManager] ERROR: Failed to write network %d to preferences!\n", index);
      return;
    }

    strncpy(networks[index].ssid, ssid, sizeof(networks[index].ssid) - 1);
    networks[index].ssid[sizeof(networks[index].ssid) - 1] = '\0';

    strncpy(networks[index].password, password, sizeof(networks[index].password) - 1);
    networks[index].password[sizeof(networks[index].password) - 1] = '\0';

    Serial.printf("[WiFiManager] Network %d updated: %s\n", index, ssid);
  }

  bool connectToNextNetwork() {
    // Try last successful network first
    if (lastSuccessfulIndex >= 0 && lastSuccessfulIndex < MAX_WIFI_NETWORKS) {
      if (strlen(networks[lastSuccessfulIndex].ssid) > 0) {
        if (attemptConnection(lastSuccessfulIndex)) {
          return true;
        }
      }
    }

    // Try all networks in order
    for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
      if (i == lastSuccessfulIndex) continue; // Already tried
      if (strlen(networks[i].ssid) > 0) {
        if (attemptConnection(i)) {
          return true;
        }
      }
    }

    Serial.println("[WiFiManager] No available networks to connect to!");
    return false;
  }

  bool attemptConnection(int index) {
    if (index < 0 || index >= MAX_WIFI_NETWORKS) return false;
    if (strlen(networks[index].ssid) == 0) return false;

    Serial.printf("[WiFiManager] Attempting to connect to: %s\n", networks[index].ssid);
    
    WiFi.begin(networks[index].ssid, networks[index].password);
    
    unsigned long startTime = millis();
    while (!WiFi.isConnected() && (millis() - startTime) < WIFI_CONNECT_TIMEOUT_MS) {
      Serial.print(".");
      delay(500);
    }

    if (WiFi.isConnected()) {
      Serial.println("OK");
      Serial.printf("[WiFiManager] Connected to: %s\n", networks[index].ssid);
      Serial.print("[WiFiManager] IP: ");
      Serial.println(WiFi.localIP());
      saveLastSuccessful(index);
      return true;
    } else {
      Serial.println("FAILED");
      Serial.printf("[WiFiManager] Failed to connect to: %s\n", networks[index].ssid);
      WiFi.disconnect();
      return false;
    }
  }

  bool isConnected() {
    return WiFi.isConnected();
  }

  void handleReconnection() {
    if (WiFi.isConnected()) {
      return; // Already connected
    }

    unsigned long now = millis();
    if (now - lastConnectionAttempt < 30000) {
      return; // Wait 30 seconds before retry
    }

    lastConnectionAttempt = now;
    Serial.println("[WiFiManager] WiFi disconnected! Attempting to reconnect...");
    connectToNextNetwork();
  }

  int getNetworkCount() {
    int count = 0;
    for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
      if (strlen(networks[i].ssid) > 0) count++;
    }
    return count;
  }

  const char* getNetworkSSID(int index) {
    if (index < 0 || index >= MAX_WIFI_NETWORKS) return "";
    return networks[index].ssid;
  }

  void printNetworks() {
    Serial.println("[WiFiManager] Configured networks:");
    for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
      if (strlen(networks[i].ssid) > 0) {
        Serial.printf("  [%d] %s %s\n", i, networks[i].ssid, 
                      (i == lastSuccessfulIndex) ? "(last successful)" : "");
      }
    }
  }
};

