#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "iPixelDeviceRegistry.h"
#include "WiFiManager.h"
#include "DeviceManager.h"

AsyncWebServer server(80);
extern WiFiManager wifiManager;
extern DeviceManager deviceManager;

// Global buffers for file uploads (outside of lambda scope)
static std::vector<uint8_t> g_pngBuffer;
static std::vector<uint8_t> g_gifBuffer;

void init_webserver() {
    Serial.println("Initializing webserver...");

    // Serve index.html for root
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/web/index.html", "text/html");
    });

    // Serve control.js
    server.on("/control.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/web/control.js", "application/javascript");
    });

    server.onNotFound([](AsyncWebServerRequest* request) {
        String url = request->url();
        Serial.println("Incoming: " + url);

        if (!url.startsWith("/device/")) {
            request->send(404, "text/plain", "Unknown route");
            return;
        }

        int firstSlash = url.indexOf('/', 8); // position after "/device/"
        if (firstSlash == -1) {
            request->send(400, "text/plain", "Invalid device address format");
            return;
        }

        String deviceIdentifier = url.substring(8, firstSlash);
        String action = url.substring(firstSlash + 1);

        Serial.printf("Device=%s, action=%s\n", deviceIdentifier.c_str(), action.c_str());

        // Try to resolve device identifier (MAC or name)
        NimBLEAddress addr;
        bool deviceFound = false;

        // First, try to find by name
        int deviceIndex = deviceManager.findDeviceByName(deviceIdentifier.c_str());
        if (deviceIndex >= 0) {
            addr = NimBLEAddress(deviceManager.getDeviceMAC(deviceIndex), 0);
            deviceFound = true;
            Serial.printf("[Webserver] Found device by name: %s -> %s\n", deviceIdentifier.c_str(), deviceManager.getDeviceMAC(deviceIndex));
        } else {
            // Try to find by MAC
            deviceIndex = deviceManager.findDeviceByMAC(deviceIdentifier.c_str());
            if (deviceIndex >= 0) {
                addr = NimBLEAddress(deviceIdentifier.c_str(), 0);
                deviceFound = true;
                Serial.printf("[Webserver] Found device by MAC: %s\n", deviceIdentifier.c_str());
            } else {
                // Assume it's a MAC address and try to create it
                addr = NimBLEAddress(deviceIdentifier.c_str(), 0);
                Serial.printf("[Webserver] Device not in registry, treating as MAC: %s\n", deviceIdentifier.c_str());
            }
        }

        iPixelDevice* dev = getOrCreateDevice(addr);

        if (!dev->connected) {
            request->send(408, "text/plain", "Device is connecting");
            return;
        }

        auto getParamInt = [&](const char* name, long def = 0L) -> long {
            if (request->hasParam(name)) return request->getParam(name)->value().toInt();
            return def;
        };
        auto getParamBool = [&](const char* name, bool def = false) {
            if (request->hasParam(name)) return request->getParam(name)->value() == "true";
            return def;
        };
        auto getParamString = [&](const char* name, const String def = "Unknown") {
            if (request->hasParam(name)) return request->getParam(name)->value();
            return def;
        };

        try {
            if (action == "setTime") {
                dev->setTime(getParamInt("hour"), getParamInt("minute"), getParamInt("second"));
            } else if (action == "setFunMode") {
                dev->setFunMode(getParamBool("funMode"));
            } else if (action == "setOrientation") {
                dev->setOrientation(getParamInt("orientation"));
            } else if (action == "clear") {
                dev->clear();
            } else if (action == "setBrightness") {
                dev->setBrightness(getParamInt("brightness"));
            } else if (action == "setSpeed") {
                dev->setSpeed(getParamInt("speed"));
            } else if (action == "ledOff") {
                dev->ledOff();
            } else if (action == "ledOn") {
                dev->ledOn();
            } else if (action == "deleteScreen") {
                dev->deleteScreen(getParamInt("screen"));
            } else if (action == "setPixel") {
                dev->setPixel(
                    getParamInt("x"),
                    getParamInt("y"),
                    getParamInt("r"),
                    getParamInt("g"),
                    getParamInt("b")
                );
            } else if (action == "setClockMode") {
                // Get all parameters, use -1 as default to trigger automatic defaults from NTP/checkRange
                long style = request->hasParam("style") ? getParamInt("style") : -1;
                long dayOfWeek = request->hasParam("dayOfWeek") ? getParamInt("dayOfWeek") : -1;
                long year = request->hasParam("year") ? getParamInt("year") : -1;
                long month = request->hasParam("month") ? getParamInt("month") : -1;
                long day = request->hasParam("day") ? getParamInt("day") : -1;
                long hour = request->hasParam("hour") ? getParamInt("hour") : -1;
                long minute = request->hasParam("minute") ? getParamInt("minute") : -1;
                long second = request->hasParam("second") ? getParamInt("second") : -1;
                dev->setClockMode(style, dayOfWeek, year, month, day, getParamBool("showDate"), getParamBool("format24"), hour, minute, second);
            } else if (action == "setRhythmLevelMode") {
                int levels[11];
                for (int i = 0; i < 11; ++i) {
                    levels[i] = getParamInt("l" + char(char('0') + i));
                }
                dev->setRhythmLevelMode(getParamInt("style"), levels);
            } else if (action == "setRhythmAnimationMode") {
                dev->setRhythmAnimationMode(getParamInt("style"), getParamInt("frame"));
            } else if (action == "fill") {
                for(int x=0; x<32; x++) {
                    for(int y=0; y<32; y++) {
                        dev->setPixel(x, y, 255, 255, 255);
                    }
                }
            } else if (action == "sendText") {
                dev->sendText(
                    getParamString("text"),
                    getParamInt("animation"),
                    getParamInt("save_slot"),
                    getParamInt("speed"),
                    getParamInt("colorR"),
                    getParamInt("colorG"),
                    getParamInt("colorB"),
                    getParamInt("rainbow_mode"),
                    getParamInt("matrix_height")
                );
            } else if (action == "setRhythmAnimationMode2") {
                dev->setRhythmAnimationMode2(getParamInt("style"), getParamInt("animationTime"));
            }  else {
                request->send(400, "text/plain", "Invalid action");
                return;
            }
            request->send(200, "text/plain", "OK");
        } catch(const std::invalid_argument &ex) {
            request->send(404, "text/plain", ex.what());
        }
  });

  // PNG Upload Endpoint
  server.on("/device/sendPNG", HTTP_POST, [](AsyncWebServerRequest* request) {
    // This is just the response handler, body is handled below
  }, [](AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final) {
    // Handle PNG file upload
    if (index == 0) {
      g_pngBuffer.clear();
      Serial.println("[Webserver] Starting PNG upload...");
    }

    // Append data to buffer
    g_pngBuffer.insert(g_pngBuffer.end(), data, data + len);

    if (final) {
      Serial.printf("[Webserver] PNG upload complete: %d bytes\n", g_pngBuffer.size());

      // Parse MAC from query parameter
      if (request->hasParam("mac")) {
        String macStr = request->getParam("mac")->value();
        NimBLEAddress addr(macStr.c_str(), 0);
        iPixelDevice* dev = getOrCreateDevice(addr);

        if (dev->connected) {
          try {
            dev->sendPNG(g_pngBuffer);
            request->send(200, "text/plain", "PNG sent successfully");
          } catch(const std::exception &ex) {
            request->send(400, "text/plain", ex.what());
          }
        } else {
          request->send(408, "text/plain", "Device is not connected");
        }
      } else {
        request->send(400, "text/plain", "Missing parameter: mac");
      }

      g_pngBuffer.clear();
    }
  });

  // GIF/Animation Upload Endpoint
  server.on("/device/sendAnimation", HTTP_POST, [](AsyncWebServerRequest* request) {
    // This is just the response handler, body is handled below
  }, [](AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final) {
    // Handle GIF file upload
    if (index == 0) {
      g_gifBuffer.clear();
      Serial.println("[Webserver] Starting GIF upload...");
    }

    // Append data to buffer
    g_gifBuffer.insert(g_gifBuffer.end(), data, data + len);

    if (final) {
      Serial.printf("[Webserver] GIF upload complete: %d bytes\n", g_gifBuffer.size());

      // Parse MAC from query parameter
      if (request->hasParam("mac")) {
        String macStr = request->getParam("mac")->value();
        NimBLEAddress addr(macStr.c_str(), 0);
        iPixelDevice* dev = getOrCreateDevice(addr);

        if (dev->connected) {
          try {
            dev->sendAnimation(g_gifBuffer);
            request->send(200, "text/plain", "Animation sent successfully");
          } catch(const std::exception &ex) {
            request->send(400, "text/plain", ex.what());
          }
        } else {
          request->send(408, "text/plain", "Device is not connected");
        }
      } else {
        request->send(400, "text/plain", "Missing parameter: mac");
      }

      g_gifBuffer.clear();
    }
  });

  // WiFi Management Endpoints
  server.on("/wifi/networks", HTTP_GET, [](AsyncWebServerRequest* request) {
    String response = "[";
    for (int i = 0; i < 3; i++) {
      const char* ssid = wifiManager.getNetworkSSID(i);
      if (strlen(ssid) > 0) {
        if (i > 0) response += ",";
        response += "{\"index\":" + String(i) + ",\"ssid\":\"" + String(ssid) + "\"}";
      }
    }
    response += "]";
    request->send(200, "application/json", response);
  });

  server.on("/wifi/set", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (!request->hasParam("index") || !request->hasParam("ssid") || !request->hasParam("password")) {
      request->send(400, "text/plain", "Missing parameters: index, ssid, password");
      return;
    }

    int index = request->getParam("index")->value().toInt();
    String ssid = request->getParam("ssid")->value();
    String password = request->getParam("password")->value();

    if (index < 0 || index >= 3) {
      request->send(400, "text/plain", "Invalid index (0-2)");
      return;
    }

    wifiManager.setNetwork(index, ssid.c_str(), password.c_str());
    request->send(200, "text/plain", "Network updated");
  });

  server.on("/wifi/connect", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (!request->hasParam("index")) {
      request->send(400, "text/plain", "Missing parameter: index");
      return;
    }

    int index = request->getParam("index")->value().toInt();
    bool success = wifiManager.attemptConnection(index);

    if (success) {
      request->send(200, "text/plain", "Connected successfully");
    } else {
      request->send(503, "text/plain", "Failed to connect");
    }
  });

  // Device Management Endpoints
  server.on("/devices/list", HTTP_GET, [](AsyncWebServerRequest* request) {
    String response = "[";
    int count = 0;
    for (int i = 0; i < 10; i++) {
      const char* mac = deviceManager.getDeviceMAC(i);
      if (strlen(mac) > 0) {
        if (count > 0) response += ",";
        response += "{\"index\":" + String(i) + ",\"mac\":\"" + String(mac) + "\",\"name\":\"" + String(deviceManager.getDeviceName(i)) + "\",\"lastConnected\":" + String(i == deviceManager.getLastConnectedIndex() ? "true" : "false") + "}";
        count++;
      }
    }
    response += "]";
    request->send(200, "application/json", response);
  });

  // Combined add/update endpoint - adds new device or updates existing one
  server.on("/devices/set", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (!request->hasParam("mac")) {
      request->send(400, "text/plain", "Missing parameter: mac");
      return;
    }

    String mac = request->getParam("mac")->value();
    String name = request->hasParam("name") ? request->getParam("name")->value() : "";

    int index = deviceManager.findDeviceByMAC(mac.c_str());
    bool isNew = (index < 0);

    // Add or update device
    deviceManager.addDevice(mac.c_str(), name.length() > 0 ? name.c_str() : "");

    String response = isNew ? "Device added successfully" : "Device updated successfully";
    if (name.length() > 0) {
      response += " (name: " + name + ")";
    }
    request->send(200, "text/plain", response);
  });

  server.on("/devices/last", HTTP_GET, [](AsyncWebServerRequest* request) {
    const char* lastMAC = deviceManager.getLastConnectedMAC();
    if (strlen(lastMAC) == 0) {
      request->send(404, "text/plain", "No last connected device");
      return;
    }

    int index = deviceManager.findDeviceByMAC(lastMAC);
    String response = "{\"mac\":\"" + String(lastMAC) + "\",\"name\":\"" + String(deviceManager.getDeviceName(index)) + "\"}";
    request->send(200, "application/json", response);
  });

  server.begin();
  Serial.println("Webserver initialized!");
}