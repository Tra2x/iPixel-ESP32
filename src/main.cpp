#include <Arduino.h>
#include <ImprovWiFiLibrary.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include "Bluetooth.h"
#include "Webserver.h"
#include "WiFiManager.h"
#include "NTPManager.h"
#include "DeviceManager.h"
#include "iPixelDeviceRegistry.h"

Preferences preferences;
ImprovWiFi improvSerial(&Serial);
WiFiManager wifiManager;
NTPManager ntpManager;
DeviceManager deviceManager;

void loop_connected();
void setup_connected();
void setup_wifi_post();

void onImprovWiFiErrorCb(ImprovTypes::Error err) {
  Serial.println("[Improv] WiFi failed! Reconnecting to saved networks...");
  setup_wifi_post();
}

void onImprovWiFiConnectedCb(const char *ssid, const char *password) {
  Serial.println("[Improv] Got WiFi credentials! (no, we won't leak them here :/)");
  // Store as first network (index 0)
  wifiManager.setNetwork(0, ssid, password);
  setup_wifi_post();
}

bool connectWifi(const char *ssid, const char *password) {
  // This is called by Improv to connect to a new network
  // We'll store it and attempt connection
  wifiManager.setNetwork(0, ssid, password);

  // Attempt connection with timeout
  unsigned long startTime = millis();
  WiFi.begin(ssid, password);

  while (!WiFi.isConnected() && (millis() - startTime) < WIFI_CONNECT_TIMEOUT_MS) {
    delay(100);
  }

  return WiFi.isConnected();
}

void setup_wifi_pre() {
  WiFi.mode(WIFI_STA);
  Serial.println("[WiFi] Mode is now 'STATION'!");

  WiFi.disconnect();
  Serial.println("[WiFi] Disconnected after startup!");
}

void setup_wifi_post() {
  wifiManager.printNetworks();

  if (wifiManager.getNetworkCount() == 0) {
    Serial.println("[WiFi] No credentials set! Waiting for Improv configuration...");
    return;
  }

  if (wifiManager.connectToNextNetwork()) {
    setup_connected();
  } else {
    Serial.println("[WiFi] Failed to connect to any network!");
  }
}

void setup_improv() {
  Serial.println("[Improv] Setting up...");
  improvSerial.setDeviceInfo(ImprovTypes::ChipFamily::CF_ESP32, "iPixel-Server", "1.0.0", "ESP32", "http://{LOCAL_IPV4}?name=Guest");
  improvSerial.onImprovError(onImprovWiFiErrorCb);
  improvSerial.onImprovConnected(onImprovWiFiConnectedCb);
  improvSerial.setCustomConnectWiFi(connectWifi);
  Serial.println("[Improv] Ready!");
}

void setup() {
  delay(2000);
  Serial.begin(115200);
  Serial.println("[Setup] Hello World! Let's hope we can pixel together!");

  // Initialize SPIFFS for web files
  if (!SPIFFS.begin(true)) {
    Serial.println("[Setup] SPIFFS Mount Failed");
  } else {
    Serial.println("[Setup] SPIFFS Mounted Successfully");
  }

  // Initialize WiFiManager BEFORE using it
  wifiManager.init();

  // Initialize DeviceManager BEFORE using it
  deviceManager.init();

  setup_wifi_pre();
  setup_improv();
  setup_wifi_post();
}

void setup_connected() {
  init_bluetooth();
  init_webserver();

  // Initialize NTP synchronization
  ntpManager.init();
  ntpManager.syncTime();  // Sync immediately on connection

  // Auto-connect to last connected device
  const char* lastMAC = deviceManager.getLastConnectedMAC();
  if (lastMAC && strlen(lastMAC) > 0) {
    Serial.printf("[Setup] Auto-connecting to last device: %s\n", lastMAC);
    NimBLEAddress lastAddr(lastMAC, 0);
    getOrCreateDevice(lastAddr)->connectAsync();
  }
}

void loop() {
  improvSerial.handleSerial();

  // Handle WiFi reconnection if disconnected
  wifiManager.handleReconnection();

  if (improvSerial.isConnected()) loop_connected();
}

void loop_connected() {
  // Handle periodic NTP synchronization
  ntpManager.handleSync();

  loop_deviceregistry();
}

//  iPixelDevice test(BLEAddress("3d:50:0c:1f:6d:ec"));
//2F:9F:9C:9C:51:AC	