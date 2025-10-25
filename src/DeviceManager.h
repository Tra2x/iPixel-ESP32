#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <NimBLEDevice.h>

#define MAX_KNOWN_DEVICES 10

struct KnownDevice {
    char macAddress[18];  // MAC address as string (17 chars + null terminator)
    char deviceName[33];  // Optional device name (32 chars + null terminator)
};

class DeviceManager {
private:
    Preferences preferences;
    KnownDevice devices[MAX_KNOWN_DEVICES];
    int lastConnectedIndex = -1;
    bool initialized = false;

public:
    DeviceManager() {
        memset(devices, 0, sizeof(devices));
    }

    void init() {
        if (initialized) return;

        Serial.println("[DeviceManager] Initializing...");
        delay(100); // Give NVS time to initialize

        loadDevices();
        loadLastConnected();
        initialized = true;

        Serial.println("[DeviceManager] Initialization complete!");
    }

    void loadDevices() {
        if (!preferences.begin("devices", true)) { // read-only
            // This is expected on first boot - namespace doesn't exist yet
            return;
        }

        for (int i = 0; i < MAX_KNOWN_DEVICES; i++) {
            String macKey = "mac" + String(i);
            String nameKey = "name" + String(i);

            // Only try to read if the key exists to avoid error messages
            if (preferences.isKey(macKey.c_str())) {
                String mac = preferences.getString(macKey.c_str(), "");

                if (mac.length() > 0) {
                    mac.toCharArray(devices[i].macAddress, sizeof(devices[i].macAddress));

                    // Only read name if it exists
                    if (preferences.isKey(nameKey.c_str())) {
                        String name = preferences.getString(nameKey.c_str(), "");
                        if (name.length() > 0) {
                            name.toCharArray(devices[i].deviceName, sizeof(devices[i].deviceName));
                        }
                    }
                    Serial.printf("[DeviceManager] Loaded device %d: %s\n", i, devices[i].macAddress);
                }
            }
        }
        preferences.end();
    }

    void loadLastConnected() {
        if (!preferences.begin("devices", true)) {
            Serial.println("[DeviceManager] ERROR: Failed to open preferences (read)!");
            return;
        }

        lastConnectedIndex = preferences.getInt("lastConnIdx", -1);
        if (lastConnectedIndex >= 0 && lastConnectedIndex < MAX_KNOWN_DEVICES) {
            if (strlen(devices[lastConnectedIndex].macAddress) > 0) {
                Serial.printf("[DeviceManager] Last connected device index: %d (%s)\n", 
                              lastConnectedIndex, devices[lastConnectedIndex].macAddress);
            }
        }
        preferences.end();
    }

    void saveLastConnected(int index) {
        if (index < 0 || index >= MAX_KNOWN_DEVICES) {
            Serial.printf("[DeviceManager] ERROR: Invalid device index: %d\n", index);
            return;
        }

        if (!preferences.begin("devices", false)) {
            Serial.println("[DeviceManager] ERROR: Failed to open preferences (write)!");
            return;
        }

        bool ok = preferences.putInt("lastConnIdx", index);
        preferences.end();

        if (!ok) {
            Serial.println("[DeviceManager] ERROR: Failed to save last connected index!");
            return;
        }

        lastConnectedIndex = index;
        Serial.printf("[DeviceManager] Saved last connected device index: %d (%s)\n", 
                      index, devices[index].macAddress);
    }

    void addDevice(const char *macAddress, const char *deviceName = "") {
        // Check if device already exists
        for (int i = 0; i < MAX_KNOWN_DEVICES; i++) {
            if (strcmp(devices[i].macAddress, macAddress) == 0) {
                // Device exists - update name if provided
                if (deviceName && strlen(deviceName) > 0) {
                    if (!preferences.begin("devices", false)) {
                        Serial.println("[DeviceManager] ERROR: Failed to open preferences (write)!");
                        return;
                    }

                    String nameKey = "name" + String(i);
                    bool nameOk = preferences.putString(nameKey.c_str(), deviceName);
                    preferences.end();

                    if (!nameOk) {
                        Serial.printf("[DeviceManager] ERROR: Failed to update device name for index %d!\n", i);
                        return;
                    }

                    strncpy(devices[i].deviceName, deviceName, sizeof(devices[i].deviceName) - 1);
                    devices[i].deviceName[sizeof(devices[i].deviceName) - 1] = '\0';
                    Serial.printf("[DeviceManager] Device %d updated with name: %s\n", i, deviceName);
                } else {
                    Serial.printf("[DeviceManager] Device already exists: %s\n", macAddress);
                }
                return;
            }
        }

        // Find first empty slot
        for (int i = 0; i < MAX_KNOWN_DEVICES; i++) {
            if (strlen(devices[i].macAddress) == 0) {
                if (!preferences.begin("devices", false)) {
                    Serial.println("[DeviceManager] ERROR: Failed to open preferences (write)!");
                    return;
                }

                String macKey = "mac" + String(i);
                String nameKey = "name" + String(i);

                bool macOk = preferences.putString(macKey.c_str(), macAddress);
                bool nameOk = true;
                if (deviceName && strlen(deviceName) > 0) {
                    nameOk = preferences.putString(nameKey.c_str(), deviceName);
                }

                preferences.end();

                if (!macOk || !nameOk) {
                    Serial.printf("[DeviceManager] ERROR: Failed to write device %d to preferences!\n", i);
                    return;
                }

                strncpy(devices[i].macAddress, macAddress, sizeof(devices[i].macAddress) - 1);
                devices[i].macAddress[sizeof(devices[i].macAddress) - 1] = '\0';

                if (deviceName && strlen(deviceName) > 0) {
                    strncpy(devices[i].deviceName, deviceName, sizeof(devices[i].deviceName) - 1);
                    devices[i].deviceName[sizeof(devices[i].deviceName) - 1] = '\0';
                }

                Serial.printf("[DeviceManager] Device %d added: %s\n", i, macAddress);
                return;
            }
        }

        Serial.println("[DeviceManager] ERROR: No space to add new device!");
    }

    int getLastConnectedIndex() {
        return lastConnectedIndex;
    }

    const char* getLastConnectedMAC() {
        if (lastConnectedIndex >= 0 && lastConnectedIndex < MAX_KNOWN_DEVICES) {
            if (strlen(devices[lastConnectedIndex].macAddress) > 0) {
                return devices[lastConnectedIndex].macAddress;
            }
        }
        return "";
    }

    int getDeviceCount() {
        int count = 0;
        for (int i = 0; i < MAX_KNOWN_DEVICES; i++) {
            if (strlen(devices[i].macAddress) > 0) count++;
        }
        return count;
    }

    const char* getDeviceMAC(int index) {
        if (index < 0 || index >= MAX_KNOWN_DEVICES) return "";
        return devices[index].macAddress;
    }

    const char* getDeviceName(int index) {
        if (index < 0 || index >= MAX_KNOWN_DEVICES) return "";
        return devices[index].deviceName;
    }

    // Find device by MAC address
    int findDeviceByMAC(const char* macAddress) {
        for (int i = 0; i < MAX_KNOWN_DEVICES; i++) {
            if (strcmp(devices[i].macAddress, macAddress) == 0) {
                return i;
            }
        }
        return -1;
    }

    // Find device by name
    int findDeviceByName(const char* deviceName) {
        for (int i = 0; i < MAX_KNOWN_DEVICES; i++) {
            if (strlen(devices[i].deviceName) > 0 &&
                strcmp(devices[i].deviceName, deviceName) == 0) {
                return i;
            }
        }
        return -1;
    }

    // Get device identifier (MAC or name if available)
    const char* getDeviceIdentifier(int index) {
        if (index < 0 || index >= MAX_KNOWN_DEVICES) return "";
        if (strlen(devices[index].deviceName) > 0) {
            return devices[index].deviceName;
        }
        return devices[index].macAddress;
    }

    void printDevices() {
        Serial.println("[DeviceManager] Known devices:");
        for (int i = 0; i < MAX_KNOWN_DEVICES; i++) {
            if (strlen(devices[i].macAddress) > 0) {
                Serial.printf("  [%d] %s %s%s\n", i, devices[i].macAddress,
                              strlen(devices[i].deviceName) > 0 ? devices[i].deviceName : "",
                              (i == lastConnectedIndex) ? " (last connected)" : "");
            }
        }
    }
};

