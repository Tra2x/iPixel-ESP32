#include "iPixelDeviceRegistry.h"
#include "DeviceManager.h"

std::vector<iPixelDevice*> knownDevices;
extern DeviceManager deviceManager;

iPixelDevice* getOrCreateDevice(const BLEAddress& addr) {
    for (auto* dev : knownDevices) {
        if (dev->address.equals(addr)) {
        return dev;
        }
    }

    iPixelDevice* newDev = new iPixelDevice(addr);
    knownDevices.push_back(newDev);

    // Save the device MAC address to persistent storage
    std::string macStr = addr.toString();
    deviceManager.addDevice(macStr.c_str());

    return newDev;
}

void loop_deviceregistry() {
    static unsigned long lastAttempt = 0;
    //Connection attempts
    if (millis() - lastAttempt > 1000) {
        lastAttempt = millis();
        for (auto* dev : knownDevices) {
        if (!dev->connected)
            dev->connectAsync();
        }
    }

    //Send from queue
    for (auto* dev : knownDevices)
        if (dev->connected) dev->queueTick();
}